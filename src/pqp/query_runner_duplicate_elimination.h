#ifndef SRC_PQP_QUERY_RUNNER_DUPLICATE_ELIMINATION_H
#define SRC_PQP_QUERY_RUNNER_DUPLICATE_ELIMINATION_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerDuplicateElimination : public QueryRunner {
 public:
  QueryRunnerDuplicateElimination(QueryNode *query_node);
  ~QueryRunnerDuplicateElimination();

  bool Initialize(SqlErrors::Type& error_code) final;
  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;

private:
  Tuple previous_tuple_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_DUPLICATE_ELIMINATION_H
