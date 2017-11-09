#ifndef SRC_QUERY_PLAN_STATEMENT_H
#define SRC_QUERY_PLAN_STATEMENT_H

#include <string>
#include <vector>

#include "parser/sql_node.h"

class Statement {
 public:
  Statement(SqlNode *root_node);
  virtual ~Statement();

  std::vector<std::string> Tables() const;
  void SetTables(const std::vector<std::string> names);

 protected:
  std::vector<std::string> tables_;
  SqlNode *root_node_;

 private:
  Statement() {}
};

#endif // SRC_QUERY_PLAN_STATEMENT_H
