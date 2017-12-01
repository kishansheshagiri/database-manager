#include "pqp/query_runner.h"

#include "pqp/query_runner_factory.h"

QueryRunner::QueryRunner(QueryNode *query_node)
  : query_node_(query_node),
    storage_adapter_(StorageAdapter::Get()) {
}

bool QueryRunner::Start(SqlErrors::Type& error_code) {
  return false;
}

QueryRunner *QueryRunner::Create(QueryNode *child_node) {
  QueryRunnerFactory factory(child_node);
  return factory.Create();
}
