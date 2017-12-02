#include "pqp/query_runner_factory.h"

#include "pqp/query_runner_projection.h"
#include "pqp/query_runner_scan.h"
#include "pqp/query_runner_selection.h"

QueryRunnerFactory::QueryRunnerFactory(QueryNode *query_node)
    : query_node_(query_node) {
}

QueryRunner *QueryRunnerFactory::Create() const {
  if (query_node_ == nullptr) {
    return nullptr;
  }

  QueryNode::QueryNodeType node_type = query_node_->Type();
  switch (node_type) {
    case QueryNode::QUERY_NODE_TYPE_TABLE_SCAN:
      return new QueryRunnerScan(query_node_);
    case QueryNode::QUERY_NODE_TYPE_PROJECTION:
      return new QueryRunnerProjection(query_node_);
    case QueryNode::QUERY_NODE_TYPE_SELECTION:
      return new QueryRunnerSelection(query_node_);
    default:
      return nullptr;
  }
}
