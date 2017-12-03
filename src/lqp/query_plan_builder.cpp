#include "lqp/query_plan_builder.h"

#include <algorithm>
#include <utility>

#include "base/debug.h"
#include "base/tokenizer.h"
#include "pqp/query_runner.h"
#include "pqp/query_runner_factory.h"

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
  where_helper_ = new WhereClauseHelperSelect(
      WhereClauseHelperSelect::WHERE_CLAUSE_HELPER_TYPE_WHERE);
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

bool QueryPlanBuilder::Build(SqlErrors::Type& error_code) {
  QueryNode *sort_node = nullptr;
  if (!sort_column_.empty()) {
    sort_node = createNode(nullptr, QueryNode::QUERY_NODE_TYPE_SORT);
    sort_node->SetSortColumn(sort_column_);
  }

  QueryNode *duplication_elimination_node = nullptr;
  if (distinct_) {
    if (sort_node == nullptr) {
      sort_node = createNode(nullptr, QueryNode::QUERY_NODE_TYPE_SORT);
      sort_column_ = select_list_[0];
      sort_node->SetSortColumn(sort_column_);
    }

    duplication_elimination_node = createNode(nullptr,
        QueryNode::QUERY_NODE_TYPE_DUPLICATE_ELIMINATION);
  }

  QueryNode *projection_node = createNode(nullptr,
      QueryNode::QUERY_NODE_TYPE_PROJECTION);
  projection_node->SetSelectList(select_list_);

  std::vector<QueryNode *> nodes = {
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
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return false;
  }

  JoinAttributes join_attributes;
  PushCandidates push_candidates;
  if (where_node_) {
    where_helper_->OptimizationCandidates(push_candidates,
      join_attributes);
  }

  bool joins_created = createJoins(next_node, join_attributes, push_candidates,
      sort_node);

  if (!joins_created) {
    if (where_node_) {
      QueryNode *selection_node = createNode(next_node,
          QueryNode::QUERY_NODE_TYPE_SELECTION);
      selection_node->SetWhereHelper(where_helper_);
      next_node = selection_node;
    }

    if (!createProducts(0, next_node, push_candidates,
        sort_node)) {
      DEBUG_MSG("Failed to create products");
      error_code = SqlErrors::ERROR_SELECTION;
      return false;
    }
  }

  if (sort_node != nullptr) {
    QueryNode *next_child = next_node->Child(0);
    next_node->RemoveChild(next_child);
    next_node->AppendChild(sort_node);
    sort_node->AppendChild(next_child);
    next_node = sort_node;
    sort_node = nullptr;
  }

  if (push_candidates.size() != 0 || sort_node != nullptr) {
    DEBUG_MSG("Push candidates(" << push_candidates.size() <<
        ")/sort node(" << (sort_node != nullptr) << ") not empty");
    error_code = SqlErrors::WHERE_CLAUSE_ERROR;
    return false;
  }

  QueryRunnerFactory factory(query_node_root_);
  QueryRunner *query_runner = factory.Create();
  bool return_value = query_runner->Start(error_code);

  query_runner->DeleteTemporaryRelations();
  delete query_runner;
  return return_value;
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

bool QueryPlanBuilder::createProducts(const int index, QueryNode *parent,
    PushCandidates& push_candidates, QueryNode *&sort_node) {
  if (index >= table_list_.size()) {
    return true;
  } else if (index < 0) {
    DEBUG_MSG("");
    return false;
  }

  if (index == table_list_.size() - 1) {
    std::pair<QueryNode *, QueryNode *> node_endings = std::make_pair(
        nullptr, nullptr);
    createPushCandidateNodes(push_candidates, sort_node,
        table_list_[index], node_endings);
    if (node_endings.first != nullptr) {
      parent->AppendChild(node_endings.first);
    }

    QueryNode *next_child = node_endings.first == nullptr ?
        parent : node_endings.second;

    QueryNode *table_scan_node = createNode(next_child,
        QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
    table_scan_node->SetTableName(table_list_[index]);
    return true;
  }

  QueryNode *product_node = createNode(parent,
      QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT);

  std::pair<QueryNode *, QueryNode *> node_endings = std::make_pair(
      nullptr, nullptr);
  createPushCandidateNodes(push_candidates, sort_node,
      table_list_[index], node_endings);
  if (node_endings.first != nullptr) {
    product_node->AppendChild(node_endings.first);
  }

  QueryNode *next_child = node_endings.first == nullptr ?
      product_node : node_endings.second;

  QueryNode *table_scan_node = createNode(
      next_child, QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
  table_scan_node->SetTableName(table_list_[index]);

  return createProducts(index + 1, product_node, push_candidates, sort_node);
}

bool QueryPlanBuilder::createJoins(QueryNode *parent,
    JoinAttributes join_attributes, PushCandidates& push_candidates,
    QueryNode *&sort_node) {
  if (join_attributes.size() > 1) {
    DEBUG_MSG("Join works with one column name");
    return false;
  }

  if (table_list_.size() != 2) {
    return false;
  }

  std::string join_attribute_name;
  if (!validateJoinAttributes(join_attributes, join_attribute_name)) {
    DEBUG_MSG("Invalid join attributes");
    return false;
  }

  QueryNode *join_node = createNode(parent,
      QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT);

  for (auto table : table_list_) {
    QueryNode *next_node = join_node;
    std::pair<QueryNode *, QueryNode *> node_endings = std::make_pair(
        nullptr, nullptr);

    QueryNode *join_sort_node = nullptr;
    if (sort_node != nullptr) {
      std::string table_name_sort, attribute_name_sort;
      Tokenizer::SplitIntoTwo(sort_column_, '.',
          table_name_sort, attribute_name_sort);
      if (table_name_sort == "*" || (table_name_sort == table &&
          attribute_name_sort == join_attribute_name)) {
        join_sort_node = sort_node;
        sort_node = nullptr;
      }
    }

    if (join_sort_node == nullptr) {
      join_sort_node = createNode(nullptr, QueryNode::QUERY_NODE_TYPE_SORT);
    }

    join_sort_node->SetSortColumn(join_attribute_name);
    createPushCandidateNodes(push_candidates, join_sort_node, table,
        node_endings);
    if (node_endings.first != nullptr) {
      next_node->AppendChild(node_endings.first);
      next_node = node_endings.second;
    }

    QueryNode *table_scan_node = createNode(next_node,
        QueryNode::QUERY_NODE_TYPE_TABLE_SCAN);
    table_scan_node->SetTableName(table);
  }

  return true;
}

bool QueryPlanBuilder::validateJoinAttributes(
    const JoinAttributes join_attributes, std::string& join_attribute_name) {
  if (join_attributes.empty()) {
    DEBUG_MSG("Empty join attributes");
    return false;
  }

  join_attribute_name.clear();
  for (auto candidate_pair : join_attributes) {
    std::string table_name_first, attribute_name_first;
    Tokenizer::SplitIntoTwo(candidate_pair.first, '.',
        table_name_first, attribute_name_first);

    std::string table_name_second, attribute_name_second;
    Tokenizer::SplitIntoTwo(candidate_pair.second, '.',
        table_name_second, attribute_name_second);

    if (table_name_first == table_name_second) {
      DEBUG_MSG("");
      return false;
    }

    if (attribute_name_first != attribute_name_second) {
      DEBUG_MSG("");
      return false;
    }

    if (join_attribute_name.empty()) {
      join_attribute_name = attribute_name_first;
    }

    if (join_attribute_name != attribute_name_first) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

void QueryPlanBuilder::createPushCandidateNodes(PushCandidates& push_candidates,
    QueryNode *&sort_node, const std::string table_name,
    std::pair<QueryNode *, QueryNode *>& node_endings) {
  std::vector<QueryNode *> push_candidate_nodes;
  if (sort_node != nullptr) {
    std::string sort_column_name, sort_table_name, sort_attribute_name;
    if (sort_node->SortColumn(sort_column_name)) {
      Tokenizer::SplitIntoTwo(sort_column_name, '.',
          sort_table_name, sort_attribute_name);
      if (sort_attribute_name.empty()) {
        sort_attribute_name = sort_table_name;
        sort_table_name = table_list_[0];
      }

      if (sort_table_name == table_name || sort_table_name == "*") {
        if (sort_table_name == "*") {
          sort_node->SetSortColumn("*");
        } else {
          sort_node->SetSortColumn(sort_attribute_name);
        }
        push_candidate_nodes.push_back(sort_node);
        sort_node = nullptr;
      }
    }
  }

  auto push_index = push_candidates.begin();
  while (push_index != push_candidates.end()) {
    std::string candidate_table_name, candidate_attribute_name;
    Tokenizer::SplitIntoTwo((*push_index).first, '.',
        candidate_table_name, candidate_attribute_name);
    if (candidate_table_name == table_name) {
      QueryNode *push_candidate_node = createNode(nullptr,
          QueryNode::QUERY_NODE_TYPE_SELECTION);
      WhereClauseHelperSelect *where_helper = new WhereClauseHelperSelect(
          WhereClauseHelperSelect::WHERE_CLAUSE_HELPER_TYPE_BOOLEAN_FACTOR);
      std::vector<std::string> table_list = { table_name };
      where_helper->Initialize((*push_index).second, table_list);
      push_candidate_node->SetWhereHelper(where_helper);
      push_candidate_nodes.push_back(push_candidate_node);
      push_index = push_candidates.erase(push_index);
    } else {
      push_index++;
    }
  }

  serializeNodes(push_candidate_nodes, node_endings);
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
