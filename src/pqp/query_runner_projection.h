#ifndef SRC_PQP_QUERY_RUNNER_PROJECTION_H
#define SRC_PQP_QUERY_RUNNER_PROJECTION_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerProjection : public QueryRunner {
 public:
  QueryRunnerProjection(QueryNode *query_node);
  ~QueryRunnerProjection();

  bool Initialize(SqlErrors::Type& error_code) final;
  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;
};

#endif // SRC_PQP_QUERY_RUNNER_PROJECTION_H
