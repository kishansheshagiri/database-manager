#include "lqp/statement_select.h"

#include <algorithm>

#include "base/debug.h"
#include "base/tokenizer.h"

StatementSelect::StatementSelect(const SqlNode *root_node)
  : Statement(root_node) {
}

StatementSelect::~StatementSelect() {
}

void StatementSelect::Execute(SqlErrors::Type& error_code) {
  if (RootNode() == nullptr ||
      RootNode()->Type() != SqlNode::NODE_TYPE_SELECT_STATEMENT) {
    DEBUG_MSG("");
    error_code = SqlErrors::UNACCEPTABLE_STATEMENT;
    return;
  }

  if (RootNode()->Child(0)->Type() == SqlNode::NODE_TYPE_DISTINCT) {
    distinct_ = true;
  }

  if (!RootNode()->Child(distinct_ ? 1 : 0)->SelectList(select_list_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_SELECT_LIST;
    return;
  }

  if (!RootNode()->Child(distinct_ ? 2 : 1)->TableList(table_list_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_TABLE_LIST;
    return;
  }

  if (!validateLists(error_code)) {
    return;
  }

  if (select_list_[0] == "*" && table_list_.size() == 1) {
    TupleList tuples;
    Storage()->Tuples(table_list_[0], tuples);
    Storage()->PrintTupleList(table_list_[0], tuples);
    return;
  }

  error_code = SqlErrors::UNKNOWN_ERROR;
}

bool StatementSelect::validateLists(SqlErrors::Type& error_code) const {
  if (select_list_.size() == 1 && select_list_[0] == "*") {
    return true;
  }

  for (auto column_name : select_list_) {
    std::string table_name, attribute_name;
    Tokenizer::SplitIntoTwo(column_name, '.', table_name, attribute_name);
    if (attribute_name.empty()) {
      attribute_name = table_name;
      table_name.clear();
    }

    if (table_name.empty()) {
      if (table_list_.size() == 1) {
        continue;
      } else {
        DEBUG_MSG("");
        error_code = SqlErrors::AMBIGUOUS_SELECT_LIST;
        return false;
      }
    }

    if (std::find(table_list_.begin(), table_list_.end(), table_name) ==
        table_list_.end()) {
      DEBUG_MSG("");
      error_code = SqlErrors::INVALID_SELECT_LIST;
      return false;
    }
  }

  return true;
}
