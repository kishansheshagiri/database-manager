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
  where_helper_ = new WhereClauseHelperSelect();
}

QueryPlanBuilder::~QueryPlanBuilder() {
  if (query_node_root_) {
    delete query_node_root_;
    query_node_root_ = nullptr;
  }

  if (where_helper_) {
    delete where_helper_;
    where_helper_ = nullptr;
  }
}

bool QueryPlanBuilder::Build() {
  QueryNode *sort_node = nullptr;
  if (!sort_column_.empty()) {
    sort_node = createNode(nullptr, QueryNode::QUERY_NODE_TYPE_SORT);
    sort_node->SetSortColumn(sort_column_);
  }

  QueryNode *duplication_elimination_node = nullptr;
  if (distinct_) {
    duplication_elimination_node = createNode(nullptr,
        QueryNode::QUERY_NODE_TYPE_DUPLICATE_ELIMINATION);
  }

  QueryNode *projection_node = createNode(nullptr,
      QueryNode::QUERY_NODE_TYPE_PROJECTION);
  projection_node->SetSelectList(select_list_);

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
      !where_helper_->Initialize(where_node_, table_list_)) {
    DEBUG_MSG("");
    return false;
  }

  JoinAttributes join_attributes;
  std::vector<SqlNode *> selection_push_candidates;
  if (where_node_) {
    where_helper_->OptimizationCandidates(selection_push_candidates,
      join_attributes);
  }

  QueryNode *selection_node = createNode(next_node,
      QueryNode::QUERY_NODE_TYPE_SELECTION);
  selection_node->SetWhereHelper(where_helper_);

  return createProducts(0, QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT,
      selection_node);
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

  if (index == table_list_.size() - 1) {
    QueryNode *table_scan_node = createNode(parent,
        QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
    table_scan_node->SetTableName(table_list_[index]);
    return true;
  }

  QueryNode *product_node = createNode(parent, product_type);
  QueryNode *table_scan_node = createNode(
      product_node, QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
  table_scan_node->SetTableName(table_list_[index]);

  return createProducts(index + 1, product_type, product_node);
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
