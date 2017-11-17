#include "pqp/where_clause_helper_delete.h"

#include "base/debug.h"
#include "storage/storage_adapter.h"

WhereClauseHelperDelete::WhereClauseHelperDelete() {
}

WhereClauseHelperDelete::~WhereClauseHelperDelete() {

}

bool WhereClauseHelperDelete::Initialize(SqlNode *where_node,
    std::string table_name) {
  table_name_ = table_name;

  return WhereClauseHelper::Initialize(where_node);
}

bool WhereClauseHelperDelete::Evaluate(Tuple *tuple,
    SqlErrors::Type& error_code) {
  if (tuple == nullptr) {
    error_code = SqlErrors::UNKNOWN_ERROR;
    DEBUG_MSG("Invalid tuple");
    return false;
  }

  current_tuple_ = tuple;
  return HandleSearchCondition();
}

// Private methods
std::string WhereClauseHelperDelete::HandleColumnName(
    SqlNode *column_name) const {
  ERROR_MSG("NOT_IMPLEMENTED");
  return std::string();
}
