#include "pqp/query_runner.h"

#include <iomanip>

#include "base/debug.h"
#include "pqp/query_runner_factory.h"

QueryRunner::QueryRunner(QueryNode *query_node)
  : query_node_(query_node),
    child_runner_(nullptr),
    fields_printed_(0),
    storage_adapter_(StorageAdapter::Get()) {
}

QueryRunner::~QueryRunner() {
  if (child_runner_ != nullptr) {
    delete child_runner_;
    child_runner_ = nullptr;
  }
}

bool QueryRunner::Start(SqlErrors::Type& error_code) {
  if (!Run(std::bind(&QueryRunner::Print, this,
                     std::placeholders::_1, std::placeholders::_2,
                     std::placeholders::_3),
             error_code)) {
    DEBUG_MSG("");
    return false;
  }

  printClose();

  return false;
}

bool QueryRunner::Print(QueryRunner *child,
    std::vector<Tuple>& tuples, bool headers) {
  if (headers) {
    Tuple field_names = tuples[0];
    int field_count = field_names.getNumOfFields();
    ERROR_MSG_SINGLE_LINE("\n");
    ERROR_MSG_SINGLE_LINE(
        "+" << std::string(19 * field_count, '-') << "+");
    ERROR_MSG_SINGLE_LINE("\n");

    for (int index = 0; index < field_count; index++) {
      ERROR_MSG_SINGLE_LINE("|" << setw(18) << std::left <<
          *(field_names.getField(index).str));
    }

    ERROR_MSG_SINGLE_LINE(" |\n");
    ERROR_MSG_SINGLE_LINE(
        "+" << std::string(19 * field_count, '-') << "+");
    ERROR_MSG_SINGLE_LINE("\n");

    fields_printed_ = field_count;
  } else {
    for (auto tuple : tuples) {
      for (int index = 0; index < tuple.getNumOfFields(); index++) {
        std::string field_value;
        if (tuple.getSchema().getFieldType(index) == INT) {
          int value = tuple.getField(index).integer;
          field_value = std::to_string(value);
          if (value == -1) {
            field_value = "NULL";
          }
        } else if (tuple.getSchema().getFieldType(index) == STR20) {
          field_value = *(tuple.getField(index).str);
        }

        ERROR_MSG_SINGLE_LINE("|" << setw(18) << std::left << field_value);
      }

      ERROR_MSG_SINGLE_LINE(" |\n");
    }
  }

  return true;
}

void QueryRunner::PassScanParams(ScanParams params) {
  if (child_runner_ && Node() && Node()->ChildrenCount() == 1) {
    child_runner_->PassScanParams(params);
  }
}

bool QueryRunner::TableName(std::string& table_name) {
  if (child_runner_ && Node() && Node()->ChildrenCount() == 1) {
    return child_runner_->TableName(table_name);
  }

  DEBUG_MSG("");
  return false;
}

bool QueryRunner::TableSize(int& blocks, int& tuples) {
  if (child_runner_ && Node() && Node()->ChildrenCount() == 1) {
    return child_runner_->TableSize(blocks, tuples);
  }

  DEBUG_MSG("");
  return false;
}

bool QueryRunner::TableHeaders(std::vector<Tuple>& tuples) {
  if (child_runner_ && Node() && Node()->ChildrenCount() == 1) {
    return child_runner_->TableHeaders(tuples);
  }

  DEBUG_MSG("");
  return false;
}

void QueryRunner::SetChildRunner(QueryRunner *child_runner) {
  child_runner_ = child_runner;
}

void QueryRunner::SetCallback(QueryResultCallback callback) {
  callback_ = callback;
}

QueryRunner *QueryRunner::Create(QueryNode *child_node) {
  QueryRunnerFactory factory(child_node);
  return factory.Create();
}

bool QueryRunner::MergeTableHeaders(std::vector<Tuple>& first,
    std::string table_name_first, std::vector<Tuple>& second,
    std::string table_name_second, std::vector<Tuple>& merged_tuples) {
  Tuple tuple_first = first[0];
  std::vector<std::string> field_names_first;
  for (int index = 0; index < tuple_first.getNumOfFields(); index++) {
    field_names_first.push_back(table_name_first + "." +
        *(tuple_first.getField(index).str));
  }

  Tuple tuple_second = second[0];
  std::vector<std::string> field_names_second;
  for (int index = 0; index < tuple_second.getNumOfFields(); index++) {
    field_names_second.push_back(table_name_second + "." +
        *(tuple_second.getField(index).str));
  }

  std::string temp_relation_name;
  field_names_first.insert(field_names_first.end(),
      field_names_second.begin(), field_names_second.end());
  if (!Storage()->CreateDummyRelation("Header_", field_names_first,
      temp_relation_name)) {
    DEBUG_MSG("");
    return false;
  }

  bool created = false;
  Tuple field_name_tuple = Storage()->CreateTuple(
      temp_relation_name, field_names_first, created);
  if (!created) {
    DEBUG_MSG("");
    return false;
  }

  merged_tuples.clear();
  merged_tuples.push_back(field_name_tuple);
  // Storage()->DeleteDummyRelation(temp_relation_name);
  return true;
}

void QueryRunner::printClose() {
  ERROR_MSG_SINGLE_LINE(
      "+" << std::string(19 * fields_printed_, '-') << "+");
  ERROR_MSG_SINGLE_LINE("\n");
}
