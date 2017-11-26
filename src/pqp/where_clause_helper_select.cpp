#include "pqp/where_clause_helper_select.h"

#include "base/debug.h"
#include "base/tokenizer.h"
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
    JoinAttributes& join_attributes) const {
  if (RootNode()->ChildrenCount() > 1) {
    DEBUG_MSG("");
    return false;
  }

  join_attributes.clear();
  std::vector<SqlNode *> children = RootNode()->Children();
  return tryJoinBooleanTerm(RootNode()->Child(0), join_attributes);
}

// Private methods
std::string WhereClauseHelperSelect::HandleColumnName(
    SqlNode *column_name) {
  DEBUG_MSG("NOT_IMPLEMENTED");
  return std::string();
}

bool WhereClauseHelperSelect::tryJoinBooleanTerm(SqlNode *boolean_term,
    JoinAttributes& join_attributes) const {
  std::vector<SqlNode *> children = boolean_term->Children();
  bool joinable = tryJoinBooleanFactor(children[0], join_attributes);
  for (int index = 1; index < children.size(); index++) {
    joinable = joinable &&
        tryJoinBooleanFactor(children[index], join_attributes);
  }

  return joinable;
}

bool WhereClauseHelperSelect::tryJoinBooleanFactor(SqlNode *boolean_factor,
    JoinAttributes& join_attributes) const {
  if (boolean_factor->Data() != "=") {
    DEBUG_MSG("");
    return false;
  }

  std::string join_candidate_left;
  bool left_joinable = tryJoinExpression(boolean_factor->Child(0),
      join_candidate_left);
  std::string join_candidate_right;
  bool right_joinable = tryJoinExpression(boolean_factor->Child(1),
      join_candidate_right);

  if (!left_joinable || !right_joinable) {
    DEBUG_MSG("");
    return false;
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
    return false;
  }

  FIELD_TYPE field_type_left;
  Storage()->AttributeType(table_name_left, attribute_name_left,
      field_type_left);
  FIELD_TYPE field_type_right;
  Storage()->AttributeType(table_name_right, attribute_name_right,
      field_type_right);

  if (field_type_left != field_type_right) {
    DEBUG_MSG("");
    return false;
  }

  auto join_candidates = std::make_pair(join_candidate_left,
      join_candidate_right);
  join_attributes.push_back(join_candidates);
  return true;
}

bool WhereClauseHelperSelect::tryJoinExpression(SqlNode *expression,
    std::string& join_candidate) const {
  if (expression->ChildrenCount() != 1) {
    DEBUG_MSG("");
    return false;
  }

  SqlNode *term = expression->Child(0);
  if (term->ChildrenCount() != 1) {
    DEBUG_MSG("");
    return false;
  }

  SqlNode *column_name = term->Child(0);
  if (!column_name->ColumnName(join_candidate)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}
