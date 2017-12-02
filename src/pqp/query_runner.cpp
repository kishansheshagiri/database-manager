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
  if (!Run(std::bind(&QueryRunner::Print,
                       this, std::placeholders::_1, std::placeholders::_2),
             error_code)) {
    DEBUG_MSG("");
    return false;
  }

  printClose();

  return false;
}

bool QueryRunner::Print(std::vector<Tuple>& tuples, bool headers) {
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

void QueryRunner::printClose() {
  ERROR_MSG_SINGLE_LINE(
      "+" << std::string(19 * fields_printed_, '-') << "+");
  ERROR_MSG_SINGLE_LINE("\n");
}
