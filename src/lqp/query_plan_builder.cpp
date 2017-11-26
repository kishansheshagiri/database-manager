#include "lqp/query_plan_builder.h"

#include "base/debug.h"

QueryPlanBuilder::QueryPlanBuilder(bool distinct,
    std::string sort_column,
    std::vector<std::string> select_list,
    std::vector<std::string> table_list, SqlNode *where_node)
    : distinct_(distinct),
      sort_column_(sort_column),
      select_list_(select_list),
      table_list_(table_list),
      where_node_(where_node),
      query_node_root_(nullptr) {
}

QueryPlanBuilder::~QueryPlanBuilder() {
  delete query_node_root_;
}

bool QueryPlanBuilder::Build() {
  QueryNode *sort_node = nullptr;
  if (!sort_column_.empty()) {
    sort_node = createNode(nullptr, QueryNode::QUERY_NODE_TYPE_SORT);
  }

  QueryNode *duplication_elimination_node = nullptr;
  if (distinct_) {
    duplication_elimination_node = createNode(nullptr,
        QueryNode::QUERY_NODE_TYPE_DUPLICATE_ELIMINATION);
  }

  QueryNode *projection_node = createNode(nullptr,
      QueryNode::QUERY_NODE_TYPE_PROJECTION);

  std::vector<QueryNode *> nodes = {
      sort_node,
      duplication_elimination_node,
      projection_node
  };
  std::pair<QueryNode *, QueryNode *> node_endings;
  serializeNodes(nodes, node_endings);

  query_node_root_ = node_endings.first;
  QueryNode *next_node = node_endings.second;

  if (where_node_ != nullptr &&
      !where_helper_.Initialize(where_node_, table_list_)) {
    DEBUG_MSG("");
    return false;
  }

  std::vector<std::string> attribute_list;
  if (where_node_ != nullptr && where_helper_.CanUseJoin(attribute_list)) {
    DEBUG_MSG("NOT IMPLEMENTED");
    return false;
  }

  return createProducts(0, QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT,
      projection_node);
}

QueryNode *QueryPlanBuilder::createNode(QueryNode *parent,
    const QueryNode::QueryNodeType product_type) {
  QueryNode *node = new QueryNode(product_type);
  if (parent) {
    LOG_MSG("");
    parent->AppendChild(node);
  }

  return node;
}

bool QueryPlanBuilder::createProducts(const int index,
    const QueryNode::QueryNodeType product_type, QueryNode *parent) {
  if (index >= table_list_.size()) {
    return true;
  } else if (index < 0 ||
      (product_type != QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT &&
      product_type != QueryNode::QUERY_NODE_TYPE_NATURAL_JOIN)) {
    DEBUG_MSG("");
    return false;
  }

  if (parent->ChildrenCount() < 1 || (table_list_.size() == 1 && index == 0)) {
    createNode(parent, QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
    return createProducts(index + 1, product_type, parent);
  } else if (parent->ChildrenCount() == 1) {
    if (index == table_list_.size() - 1) {
      createNode(parent, QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
      return true;
    } else {
      return createProducts(index, product_type, createNode(parent,
                                                            product_type));
    }
  } else {
    DEBUG_MSG("");
    return false;
  }
}

void QueryPlanBuilder::serializeNodes(std::vector<QueryNode *> nodes,
    std::pair<QueryNode *, QueryNode *>& node_endings) const {
  if (nodes.size() == 0) {
    return;
  }

  nodes.erase(std::remove(nodes.begin(), nodes.end(), nullptr), nodes.end());
  QueryNode *previous_node = nullptr;
  for (auto node : nodes) {
    if (previous_node != nullptr) {
      previous_node->AppendChild(node);
    }

    previous_node = node;
  }

  node_endings = std::make_pair(*(nodes.begin()), *(nodes.end() - 1));
}
