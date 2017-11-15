#ifndef SRC_LQP_DELETE_STATEMENT_H
#define SRC_LQP_DELETE_STATEMENT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class DeleteStatement : public Statement {
 public:
  DeleteStatement(const SqlNode *root_node);
  virtual ~DeleteStatement();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;
};

#endif // SRC_LQP_DELETE_STATEMENT_H
