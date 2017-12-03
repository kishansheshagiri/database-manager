#ifndef SRC_PQP_QUERY_RUNNER_SORT_H
#define SRC_PQP_QUERY_RUNNER_SORT_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerSort : public QueryRunner {
 public:
  QueryRunnerSort(QueryNode *query_node);
  ~QueryRunnerSort();

  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(std::vector<Tuple>& tuples, bool headers) final;

private:
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_SORT_H
