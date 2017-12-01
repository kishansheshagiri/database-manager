#include "pqp/query_runner_factory.h"

#include "pqp/query_runner_projection.h"

QueryRunnerFactory::QueryRunnerFactory(QueryNode *query_node)
    : query_node_(query_node) {
}

QueryRunner *QueryRunnerFactory::Create() const {
  if (query_node_ == nullptr) {
    return nullptr;
  }

  QueryNode::QueryNodeType node_type = query_node_->Type();
  switch (node_type) {
    case QueryNode::QUERY_NODE_TYPE_PROJECTION:
      return new QueryRunnerProjection(query_node_);
    default:
      return nullptr;
  }
}
