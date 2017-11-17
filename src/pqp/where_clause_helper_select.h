#ifndef SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_
#define SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_

#include <string>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "pqp/where_clause_helper.h"
#include "storage/storage_manager_headers.h"

class StorageAdapter;

class WhereClauseHelperSelect : public WhereClauseHelper {
 public:
  WhereClauseHelperSelect();
  ~WhereClauseHelperSelect();

  bool Initialize(SqlNode *where_node,
      const std::vector<std::string> table_list);
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code) override;

 private:
  virtual std::string HandleColumnName(SqlNode *column_name) const override;

  std::vector<std::string> table_list_;
  Tuple *current_tuple_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_
