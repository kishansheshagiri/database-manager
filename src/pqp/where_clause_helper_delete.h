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

  bool Initialize(SqlNode *where_node, std::string table_name = "");
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code) override;

 private:
  virtual std::string HandleColumnName(SqlNode *column_name) const override;

  std::string table_name_;
  Tuple *current_tuple_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_DELETE_H_
