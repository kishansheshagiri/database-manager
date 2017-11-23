#ifndef SRC_LQP_STATEMENT_INSERT_H
#define SRC_LQP_STATEMENT_INSERT_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementInsert : public Statement {
 public:
  StatementInsert(const SqlNode *root_node);
  virtual ~StatementInsert();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;

 private:
  std::vector<std::string> field_names_;
  std::vector<std::string> values_;
};

#endif // SRC_LQP_STATEMENT_INSERT_H
