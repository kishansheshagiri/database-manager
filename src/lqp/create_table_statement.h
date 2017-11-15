#ifndef SRC_LQP_CREATE_TABLE_STATEMENT_H
#define SRC_LQP_CREATE_TABLE_STATEMENT_H

#include <string>
#include <vector>

#include "lqp/statement.h"
#include "parser/sql_node.h"
#include "storage/storage_manager_headers.h"

class CreateTableStatement : public Statement {
 public:
  CreateTableStatement(const SqlNode *root_node);
  virtual ~CreateTableStatement();

  void Execute(SqlErrors::Type& error_code) override;

 protected:
  virtual bool SetTables(const std::vector<std::string> tables) override;

 private:
  std::vector<std::string> attribute_names_;
  std::vector<FIELD_TYPE> attribute_types_;
};

#endif // SRC_LQP_CREATE_TABLE_STATEMENT_H
