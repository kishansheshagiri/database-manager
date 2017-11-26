#ifndef SRC_LQP_QUERY_NODE_H
#define SRC_LQP_QUERY_NODE_H

#include <vector>

class QueryNode {
 public:
  enum QueryNodeType {
    QUERY_NODE_TYPE_TABLE_SCAN = 0,

    QUERY_NODE_TYPE_SORT = 10,
    QUERY_NODE_TYPE_DUPLICATE_ELIMINATION,

    QUERY_NODE_TYPE_PROJECTION = 20,
    QUERY_NODE_TYPE_SELECTION,

    QUERY_NODE_TYPE_CROSS_PRODUCT = 30,
    QUERY_NODE_TYPE_NATURAL_JOIN
  };

  QueryNode(QueryNodeType type);
  virtual ~QueryNode();

  void AppendChild(QueryNode* child);
  void RemoveChild(QueryNode* child);
  QueryNode *Child(int index) const;
  void RemoveAllChildren();
  std::vector<QueryNode *> Children() const;
  int ChildrenCount() const;

  QueryNodeType Type() const;
  void SetType(QueryNodeType type);

 private:
  QueryNode();

  QueryNodeType type_;
  std::vector<QueryNode *> children_;
};

#endif // SRC_LQP_QUERY_NODE_H
