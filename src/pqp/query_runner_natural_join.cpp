#include "pqp/query_runner_natural_join.h"

#include <numeric>
#include <vector>

#include "base/debug.h"
#include "pqp/tuple_helper.h"
#include "pqp/where_clause_helper_select.h"

QueryRunnerNaturalJoin::QueryRunnerNaturalJoin(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerNaturalJoin::~QueryRunnerNaturalJoin() {
  if (right_child_ != nullptr) {
    delete right_child_;
    right_child_ = nullptr;
  }
}

bool QueryRunnerNaturalJoin::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 2) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_NATURAL_JOIN;
    return false;
  }

  if (!Node()->WhereHelper(where_helper_) || where_helper_ == nullptr) {
    DEBUG_MSG("");
    return false;
  }

  if (!Node()->SortColumn(compare_column_)) {
    DEBUG_MSG("");
    return false;
  }


  SetChildRunner(Create(Node()->Child(0)));
  right_child_ = Create(Node()->Child(1));

  if (!ChildRunner()->Initialize(error_code) ||
      !right_child_->Initialize(error_code)) {
    DEBUG_MSG("Failed to initialize");
    return false;
  }

  return true;
}

bool QueryRunnerNaturalJoin::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  ScanParams params;
  params.start_index_ = 0;
  params.num_blocks_ = Storage()->MainMemorySize() / 2;
  ChildRunner()->PassScanParams(params);

  params.start_index_ = Storage()->MainMemorySize() / 2;
  params.num_blocks_ = Storage()->MainMemorySize() / 2 - 1;
  right_child_->PassScanParams(params);

  if (!ChildRunner()->Run(
          std::bind(&QueryRunnerNaturalJoin::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
          error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_NATURAL_JOIN;
    }

    error_code = error_code_;
    return false;
  }

  if (!right_child_->Run(
      std::bind(&QueryRunnerNaturalJoin::ResultCallback, this,
      std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_NATURAL_JOIN;
    }

    error_code = error_code_;
    return false;
  }

  if (left_tuples_.empty()) {
    return Callback()(this, right_tuples_);
  }

  if (right_tuples_.empty()) {
    return Callback()(this, left_tuples_);
  }

  if (intermediate_relation_name_.empty()) {
    std::string table_name_first, table_name_second;
    ChildRunner()->TableName(table_name_first);
    right_child_->TableName(table_name_second);

    if (!createIntermediateRelation(left_tuples_[0], right_tuples_[0],
        table_name_first, table_name_second)) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_NATURAL_JOIN;
      return false;
    }
  }

  auto left_index = 0, right_index = 0;
  while (left_index < left_tuples_.size() &&
      right_index < right_tuples_.size()) {
    Tuple merged_tuple = Tuple::getDummyTuple();
    if (!mergeTuples(left_tuples_[left_index], right_tuples_[right_index],
        merged_tuple)) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_NATURAL_JOIN;
      return false;
    }

    if (where_helper_->Evaluate(&merged_tuple, error_code)) {
      std::vector<Tuple> output_tuples(1, merged_tuple);
      Callback()(this, output_tuples);
      left_index++;
      right_index++;
      continue;
    }

    std::vector<Tuple> comparison_tuples = {
        left_tuples_[left_index],
        right_tuples_[right_index]
    };

    CompareTuples comparator(this, comparison_tuples);
    if (comparator(0, 1)) {
      left_index++;
    } else {
      right_index++;
    }
  }

  return true;
}

bool QueryRunnerNaturalJoin::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  if (tuples.empty()) {
    DEBUG_MSG("");
    return true;
  }

  if (child == ChildRunner()) {
    left_tuples_.insert(left_tuples_.end(), tuples.begin(), tuples.end());
  } else if (child == right_child_) {
    right_tuples_.insert(right_tuples_.end(), tuples.begin(), tuples.end());
  } else {
    DEBUG_MSG("Invalid child for natural join");
    return false;
  }

  return true;
}

std::string QueryRunnerNaturalJoin::CompareColumn() const {
  return compare_column_;
}

void QueryRunnerNaturalJoin::PassScanParams(ScanParams params) {
}

bool QueryRunnerNaturalJoin::TableName(std::string& table_name) {
  return false;
}

bool QueryRunnerNaturalJoin::TableSize(int& blocks, int& tuples) {
  return false;
}

bool QueryRunnerNaturalJoin::HasSortNode() const {
  return true;
}

void QueryRunnerNaturalJoin::DeleteTemporaryRelations() {
  if (right_child_ != nullptr) {
    right_child_->DeleteTemporaryRelations();
  }

  QueryRunner::DeleteTemporaryRelations();
}

bool QueryRunnerNaturalJoin::createIntermediateRelation(Tuple first, Tuple second,
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

  if (!Storage()->CreateDummyRelation("NaturalJoin_", field_names, field_types,
      intermediate_relation_name_)) {
    DEBUG_MSG("");
    return false;
  }

  MarkTemporaryRelation(intermediate_relation_name_);

  return true;
}

bool QueryRunnerNaturalJoin::mergeTuples(Tuple first, Tuple second,
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
