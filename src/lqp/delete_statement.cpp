#include "lqp/delete_statement.h"

#include "base/debug.h"
#include "parser/where_clause_helper.h"

DeleteStatement::DeleteStatement(const SqlNode *root_node)
  : Statement(root_node) {
}

DeleteStatement::~DeleteStatement() {
}

void DeleteStatement::Execute(SqlErrors::Type& error_code) {
  if (RootNode() == nullptr ||
      RootNode()->Type() != SqlNode::NODE_TYPE_DELETE_STATEMENT) {
    DEBUG_MSG("");
    error_code = SqlErrors::UNACCEPTABLE_STATEMENT;
    return;
  }

  std::string table_name;
  if (!RootNode()->Child(0)->TableName(table_name)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_TABLE_NAME;
    return;
  }

  if (RootNode()->Child(1) == nullptr) {
    if (!Storage()->DeleteAllTuples(table_name)) {
      error_code = SqlErrors::INEXISTENT_TABLE;
    }

    return;
  }

  if (!RootNode()->Child(1)->ValidateSearchCondition()) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }
}

bool DeleteStatement::SetTables(const std::vector<std::string> tables) {
  if (tables.size() != 1) {
    DEBUG_MSG("");
    return false;
  }

  return Statement::SetTables(tables);
}
