#ifndef SRC_PQP_QUERY_RUNNER_PRODUCT_H
#define SRC_PQP_QUERY_RUNNER_PRODUCT_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerProduct : public QueryRunner {
 public:
  QueryRunnerProduct(QueryNode *query_node);
  ~QueryRunnerProduct();

  bool Initialize(SqlErrors::Type& error_code) final;
  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;

  void PassScanParams(ScanParams params) final;
  bool TableName(std::string& table_name) final;
  bool TableSize(int& blocks, int& tuples) final;
  bool HasSortNode() const final;

  void DeleteTemporaryRelations() final;

private:
  bool createIntermediateRelation(Tuple first, Tuple second,
      std::string table_name_first, std::string table_name_second);
  bool mergeTuples(Tuple first, Tuple second, Tuple& merged_tuple);

  ScanParams scan_params_;
  std::string intermediate_relation_name_;
  std::vector<Tuple> first_tuples_;

  QueryRunner *table_scan_child_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_PRODUCT_H
