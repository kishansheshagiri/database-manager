#include "sql_node.h"

#include <algorithm>

#include "debug.h"

SqlNode::SqlNode(NodeType type, std::string value)
    : type_(type),
      value_(value) {
}

SqlNode::~SqlNode() {

}

void SqlNode::AppendChild(SqlNode* child) {
  children_.push_back(child);
}

void SqlNode::RemoveChild(SqlNode* child) {
  if (!child) {
    return;
  }

  // TODO(Kishan): We need to use an efficient container which avoid duplicates.
  auto item = std::find(children_.begin(), children_.end(), child);
  if (item != children_.end())
    children_.erase(item);
}

SqlNode *SqlNode::Child(int index) {
  if (index <= 0 || children_.size() < index) {
    DEBUG_MSG("Invalid index");
    return nullptr;
  }

  return children_[index - 1];
}

void SqlNode::RemoveAllChildren() {
  children_.clear();
}

std::vector<SqlNode *> SqlNode::Children() {
  return children_;
}

SqlNode::NodeType SqlNode::Type() {
  return type_;
}

void SqlNode::SetType(NodeType type) {
  type_ = type;
}

std::string SqlNode::Value() {
  return value_;
}

void SqlNode::SetValue(const std::string value) {
  value_ = value;
}
