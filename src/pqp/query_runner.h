#ifndef SRC_PQP_QUERY_RUNNER_H
#define SRC_PQP_QUERY_RUNNER_H

#include <functional>

#include "base/sql_errors.h"
#include "lqp/query_node.h"
#include "storage/storage_adapter.h"
#include "storage/storage_manager_headers.h"

typedef std::function<bool(std::vector<Tuple>&)> QueryResultCallback;

class QueryRunner {
 public:
  QueryRunner(QueryNode *query_node);
  virtual ~QueryRunner() {}

  bool Start(SqlErrors::Type& error_code);

  virtual bool Run(QueryResultCallback callback,
      SqlErrors::Type& error_code) = 0;
  virtual bool ResultCallback(std::vector<Tuple>& tuples) = 0;

 protected:
  QueryNode *Node() const { return query_node_; }
  StorageAdapter *Storage() const { return storage_adapter_; }
  QueryRunner *Create(QueryNode *child_node);

 private:
  QueryNode *query_node_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PQP_QUERY_RUNNER_H
