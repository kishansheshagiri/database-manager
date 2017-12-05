#include "pqp/query_runner_duplicate_elimination.h"

#include <vector>

#include "base/debug.h"
#include "pqp/tuple_helper.h"

QueryRunnerDuplicateElimination::QueryRunnerDuplicateElimination(
      QueryNode *query_node)
  : QueryRunner(query_node),
    previous_tuple_(Tuple::getDummyTuple()),
    error_code_(SqlErrors::NO_ERROR) {
  previous_tuple_.null();
}

QueryRunnerDuplicateElimination::~QueryRunnerDuplicateElimination() {
}

bool QueryRunnerDuplicateElimination::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_DUPLICATE_ELIMINATION;
    return false;
  }

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));
  return ChildRunner()->Initialize(error_code);
}

bool QueryRunnerDuplicateElimination::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerDuplicateElimination::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_DUPLICATE_ELIMINATION;
    }

    error_code = error_code_;
    return false;
  }

  return true;
}

bool QueryRunnerDuplicateElimination::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  if (tuples.empty()) {
    return true;
  }

  std::vector<Tuple> output_tuples;
  for (auto tuple : tuples) {
    if (!previous_tuple_.isNull() && !(tuple == previous_tuple_)) {
      output_tuples.push_back(tuple);
    }

    previous_tuple_ = tuple;
  }

  if (!output_tuples.empty() && !Callback()(this, output_tuples)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}
