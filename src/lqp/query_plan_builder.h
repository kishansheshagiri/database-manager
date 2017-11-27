#ifndef SRC_LQP_QUERY_PLAN_BUILDER_H
#define SRC_LQP_QUERY_PLAN_BUILDER_H

#include <string>
#include <vector>

#include "lqp/query_node.h"
#include "parser/sql_node.h"
#include "pqp/where_clause_helper_select.h"

class QueryPlanBuilder {
 public:
  QueryPlanBuilder(bool distinct, std::string sort_column,
      std::vector<std::string> select_list,
      std::vector<std::string> table_list, SqlNode *where_node);
  virtual ~QueryPlanBuilder();

  bool Build();

 private:
  QueryNode *createNode(QueryNode *parent,
      const QueryNode::QueryNodeType product_type);
  bool createProducts(const int index,
      const QueryNode::QueryNodeType product_type, QueryNode *parent);

  void serializeNodes(std::vector<QueryNode *> nodes,
      std::pair<QueryNode *, QueryNode *>& node_endings) const;

  bool distinct_;
  std::string sort_column_;
  std::vector<std::string> select_list_;
  std::vector<std::string> table_list_;
  SqlNode *where_node_;
  QueryNode *query_node_root_;
  WhereClauseHelperSelect *where_helper_;
};

#endif // SRC_LQP_QUERY_PLAN_BUILDER_H
