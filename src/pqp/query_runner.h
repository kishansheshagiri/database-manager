#ifndef SRC_PQP_QUERY_RUNNER_H
#define SRC_PQP_QUERY_RUNNER_H

#include <functional>

#include "base/sql_errors.h"
#include "lqp/query_node.h"
#include "storage/storage_adapter.h"
#include "storage/storage_manager_headers.h"

class QueryRunner;

typedef std::function<bool(QueryRunner *,
    std::vector<Tuple>&)> QueryResultCallback;

typedef struct ScanParams {
  ScanParams()
    : one_pass_(true),
      multi_scan_(true),
      use_begin_blocks_(true),
      start_index_(0),
      num_blocks_(-1) {}
  bool one_pass_;
  bool multi_scan_;
  bool use_begin_blocks_;
  int start_index_;
  int num_blocks_;
} ScanParams;

class QueryRunner {
 public:
  QueryRunner(QueryNode *query_node);
  virtual ~QueryRunner();

  bool Start(SqlErrors::Type& error_code);
  bool Print(QueryRunner *child, std::vector<Tuple>& tuples);
  QueryNode::QueryNodeType NodeType() const { return Node()->Type(); }

  virtual bool Run(QueryResultCallback callback,
      SqlErrors::Type& error_code) = 0;
  virtual void PassScanParams(ScanParams params);
  virtual bool TableName(std::string& table_name);
  virtual bool TableSize(int& blocks, int& tuples);
  virtual bool ResultCallback(QueryRunner *child,
      std::vector<Tuple>& tuples) = 0;

  virtual void DeleteTemporaryRelations();

 protected:
  QueryNode *Node() const { return query_node_; }
  StorageAdapter *Storage() const { return storage_adapter_; }
  QueryRunner *ChildRunner() const { return child_runner_; }
  QueryResultCallback Callback() const { return callback_; }

  void SetChildRunner(QueryRunner *child_runner);
  void SetCallback(QueryResultCallback callback);

  QueryRunner *Create(QueryNode *child_node);

  void MarkTemporaryRelation(std::string relation_name);
  bool MergeTableHeaders(std::vector<Tuple>& first,
      std::string table_name_first, std::vector<Tuple>& second,
      std::string table_name_second, std::vector<Tuple>& merged_tuples);

 private:
  void printClose();

  QueryNode *query_node_;
  QueryRunner *child_runner_;
  QueryResultCallback callback_;

  std::vector<std::string> temporary_relations_;

  int fields_printed_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PQP_QUERY_RUNNER_H
