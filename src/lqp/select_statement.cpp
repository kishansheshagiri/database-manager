#include "lqp/select_statement.h"

#include "base/debug.h"

SelectStatement::SelectStatement(const SqlNode *root_node)
  : Statement(root_node) {
}

SelectStatement::~SelectStatement() {
}

void SelectStatement::Execute(SqlErrors::Type& error_code) {
  DEBUG_MSG("");
}
