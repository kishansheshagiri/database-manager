#include "lqp/select_statement.h"

#include "base/debug.h"

SelectStatement::SelectStatement(const SqlNode *root_node)
  : Statement(root_node) {
}

SelectStatement::~SelectStatement() {
}

void SelectStatement::Execute(SqlErrors::Type& error_code) {
  if (RootNode() == nullptr ||
      RootNode()->Type() != SqlNode::NODE_TYPE_SELECT_STATEMENT) {
    DEBUG_MSG("");
    error_code = SqlErrors::UNACCEPTABLE_STATEMENT;
    return;
  }

  int first_node_index = 0;
  if (RootNode()->Child(0)->Type() == SqlNode::NODE_TYPE_DISTINCT) {
    first_node_index = 1;
  }

  std::vector<std::string> select_list;
  if (!RootNode()->Child(first_node_index)->SelectList(select_list)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_SELECT_LIST;
    return;
  }

  std::vector<std::string> table_list;
  if (!RootNode()->Child(first_node_index + 1)->TableList(table_list)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_TABLE_LIST;
    return;
  }

  if (select_list[0] == "*" && table_list.size() == 1) {
    TupleList tuples;
    Storage()->Tuples(table_list[0], tuples);
    Storage()->PrintTupleList(table_list[0], tuples);
    return;
  }

  error_code = SqlErrors::UNKNOWN_ERROR;
}
