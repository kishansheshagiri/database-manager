#include "pqp/query_runner_product.h"

#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"

QueryRunnerProduct::QueryRunnerProduct(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerProduct::~QueryRunnerProduct() {
  if (table_scan_child_ != nullptr) {
    delete table_scan_child_;
    table_scan_child_ = nullptr;
  }
}

bool QueryRunnerProduct::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 2) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_CROSS_PRODUCT;
    return false;
  }

  SetCallback(callback);

  QueryNode *left_child = Node()->Child(0);
  QueryNode *right_child = Node()->Child(1);
  if (right_child->Type() == QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT) {
    SetChildRunner(Create(right_child));
    table_scan_child_ = Create(left_child);
  } else {
    SetChildRunner(Create(left_child));
    table_scan_child_ = Create(right_child);
  }

  ScanParams params;
  params.num_blocks_ = 1;
  int start_index = scan_params_.start_index_;
  params.start_index_ = start_index;
  ChildRunner()->PassScanParams(params);

  params.start_index_++;
  table_scan_child_->PassScanParams(params);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerProduct::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
    }

    error_code = error_code_;
    return false;
  }

  return true;
}

bool QueryRunnerProduct::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  if (tuples.empty()) {
    DEBUG_MSG("");
    return true;
  }

  if (child == ChildRunner()) {
    first_tuples_ = tuples;
    if (!table_scan_child_->Run(
        std::bind(&QueryRunnerProduct::ResultCallback, this,
            std::placeholders::_1, std::placeholders::_2),
        error_code_)) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
      return false;
    }

    first_tuples_.clear();
  } else if (child == table_scan_child_) {
    std::vector<Tuple> output_tuples;
    for (auto first_tuple : first_tuples_) {
      for (auto second_tuple : tuples) {
        if (intermediate_relation_name_.empty()) {
          std::string table_name_first, table_name_second;
          ChildRunner()->TableName(table_name_first);
          table_scan_child_->TableName(table_name_second);

          if (!createIntermediateRelation(first_tuple, second_tuple,
              table_name_first, table_name_second)) {
            DEBUG_MSG("");
            error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
            return false;
          }
        }

        Tuple merged_tuple = Tuple::getDummyTuple();
        if (!mergeTuples(first_tuple, second_tuple, merged_tuple)) {
          DEBUG_MSG("");
          error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
          return false;
        }

        output_tuples.push_back(merged_tuple);
      }
    }

    if (!output_tuples.empty()) {
      return Callback()(this, output_tuples);
    }
  } else {
    DEBUG_MSG("");
    error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
    return false;
  }

  return true;
}

void QueryRunnerProduct::PassScanParams(ScanParams params) {
  scan_params_ = params;
}

bool QueryRunnerProduct::TableName(std::string& table_name) {
  return true;
}

bool QueryRunnerProduct::TableSize(int& blocks, int& tuples) {
  return true;
}

void QueryRunnerProduct::DeleteTemporaryRelations() {
  if (table_scan_child_ != nullptr) {
    table_scan_child_->DeleteTemporaryRelations();
  }

  QueryRunner::DeleteTemporaryRelations();
}

bool QueryRunnerProduct::createIntermediateRelation(Tuple first, Tuple second,
    std::string table_name_first, std::string table_name_second) {
  Schema schema_first = first.getSchema();
  Schema schema_second = second.getSchema();
  std::vector<std::string> field_names = schema_first.getFieldNames();
  std::vector<enum FIELD_TYPE> field_types = schema_first.getFieldTypes();

  for (auto &field_name : field_names) {
    if (!table_name_first.empty())
      field_name = table_name_first + "." + field_name;
  }

  std::vector<std::string> field_names_second = schema_second.getFieldNames();
  std::vector<enum FIELD_TYPE> field_types_second =
      schema_second.getFieldTypes();

  for (auto &field_name : field_names_second) {
    if (!table_name_second.empty())
      field_name = table_name_second + "." + field_name;
  }

  field_names.insert(field_names.end(),
      field_names_second.begin(), field_names_second.end());
  field_types.insert(field_types.end(),
      field_types_second.begin(), field_types_second.end());

  if (!Storage()->CreateDummyRelation("Product_", field_names, field_types,
      intermediate_relation_name_)) {
    DEBUG_MSG("");
    return false;
  }

  MarkTemporaryRelation(intermediate_relation_name_);

  return true;
}

bool QueryRunnerProduct::mergeTuples(Tuple first, Tuple second,
    Tuple& merged_tuple) {
  if (intermediate_relation_name_.empty()) {
    DEBUG_MSG("");
    return false;
  }

  if (!Storage()->CreateEmptyTuple(intermediate_relation_name_, merged_tuple)) {
    DEBUG_MSG("");
    return false;
  }

  for (auto it = 0; it < first.getNumOfFields(); it++) {
    Schema schema = first.getSchema();
    if (schema.getFieldType(it) == INT) {
      merged_tuple.setField(it, first.getField(it).integer);
    } else if (schema.getFieldType(it) == STR20) {
      merged_tuple.setField(it, *(first.getField(it).str));
    } else {
      DEBUG_MSG("");
      return false;
    }
  }

  for (auto it = 0; it < second.getNumOfFields(); it++) {
    Schema schema = second.getSchema();
    if (schema.getFieldType(it) == INT) {
      merged_tuple.setField(
          first.getNumOfFields() + it, second.getField(it).integer);
    } else if (schema.getFieldType(it) == STR20) {
      merged_tuple.setField(
          first.getNumOfFields() + it, *(second.getField(it).str));
    } else {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}
