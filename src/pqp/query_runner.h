#ifndef SRC_PQP_QUERY_RUNNER_H
#define SRC_PQP_QUERY_RUNNER_H

#include <functional>

#include "base/sql_errors.h"
#include "lqp/query_node.h"
#include "storage/storage_adapter.h"
#include "storage/storage_manager_headers.h"

typedef std::function<bool(std::vector<Tuple>&, bool headers)> QueryResultCallback;

class QueryRunner {
 public:
  QueryRunner(QueryNode *query_node);
  virtual ~QueryRunner();

  bool Start(SqlErrors::Type& error_code);
  bool Print(std::vector<Tuple>& tuples, bool headers);

  virtual bool Run(QueryResultCallback callback,
      SqlErrors::Type& error_code) = 0;
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

  int fields_printed_;
  QueryNode *query_node_;
  QueryRunner *child_runner_;
  QueryResultCallback callback_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PQP_QUERY_RUNNER_H
