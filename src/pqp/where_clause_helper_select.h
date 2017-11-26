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

  bool CanUseJoin(JoinAttributes& join_attributes) const;

 private:
  virtual std::string HandleColumnName(SqlNode *column_name) override;

  bool tryJoinBooleanTerm(SqlNode *boolean_term,
      JoinAttributes& join_attributes) const;
  bool tryJoinBooleanFactor(SqlNode *boolean_factor,
      JoinAttributes& join_attributes) const;
  bool tryJoinExpression(SqlNode *expression,
      std::string& join_candidate) const;

  std::vector<std::string> table_list_;
  Tuple *current_tuple_;
};

#endif // SRC_PQP_WHERE_CLAUSE_HELPER_SELECT_H_
