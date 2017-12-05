#include "lqp/query_node.h"

#include <algorithm>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"

QueryNode::QueryNode(QueryNodeType type)
    : type_(type) {
}

QueryNode::~QueryNode() {
  for (auto child : children_) {
    if (child != nullptr) {
      delete child;
      child = nullptr;
    }
  }
}

void QueryNode::AppendChild(QueryNode* child) {
  if (children_.size() > 1) {
    ERROR_MSG("Exceeded maximum number of children allowed");
    return;
  }

  children_.push_back(child);
}

void QueryNode::RemoveChild(QueryNode* child) {
  if (!child) {
    return;
  }

  // TODO(Kishan): We need to use an efficient container which avoid duplicates.
  auto item = std::find(children_.begin(), children_.end(), child);
  if (item != children_.end())
    children_.erase(item);
}

QueryNode *QueryNode::Child(int index) const {
  if (index < 0 || index >= children_.size()) {
    DEBUG_MSG("Invalid index");
    return nullptr;
  }

  return children_[index];
}

void QueryNode::RemoveAllChildren() {
  children_.clear();
}

std::vector<QueryNode *> QueryNode::Children() const {
  return children_;
}

int QueryNode::ChildrenCount() const {
  return children_.size();
}

QueryNode::QueryNodeType QueryNode::Type() const {
  return type_;
}

void QueryNode::SetType(QueryNodeType type) {
  type_ = type;
}

void QueryNode::SetTableName(const std::string& table_name) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_TABLE_SCAN) {
    DEBUG_MSG("");
    return;
  }

  table_name_ = table_name;
}

void QueryNode::SetSortColumn(const std::string& sort_column) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SORT &&
      type_ != QueryNode::QUERY_NODE_TYPE_NATURAL_JOIN) {
    DEBUG_MSG("");
    return;
  }

  sort_column_ = sort_column;
}

void QueryNode::SetSelectList(const std::vector<std::string>& select_list) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_PROJECTION) {
    DEBUG_MSG("");
    return;
  }

  select_list_ = select_list;
}

void QueryNode::SetWhereHelper(
    WhereClauseHelperSelect *where_helper) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SELECTION &&
      type_ != QueryNode::QUERY_NODE_TYPE_NATURAL_JOIN) {
    DEBUG_MSG("");
    return;
  }

  where_helper_ = where_helper;
}

void QueryNode::SetSortForJoin(bool sort_for_join) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SORT) {
    DEBUG_MSG("");
    return;
  }

  sort_for_join_ = sort_for_join;
}

bool QueryNode::TableName(std::string& table_name) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_TABLE_SCAN) {
    DEBUG_MSG("");
    return false;
  }

  table_name = table_name_;
  return true;
}

bool QueryNode::SortColumn(std::string& sort_column) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SORT &&
      type_ != QueryNode::QUERY_NODE_TYPE_NATURAL_JOIN) {
    DEBUG_MSG("");
    return false;
  }

  sort_column = sort_column_;
  return true;
}

bool QueryNode::SelectList(std::vector<std::string>& select_list) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_PROJECTION) {
    DEBUG_MSG("");
    return false;
  }

  select_list = select_list_;
  return true;
}

bool QueryNode::WhereHelper(WhereClauseHelperSelect *&helper) {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SELECTION &&
      type_ != QueryNode::QUERY_NODE_TYPE_NATURAL_JOIN) {
    DEBUG_MSG("");
    return false;
  }

  helper = where_helper_;
  return true;
}

bool QueryNode::SortForJoin() {
  if (type_ != QueryNode::QUERY_NODE_TYPE_SORT) {
    return false;
  }

  return sort_for_join_;
}
