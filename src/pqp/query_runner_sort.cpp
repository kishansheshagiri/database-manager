#include "pqp/query_runner_sort.h"

#include <string>
#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"
#include "storage/storage_manager_headers.h"

QueryRunnerSort::QueryRunnerSort(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerSort::~QueryRunnerSort() {
}

bool QueryRunnerSort::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SORT;
    return false;
  }

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));
  return ChildRunner()->Initialize(error_code);
}

bool QueryRunnerSort::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  return ChildRunner()->Run(
      std::bind(&QueryRunnerSort::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code);
}

void QueryRunnerSort::PassScanParams(ScanParams params) {
  scan_params_ = params;
}

bool QueryRunnerSort::HasSortNode() const {
  return true;
}

bool QueryRunnerSort::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  DEBUG_MSG("EMPTY FUNCTION");
  return Callback()(this, tuples);
}
