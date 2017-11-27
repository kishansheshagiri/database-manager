#include "pqp/where_clause_helper_select.h"

#include <algorithm>

#include "base/debug.h"
#include "base/tokenizer.h"
#include "storage/storage_adapter.h"

WhereClauseHelperSelect::WhereClauseHelperSelect()
    : error_code_(SqlErrors::NO_ERROR) {
}

WhereClauseHelperSelect::~WhereClauseHelperSelect() {

}

bool WhereClauseHelperSelect::Initialize(SqlNode *where_node,
    const std::vector<std::string> table_list) {
  table_list_ = table_list;

  return WhereClauseHelper::Initialize(where_node) && isValidSearchCondition();
}

bool WhereClauseHelperSelect::Evaluate(Tuple *tuple,
    SqlErrors::Type& error_code) {
  if (tuple == nullptr) {
    error_code = SqlErrors::UNKNOWN_ERROR;
    DEBUG_MSG("Invalid tuple");
    return false;
  }

  bool condition_result = HandleSearchCondition(tuple);
  if (error_code_ != SqlErrors::NO_ERROR) {
    error_code = error_code_;
    return false;
  }

  return condition_result;
}

void WhereClauseHelperSelect::OptimizationCandidates(
    std::vector<SqlNode *>& boolean_factors,
    JoinAttributes& join_attributes) const {
  boolean_factors.clear();
  join_attributes.clear();

  if (RootNode()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    return;
  }

  std::vector<SqlNode *> children = RootNode()->Children();
  return optimizationCandidatesBooleanTerm(RootNode()->Child(0),
      boolean_factors, join_attributes);
}

// Private methods
std::string WhereClauseHelperSelect::HandleColumnName(
    SqlNode *column_name) {
  std::string column_name_string, table_name, attribute_name;
  column_name->ColumnName(column_name_string);

  Tokenizer::SplitIntoTwo(column_name_string, '.', table_name, attribute_name);
  if (attribute_name.empty()) {
    attribute_name = table_name;
  }

  std::string field_value;
  ValueFromTuple(attribute_name, field_value, error_code_);
  return field_value;
}

bool WhereClauseHelperSelect::isValidSearchCondition() const {
  for (auto boolean_term : RootNode()->Children()) {
    if (!isValidBooleanTerm(boolean_term)) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool WhereClauseHelperSelect::isValidBooleanTerm(SqlNode *boolean_term) const {
  for (auto boolean_factor : boolean_term->Children()) {
    if (!isValidBooleanFactor(boolean_factor)) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool WhereClauseHelperSelect::isValidBooleanFactor(
    SqlNode *boolean_factor) const {
  for (auto expression : boolean_factor->Children()) {
    if (!isValidExpression(expression)) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool WhereClauseHelperSelect::isValidExpression(SqlNode *expression) const {
  for (auto term : expression->Children()) {
    if (!isValidTerm(term)) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool WhereClauseHelperSelect::isValidTerm(SqlNode *term) const {
  if (term->ChildrenCount() == 0) {
    DEBUG_MSG("");
    return true;
  }

  return isValidColumnName(term->Child(0));
}

bool WhereClauseHelperSelect::isValidColumnName(SqlNode *column_node) const {
  if (table_list_.size() > 1 && column_node->ChildrenCount() == 1) {
    DEBUG_MSG("");
    return false;
  }

  std::string column_name;
  if (!column_node->ColumnName(column_name)) {
    DEBUG_MSG("");
    return false;
  }

  std::string table_name, attribute_name;
  Tokenizer::SplitIntoTwo(column_name, '.', table_name, attribute_name);
  if (attribute_name.empty()) {
    attribute_name = table_name;
    table_name.clear();
  }

  auto table_position = std::find(
      table_list_.begin(), table_list_.end(), table_name);
  if (!table_name.empty() && table_position == table_list_.end()) {
    DEBUG_MSG("");
    return false;
  }

  table_name = *table_position;

  return Storage()->IsValidColumnName(table_name, attribute_name);
}

void WhereClauseHelperSelect::optimizationCandidatesBooleanTerm(
    SqlNode *boolean_term,
    std::vector<SqlNode *>& boolean_factors,
    JoinAttributes& join_attributes) const {
  bool joinable = true;
  std::vector<SqlNode *> children = boolean_term->Children();

  for (auto boolean_factor : children) {
    bool node_joinable = false;
    bool node_optimizable = false;
    optimizationCandidatesBooleanFactor(boolean_factor, join_attributes,
        node_joinable, node_optimizable);
    joinable = joinable && (node_joinable || node_optimizable);

    if (node_optimizable) {
      boolean_factors.push_back(boolean_factor);
      boolean_term->RemoveChild(boolean_factor);
    }
  }

  if (!joinable) {
    join_attributes.clear();
  }
}

void WhereClauseHelperSelect::optimizationCandidatesBooleanFactor(
    SqlNode *boolean_factor, JoinAttributes& join_attributes,
    bool& joinable, bool& optimizable) const {
  std::string join_candidate_left;
  bool has_column_left = false;
  bool left_joinable = tryJoinExpression(boolean_factor->Child(0),
      join_candidate_left, has_column_left);

  std::string join_candidate_right;
  bool has_column_right = false;
  bool right_joinable = tryJoinExpression(boolean_factor->Child(1),
      join_candidate_right, has_column_right);

  if (has_column_left != has_column_right) {
    optimizable = true;
  }

  if (boolean_factor->Data() != "=" ||
      !left_joinable || !right_joinable) {
    DEBUG_MSG("");
    return;
  }

  std::string table_name_left, attribute_name_left;
  Tokenizer::SplitIntoTwo(join_candidate_left, '.',
      table_name_left, attribute_name_left);

  std::string table_name_right, attribute_name_right;
  Tokenizer::SplitIntoTwo(join_candidate_right, '.',
      table_name_right, attribute_name_right);

  if (attribute_name_left != attribute_name_right ||
      table_name_left == table_name_right) {
    DEBUG_MSG("");
    return;
  }

  FIELD_TYPE field_type_left;
  Storage()->AttributeType(table_name_left, attribute_name_left,
      field_type_left);
  FIELD_TYPE field_type_right;
  Storage()->AttributeType(table_name_right, attribute_name_right,
      field_type_right);

  if (field_type_left != field_type_right) {
    DEBUG_MSG("");
    return;
  }

  auto join_candidates = std::make_pair(join_candidate_left,
      join_candidate_right);
  join_attributes.push_back(join_candidates);

  joinable = true;
}

bool WhereClauseHelperSelect::tryJoinExpression(SqlNode *expression,
    std::string& join_candidate, bool& has_column) const {
  if (expression->ChildrenCount() != 1) {
    DEBUG_MSG("");
    return false;
  }

  SqlNode *term = expression->Child(0);
  if (term->ChildrenCount() != 1) {
    DEBUG_MSG("");
    return false;
  }

  has_column = true;

  SqlNode *column_name = term->Child(0);
  if (!column_name->ColumnName(join_candidate)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}
