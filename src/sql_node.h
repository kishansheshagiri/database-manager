#ifndef SRC_SQL_NODE_H
#define SRC_SQL_NODE_H

#include <string>
#include <vector>

class SqlNode {
 public:
  enum NodeType {
    NODE_TYPE_STATEMENT = 0,
    NODE_TYPE_CREATE_TABLE_STATEMENT,
    NODE_TYPE_DROP_TABLE_STATEMENT,
    NODE_TYPE_SELECT_STATEMENT,
    NODE_TYPE_DELETE_STATEMENT,
    NODE_TYPE_INSERT_STATEMENT,

    NODE_TYPE_TABLE_LIST,
    NODE_TYPE_TABLE_NAME,

    NODE_TYPE_ATTRIBUTE_TYPE_LIST,
    NODE_TYPE_ATTRIBUTE_NAME,
    NODE_TYPE_DATA_TYPE,

    NODE_TYPE_DISTINCT,
    NODE_TYPE_SELECT_LIST,
    NODE_TYPE_COLUMN_NAME,

    NODE_TYPE_ATTRIBUTE_LIST,
    NODE_TYPE_INSERT_TUPLES,
    NODE_TYPE_VALUE_LIST,

    NODE_TYPE_SEARCH_CONDITION,
    NODE_TYPE_BOOLEAN_TERM,
    NODE_TYPE_BOOLEAN_FACTOR,
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_TERM
  };

  SqlNode(NodeType type, std::string value = "");
  virtual ~SqlNode();

  void AppendChild(SqlNode* child);
  void RemoveChild(SqlNode* child);
  SqlNode *Child(int index);
  void RemoveAllChildren();
  std::vector<SqlNode *> Children();

  NodeType Type();
  void SetType(NodeType type);
  std::string Value();
  void SetValue(const std::string value);

 private:
  NodeType type_;
  std::string value_;
  std::vector<SqlNode *> children_;
};

#endif // SRC_SQL_NODE_H
