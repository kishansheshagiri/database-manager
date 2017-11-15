#ifndef SRC_LQP_WHERE_CLAUSE_HELPER_H_
#define SRC_LQP_WHERE_CLAUSE_HELPER_H_

#include <string>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "storage/storage_manager_headers.h"

class WhereClauseHelperClient {
 public:
  virtual bool IsValidColumnName(const std::string& table_name,
      const std::string& attribute_name) const = 0;
};

class WhereClauseHelper {
 public:
  WhereClauseHelper();
  ~WhereClauseHelper();

  bool Initialize(const SqlNode *where_node,
      const WhereClauseHelperClient *client,
      std::string table_name = "");
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code);

 private:
  bool isValidSearchCondition() const;
  bool isValidBooleanTerm(const SqlNode *boolean_term) const;
  bool isValidBooleanFactor(const SqlNode *boolean_factor) const;
  bool isValidExpression(const SqlNode *expression) const;
  bool isValidTerm(const SqlNode *term) const;
  bool isValidColumnName(const SqlNode *column_name) const;

  bool handleSearchCondition() const;
  bool handleBooleanTerm(const SqlNode *boolean_term) const;
  bool handleBooleanFactor(const SqlNode *boolean_factor) const;
  std::string handleExpression(SqlNode *expression) const;
  std::string handleTerm(SqlNode *term) const;
  std::string handleColumnName(SqlNode *column_name) const;

  const SqlNode *where_node_;
  std::string table_name_;
  Tuple *current_tuple_;
  const WhereClauseHelperClient *client_;
};

#endif // SRC_LQP_WHERE_CLAUSE_HELPER_H_
