#ifndef SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_
#define SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_

#include <string>
#include <utility>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "pqp/where_clause_helper.h"
#include "storage/storage_manager_headers.h"

typedef std::vector<std::pair<std::string, std::string> > JoinAttributes;

class StorageAdapter;

class WhereClauseHelperSelect : public WhereClauseHelper {
 public:
  WhereClauseHelperSelect();
  ~WhereClauseHelperSelect();

  bool Initialize(SqlNode *where_node,
      const std::vector<std::string> table_list);
  bool Evaluate(Tuple *tuple, SqlErrors::Type& error_code) override;

  void OptimizationCandidates(std::vector<SqlNode *>& boolean_factors,
      JoinAttributes& join_attributes) const;

 private:
  virtual std::string HandleColumnName(SqlNode *column_name) override;

  bool isValidSearchCondition() const;
  bool isValidBooleanTerm(SqlNode *boolean_term) const;
  bool isValidBooleanFactor(SqlNode *boolean_factor) const;
  bool isValidExpression(SqlNode *expression) const;
  bool isValidTerm(SqlNode *term) const;
  bool isValidColumnName(SqlNode *column_name) const;

  void optimizationCandidatesBooleanTerm(SqlNode *boolean_term,
      std::vector<SqlNode *>& boolean_factors,
      JoinAttributes& join_attributes) const;
  void optimizationCandidatesBooleanFactor(
      SqlNode *boolean_factor, JoinAttributes& join_attributes,
      bool& joinable, bool& optimizable) const;
  bool tryJoinExpression(SqlNode *expression,
      std::string& join_candidate, bool& has_column) const;

  SqlErrors::Type error_code_;
  std::vector<std::string> table_list_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_
