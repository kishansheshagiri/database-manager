#ifndef SRC_PQP_QUERY_RUNNER_NATURAL_JOIN_H
#define SRC_PQP_QUERY_RUNNER_NATURAL_JOIN_H

#include "base/sql_errors.h"
#include "pqp/query_runner.h"

class QueryRunnerNaturalJoin : public QueryRunner {
 public:
  QueryRunnerNaturalJoin(QueryNode *query_node);
  ~QueryRunnerNaturalJoin();

  bool Initialize(SqlErrors::Type& error_code) final;
  bool Run(QueryResultCallback callback, SqlErrors::Type& error_code) final;
  bool ResultCallback(QueryRunner *child, std::vector<Tuple>& tuples) final;
  std::string CompareColumn() const final;

  void PassScanParams(ScanParams params) final;
  bool TableName(std::string& table_name) final;
  bool TableSize(int& blocks, int& tuples) final;
  bool HasSortNode() const final;

  void DeleteTemporaryRelations() final;

private:
  bool createIntermediateRelation(Tuple first, Tuple second,
      std::string table_name_first, std::string table_name_second);
  bool mergeTuples(Tuple first, Tuple second, Tuple& merged_tuple);

  std::vector<Tuple> left_tuples_;
  std::vector<Tuple> right_tuples_;
  std::string intermediate_relation_name_;

  QueryRunner *right_child_;
  std::string compare_column_;
  WhereClauseHelperSelect *where_helper_;
  SqlErrors::Type error_code_;
};

#endif // SRC_PQP_QUERY_RUNNER_NATURAL_JOIN_H
