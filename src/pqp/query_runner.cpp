#include "pqp/query_runner.h"

#include <iomanip>

#include "base/debug.h"
#include "pqp/query_runner_factory.h"

QueryRunner::QueryRunner(QueryNode *query_node)
  : query_node_(query_node),
    storage_adapter_(StorageAdapter::Get()) {
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
    DEBUG_MSG_SINGLE_LINE("\n");
    DEBUG_MSG_SINGLE_LINE(
        "+" << std::string(19 * tuples.size(), '-') << "+");
    DEBUG_MSG_SINGLE_LINE("\n");

    int index;
    Tuple field_names = tuples[0];
    for (index = 0; index < field_names.getNumOfFields(); index++) {
      DEBUG_MSG_SINGLE_LINE("|" << setw(18) << std::left <<
          *(field_names.getField(index).str));
    }

    DEBUG_MSG_SINGLE_LINE(" |\n");
    DEBUG_MSG_SINGLE_LINE(
        "+" << std::string(19 * index, '-') << "+");
    DEBUG_MSG_SINGLE_LINE("\n");

    fields_printed_ = index;
  } else {
    for (auto tuple : tuples) {
      for (int index = 0; index < tuple.getNumOfFields(); index++) {
        DEBUG_MSG_SINGLE_LINE("|" << setw(18) << std::left <<
            *(tuple.getField(index).str));
      }

      DEBUG_MSG_SINGLE_LINE(" |\n");
    }
  }

  return true;
}

QueryRunner *QueryRunner::Create(QueryNode *child_node) {
  QueryRunnerFactory factory(child_node);
  return factory.Create();
}

void QueryRunner::printClose() {
  DEBUG_MSG_SINGLE_LINE(
      "+" << std::string(19 * fields_printed_, '-') << "+");
  DEBUG_MSG_SINGLE_LINE("\n");
}
