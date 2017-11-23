#ifndef SRC_LQP_STATEMENT_CREATE_TABLE_H
#define SRC_LQP_STATEMENT_CREATE_TABLE_H

#include <string>
#include <vector>

#include "lqp/statement.h"
#include "parser/sql_node.h"
#include "storage/storage_manager_headers.h"

class StatementCreateTable : public Statement {
 public:
  StatementCreateTable(const SqlNode *root_node);
  virtual ~StatementCreateTable();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;

 private:
  std::vector<std::string> attribute_names_;
  std::vector<FIELD_TYPE> attribute_types_;
};

#endif // SRC_LQP_STATEMENT_CREATE_TABLE_H
