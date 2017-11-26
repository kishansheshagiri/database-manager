#include "pqp/where_clause_helper_select.h"

#include "base/debug.h"
#include "storage/storage_adapter.h"

WhereClauseHelperSelect::WhereClauseHelperSelect() {
}

WhereClauseHelperSelect::~WhereClauseHelperSelect() {

}

bool WhereClauseHelperSelect::Initialize(SqlNode *where_node,
    const std::vector<std::string> table_list) {
  table_list_ = table_list;

  return WhereClauseHelper::Initialize(where_node);
}

bool WhereClauseHelperSelect::Evaluate(Tuple *tuple,
    SqlErrors::Type& error_code) {
  if (tuple == nullptr) {
    error_code = SqlErrors::UNKNOWN_ERROR;
    DEBUG_MSG("Invalid tuple");
    return false;
  }

  current_tuple_ = tuple;
  return HandleSearchCondition();
}

bool WhereClauseHelperSelect::CanUseJoin(
    std::vector<std::string>& attribute_list) const {
  DEBUG_MSG("NOT_IMPLEMENTED");
  return false;
}

// Private methods
std::string WhereClauseHelperSelect::HandleColumnName(
    SqlNode *column_name) {
  DEBUG_MSG("NOT_IMPLEMENTED");
  return std::string();
}
