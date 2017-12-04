#include "pqp/query_runner_selection.h"

#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"

QueryRunnerSelection::QueryRunnerSelection(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerSelection::~QueryRunnerSelection() {
}

bool QueryRunnerSelection::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SELECTION;
    return false;
  }

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));
  return ChildRunner()->Initialize(error_code);
}

bool QueryRunnerSelection::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerSelection::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_SELECTION;
    }

    error_code = error_code_;
    return false;
  }

  return true;
}

bool QueryRunnerSelection::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  WhereClauseHelperSelect *where_helper;
  if (!Node()->WhereHelper(where_helper) || where_helper == nullptr) {
    DEBUG_MSG("");
    return false;
  }

  if (tuples.empty()) {
    return true;
  }

  auto tuple_index = tuples.begin();
  while (tuple_index != tuples.end()) {
    if (!where_helper->Evaluate(&*tuple_index, error_code_)) {
      tuple_index = tuples.erase(tuple_index);
    } else {
      tuple_index++;
    }
  }

  if (!Callback()(this, tuples)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}
