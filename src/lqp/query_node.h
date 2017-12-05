#ifndef SRC_LQP_QUERY_NODE_H
#define SRC_LQP_QUERY_NODE_H

#include <string>
#include <vector>

class WhereClauseHelperSelect;

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

  void SetTableName(const std::string& table_name);
  void SetSortColumn(const std::string& sort_column);
  void SetSelectList(const std::vector<std::string>& select_list);
  void SetWhereHelper(WhereClauseHelperSelect *where_helper);
  void SetSortForJoin(bool sort_for_join);

  bool TableName(std::string& table_name);
  bool SortColumn(std::string& sort_column);
  bool SelectList(std::vector<std::string>& select_list);
  bool WhereHelper(WhereClauseHelperSelect *&helper);
  bool SortForJoin();

 private:
  std::string sort_column_;
  std::string table_name_;
  std::vector<std::string> select_list_;
  WhereClauseHelperSelect *where_helper_;
  bool sort_for_join_;

  QueryNodeType type_;
  std::vector<QueryNode *> children_;
};

#endif // SRC_LQP_QUERY_NODE_H
