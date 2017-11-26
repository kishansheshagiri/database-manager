#include "lqp/query_node.h"

#include "base/debug.h"

QueryNode::QueryNode(QueryNodeType type)
    : type_(type) {
}

QueryNode::~QueryNode() {
  for (auto child : children_) {
    delete child;
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
