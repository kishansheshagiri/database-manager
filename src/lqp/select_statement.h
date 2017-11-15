#ifndef SRC_LQP_SELECT_STATEMENT_H
#define SRC_LQP_SELECT_STATEMENT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class SelectStatement : public Statement {
 public:
  SelectStatement(const SqlNode *root_node);
  virtual ~SelectStatement();

  void Execute(SqlErrors::Type& error_code) override;
};

#endif // SRC_LQP_SELECT_STATEMENT_H
