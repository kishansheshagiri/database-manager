#ifndef SRC_LQP_STATEMENT_SELECT_H
#define SRC_LQP_STATEMENT_SELECT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementSelect : public Statement {
 public:
  StatementSelect(const SqlNode *root_node);
  virtual ~StatementSelect();

  void Execute(SqlErrors::Type& error_code) override;

 private:
  bool validateLists(SqlErrors::Type& error_code) const;

  bool distinct_;
  std::vector<std::string> select_list_;
  std::vector<std::string> table_list_;
  SqlNode *where_node_;
  std::string sort_column_;
};

#endif // SRC_LQP_STATEMENT_SELECT_H
