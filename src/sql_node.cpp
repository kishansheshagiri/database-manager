#include "sql_node.h"

#include <algorithm>

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

void SqlNode::RemoveAllChildren() {
  children_.clear();
}

std::vector<SqlNode *> SqlNode::Children() {
  return children_;
}
