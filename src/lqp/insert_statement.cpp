#include "lqp/insert_statement.h"

#include "base/debug.h"

InsertStatement::InsertStatement(const SqlNode *root_node)
  : Statement(root_node) {
}

InsertStatement::~InsertStatement() {
}

void InsertStatement::Execute(SqlErrors::Type& error_code) {
  DEBUG_MSG("");

  if (RootNode() == nullptr ||
      RootNode()->Type() != SqlNode::NODE_TYPE_INSERT_STATEMENT) {
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

  if (!RootNode()->Child(1)->AttributeList(field_names_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_ATTRIBUTES;
    return;
  }

  if (!RootNode()->Child(2)->InsertTuples(values_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_VALUES;
    return;
  }

  if (!Storage()->CreateTupleAndAppend(table_name,
      field_names_, values_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::TABLE_EXISTS;
  }
}

bool InsertStatement::SetTables(const std::vector<std::string> tables) {
  if (tables.size() != 1) {
    DEBUG_MSG("");
    return false;
  }

  return Statement::SetTables(tables);
}
