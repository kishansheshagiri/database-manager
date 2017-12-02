#ifndef SRC_PQP_QUERY_RUNNER_PRODUCT_H
#define SRC_PQP_QUERY_RUNNER_PRODUCT_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerProduct : public QueryRunner {
 public:
  QueryRunnerProduct(QueryNode *query_node);
  ~QueryRunnerProduct();

  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child,
      std::vector<Tuple>& tuples, bool headers) final;

private:

  QueryRunner *table_scan_child_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_PRODUCT_H
