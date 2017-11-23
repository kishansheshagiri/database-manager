#ifndef SRC_LQP_STATEMENT_DROP_TABLE_H
#define SRC_LQP_STATEMENT_DROP_TABLE_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementDropTable : public Statement {
 public:
  StatementDropTable(const SqlNode *root_node);
  virtual ~StatementDropTable();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;
};

#endif // SRC_LQP_STATEMENT_DROP_TABLE_H
