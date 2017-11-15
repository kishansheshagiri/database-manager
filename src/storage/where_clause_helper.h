#ifndef SRC_STORAGE_WHERE_CLAUSE_HELPER_H_
#define SRC_STORAGE_WHERE_CLAUSE_HELPER_H_

#include <string>

#include "parser/sql_node.h"

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

 private:
  bool isValidSearchCondition() const;
  bool isValidBooleanTerm(const SqlNode *boolean_term) const;
  bool isValidBooleanFactor(const SqlNode *boolean_factor) const;
  bool isValidExpression(const SqlNode *expression) const;
  bool isValidTerm(const SqlNode *term) const;
  bool isValidColumnName(const SqlNode *column_name) const;

  const SqlNode *where_node_;
  std::string table_name_;
  const WhereClauseHelperClient *client_;
};

#endif // SRC_STORAGE_WHERE_CLAUSE_HELPER_H_
