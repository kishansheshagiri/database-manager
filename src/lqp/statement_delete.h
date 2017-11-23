#ifndef SRC_LQP_STATEMENT_DELETE_H
#define SRC_LQP_STATEMENT_DELETE_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementDelete : public Statement {
 public:
  StatementDelete(const SqlNode *root_node);
  virtual ~StatementDelete();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;
};

#endif // SRC_LQP_STATEMENT_DELETE_H
