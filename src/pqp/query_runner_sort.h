#ifndef SRC_PQP_QUERY_RUNNER_SORT_H
#define SRC_PQP_QUERY_RUNNER_SORT_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerSort : public QueryRunner {
 public:
  QueryRunnerSort(QueryNode *query_node);
  ~QueryRunnerSort();

  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  void PassScanParams(ScanParams params) final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;

private:
  std::string column_name_;

  ScanParams scan_params_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_SORT_H
