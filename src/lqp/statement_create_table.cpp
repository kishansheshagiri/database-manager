#include "lqp/statement_create_table.h"

#include "base/debug.h"

StatementCreateTable::StatementCreateTable(const SqlNode *root_node)
    : Statement(root_node) {
}

StatementCreateTable::~StatementCreateTable() {
}

void StatementCreateTable::Execute(SqlErrors::Type& error_code) {
  if (RootNode() == nullptr ||
      RootNode()->Type() != SqlNode::NODE_TYPE_CREATE_TABLE_STATEMENT) {
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

  if (!RootNode()->Child(1)->AttributeTypeList(
      attribute_names_, attribute_types_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_ATTRIBUTES;
    return;
  }

  if (!Storage()->CreateRelation(table_name,
      attribute_names_, attribute_types_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::TABLE_EXISTS;
  }
}

bool StatementCreateTable::SetTables(const std::vector<std::string> tables) {
  if (tables.size() != 1) {
    DEBUG_MSG("");
    return false;
  }

  return Statement::SetTables(tables);
}
