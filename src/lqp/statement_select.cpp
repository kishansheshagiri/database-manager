#include "lqp/statement_select.h"

#include <algorithm>

#include "base/debug.h"
#include "base/tokenizer.h"
#include "lqp/query_plan_builder.h"

StatementSelect::StatementSelect(const SqlNode *root_node)
  : Statement(root_node),
    where_node_(nullptr) {
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

  int next_index = distinct_ ? 1 : 0;
  if (!RootNode()->Child(next_index++)->SelectList(select_list_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_SELECT_LIST;
    return;
  }

  if (!RootNode()->Child(next_index++)->TableList(table_list_)) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_TABLE_LIST;
    return;
  }

  SqlNode *next_node = nullptr;
  if (RootNode()->ChildrenCount() > next_index) {
    next_node = RootNode()->Child(next_index);
  }

  if (next_node && next_node->Type() == SqlNode::NODE_TYPE_SEARCH_CONDITION) {
    if (!next_node->ValidateSearchCondition()) {
      DEBUG_MSG("");
      error_code = SqlErrors::INVALID_SEARCH_CONDITION;
      return;
    }

    where_node_ = next_node;
    next_node = nullptr;
    next_index++;
  }

  if (next_node == nullptr && RootNode()->ChildrenCount() > next_index) {
    next_node = RootNode()->Child(next_index);
  }

  if (next_node && next_node->Type() == SqlNode::NODE_TYPE_COLUMN_NAME &&
      !RootNode()->Child(next_index)->ColumnName(sort_column_)) {
    DEBUG_MSG("");
    return;
  }

  if (!validateLists(error_code)) {
    DEBUG_MSG("");
    return;
  }

  if (where_node_ &&
      !where_node_->ValidateSearchCondition()) {
    DEBUG_MSG("");
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }

  QueryPlanBuilder query_plan_builder(distinct_, sort_column_,
      select_list_, table_list_, where_node_);
  if (!query_plan_builder.Build(error_code)) {
    DEBUG_MSG("");
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

  if (table_list_.size() > 1 && !sort_column_.empty() &&
      std::find(select_list_.begin(), select_list_.end(), sort_column_) ==
          select_list_.end() && select_list_[0] != "*") {
    ERROR_MSG("");
    error_code = SqlErrors::INVALID_SORT_ATTRIBUTE;
    return false;
  }

  return true;
}
