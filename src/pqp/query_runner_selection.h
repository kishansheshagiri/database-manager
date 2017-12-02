#ifndef SRC_PQP_QUERY_RUNNER_SELECTION_H
#define SRC_PQP_QUERY_RUNNER_SELECTION_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerSelection : public QueryRunner {
 public:
  QueryRunnerSelection(QueryNode *query_node);
  ~QueryRunnerSelection();

  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child,
      std::vector<Tuple>& tuples, bool headers) final;

private:
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_SELECTION_H
