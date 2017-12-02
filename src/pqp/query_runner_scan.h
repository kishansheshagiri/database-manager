#ifndef SRC_PQP_QUERY_RUNNER_SCAN_H
#define SRC_PQP_QUERY_RUNNER_SCAN_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerScan : public QueryRunner {
 public:
  QueryRunnerScan(QueryNode *query_node);
  ~QueryRunnerScan();

  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  void PassScanParams(ScanParams params) final;
  bool TableSize(int& blocks, int& tuples) final;
  bool ResultCallback(std::vector<Tuple>& tuples, bool headers) final;

private:
  int next_relation_start_index_;
  ScanParams scan_params_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_SCAN_H
