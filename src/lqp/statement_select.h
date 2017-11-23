#ifndef SRC_LQP_STATEMENT_SELECT_H
#define SRC_LQP_STATEMENT_SELECT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementSelect : public Statement {
 public:
  StatementSelect(const SqlNode *root_node);
  virtual ~StatementSelect();

  void Execute(SqlErrors::Type& error_code) override;
};

#endif // SRC_LQP_STATEMENT_SELECT_H
