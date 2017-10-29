#ifndef SRC_SQL_NODE_H
#define SRC_SQL_NODE_H

#include <string>
#include <vector>

class SqlNode {
 public:
  enum NodeType {
    NODE_TYPE_VALUE = 0,
    NODE_TYPE_OPERAND
  };

  SqlNode(NodeType type, std::string value);
  virtual ~SqlNode();

  void AppendChild(SqlNode* child);
  void RemoveChild(SqlNode* child);
  void RemoveAllChildren();
  std::vector<SqlNode *> Children();

 private:
  NodeType type_;
  std::string value_;
  std::vector<SqlNode *> children_;
};

#endif // SRC_SQL_NODE_H
