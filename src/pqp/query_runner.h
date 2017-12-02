#ifndef SRC_PQP_QUERY_RUNNER_H
#define SRC_PQP_QUERY_RUNNER_H

#include <functional>

#include "base/sql_errors.h"
#include "lqp/query_node.h"
#include "storage/storage_adapter.h"
#include "storage/storage_manager_headers.h"

typedef std::function<bool(std::vector<Tuple>&, bool headers)>
    QueryResultCallback;

typedef struct ScanParams {
  ScanParams() : one_pass_(true), use_begin_blocks_(true), start_index_(0) {}
  bool one_pass_;
  bool use_begin_blocks_;
  int start_index_;
} ScanParams;

class QueryRunner {
 public:
  QueryRunner(QueryNode *query_node);
  virtual ~QueryRunner();

  bool Start(SqlErrors::Type& error_code);
  bool Print(std::vector<Tuple>& tuples, bool headers);

  virtual bool Run(QueryResultCallback callback,
      SqlErrors::Type& error_code) = 0;
  virtual void PassScanParams(ScanParams params);
  virtual bool TableSize(int& blocks, int& tuples);
  virtual bool ResultCallback(std::vector<Tuple>& tuples, bool headers) = 0;

 protected:
  QueryNode *Node() const { return query_node_; }
  StorageAdapter *Storage() const { return storage_adapter_; }
  QueryRunner *ChildRunner() const { return child_runner_; }
  QueryResultCallback Callback() const { return callback_; }

  void SetChildRunner(QueryRunner *child_runner);
  void SetCallback(QueryResultCallback callback);

  QueryRunner *Create(QueryNode *child_node);

 private:
  void printClose();

  QueryNode *query_node_;
  QueryRunner *child_runner_;
  QueryResultCallback callback_;

  int fields_printed_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PQP_QUERY_RUNNER_H
