#ifndef SRC_PQP_QUERY_RUNNER_SORT_H
#define SRC_PQP_QUERY_RUNNER_SORT_H

#include <vector>

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerSort : public QueryRunner {
 public:
  QueryRunnerSort(QueryNode *query_node);
  ~QueryRunnerSort();

  bool Initialize(SqlErrors::Type& error_code) final;
  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  void PassScanParams(ScanParams params) final;
  bool HasSortNode() const final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;

  std::string SortColumn() const;

private:
  bool createIntermediateRelation(Tuple& tuple);
  bool sublistIterated(std::vector<int>& sublist_size_list,
      std::vector<int>& sublist_block_indices);

  std::string column_name_;
  std::string intermediate_relation_name_;
  int tuples_per_block_;
  std::vector<int> sublist_size_list_;

  int memory_constraint_;
  bool scan_params_passed_;
  std::string sort_column_;
  ScanParams scan_params_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_SORT_H
