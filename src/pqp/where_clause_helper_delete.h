#ifndef SRC_PQP_WHERE_CLAUSE_HELPER_DELETE_H_
#define SRC_PQP_WHERE_CLAUSE_HELPER_DELETE_H_

#include <string>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "pqp/where_clause_helper.h"
#include "storage/storage_manager_headers.h"

class StorageAdapter;

class WhereClauseHelperDelete : public WhereClauseHelper {
 public:
  WhereClauseHelperDelete();
  ~WhereClauseHelperDelete();

  bool Initialize(SqlNode *where_node, std::string table_name);
  bool Execute(SqlErrors::Type& error_code);

 private:
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code) override;
  virtual std::string HandleColumnName(SqlNode *column_name) override;

  SqlErrors::Type error_code_;
  std::string table_name_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_DELETE_H_
