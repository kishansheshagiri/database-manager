#ifndef SRC_PARSER_WHERE_CLAUSE_HELPER_H_
#define SRC_PARSER_WHERE_CLAUSE_HELPER_H_

#include <string>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "storage/storage_manager_headers.h"

class StorageAdapter;

class WhereClauseHelper {
 public:
  WhereClauseHelper();
  ~WhereClauseHelper();

  bool Initialize(const SqlNode *where_node, std::string table_name = "");
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code);

 private:
  bool handleSearchCondition() const;
  bool handleBooleanTerm(const SqlNode *boolean_term) const;
  bool handleBooleanFactor(const SqlNode *boolean_factor) const;
  std::string handleExpression(SqlNode *expression) const;
  std::string handleTerm(SqlNode *term) const;
  std::string handleColumnName(SqlNode *column_name) const;

  const SqlNode *where_node_;
  std::string table_name_;
  Tuple *current_tuple_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PARSER_WHERE_CLAUSE_HELPER_H_
