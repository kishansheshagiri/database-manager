#ifndef SRC_LQP_DROP_TABLE_STATEMENT_H
#define SRC_LQP_DROP_TABLE_STATEMENT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class DropTableStatement : public Statement {
 public:
  DropTableStatement(const SqlNode *root_node);
  virtual ~DropTableStatement();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;
};

#endif // SRC_LQP_DROP_TABLE_STATEMENT_H
