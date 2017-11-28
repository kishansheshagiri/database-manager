#ifndef SRC_PQP_WHERE_CLAUSE_HELPER_H_
#define SRC_PQP_WHERE_CLAUSE_HELPER_H_

#include <string>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "storage/storage_manager_headers.h"

class StorageAdapter;

class WhereClauseHelper {
 public:
  WhereClauseHelper();
  virtual ~WhereClauseHelper();

 protected:
  bool Initialize(SqlNode *where_node);
  virtual bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code) = 0;

  virtual std::string HandleColumnName(SqlNode *column_name) = 0;
  bool HandleSearchCondition(Tuple *tuple);
  virtual bool HandleBooleanFactor(SqlNode *boolean_factor);
  bool ValueFromTuple(const std::string attribute_name,
      std::string& field_value, SqlErrors::Type& error_code);

  StorageAdapter *Storage() const { return storage_adapter_; }
  SqlNode *RootNode() const { return where_node_; }

 private:
  bool handleBooleanTerm(SqlNode *boolean_term);
  std::string handleExpression(SqlNode *expression);
  std::string handleTerm(SqlNode *term);

  SqlNode *where_node_;
  Tuple *current_tuple_;
  StorageAdapter *storage_adapter_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_H_
