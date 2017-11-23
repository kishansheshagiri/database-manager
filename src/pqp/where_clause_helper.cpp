#include "pqp/where_clause_helper.h"

#include "base/debug.h"
#include "storage/storage_adapter.h"

WhereClauseHelper::WhereClauseHelper() {
  storage_adapter_ = StorageAdapter::Get();
}

WhereClauseHelper::~WhereClauseHelper() {

}

bool WhereClauseHelper::Initialize(SqlNode *where_node) {
  if (where_node == nullptr) {
    DEBUG_MSG("Empty values in intialization");
    return false;
  }

  if (where_node->Type() != SqlNode::NODE_TYPE_SEARCH_CONDITION) {
    DEBUG_MSG("Invalid node, check WHERE node type");
    return false;
  }

  where_node_ = where_node;

  return true;
}

// Private methods
bool WhereClauseHelper::HandleSearchCondition() {
  std::vector<SqlNode *> children = where_node_->Children();
  bool search_predicate = handleBooleanTerm(children[0]);
  for (int index = 1; index < children.size(); index++) {
    search_predicate = search_predicate || handleBooleanTerm(children[index]);
  }

  return search_predicate;
}

bool WhereClauseHelper::handleBooleanTerm(SqlNode *boolean_term) {
  std::vector<SqlNode *> children = boolean_term->Children();
  bool boolean_term_predicate = handleBooleanFactor(children[0]);
  for (int index = 1; index < children.size(); index++) {
    boolean_term_predicate = boolean_term_predicate &&
        handleBooleanFactor(children[index]);
  }

  return boolean_term_predicate;
}

bool WhereClauseHelper::handleBooleanFactor(
    SqlNode *boolean_factor) {
  std::string expression_left = handleExpression(boolean_factor->Child(0));
  std::string expression_right = handleExpression(boolean_factor->Child(1));

  std::string comp_op = boolean_factor->Data();
  if (comp_op == "<") {
    return expression_left < expression_right;
  } else if (comp_op == ">") {
    return expression_left > expression_right;
  } else {
    return expression_left == expression_right;
  }
}

std::string WhereClauseHelper::handleExpression(SqlNode *expression) {
  std::vector<SqlNode *> children = expression->Children();
  std::string term_left = handleTerm(children[0]);
  if (children.size() == 1) {
    LOG_MSG("Simple expression");
    return term_left;
  }

  std::string term_right = handleTerm(children[1]);
  std::string data_operator = expression->Data();

  long long result;
  if (data_operator == "+") {
    result = std::stol(term_left) + std::stol(term_right);
  } else if (data_operator == "-") {
    result = std::stol(term_left) - std::stol(term_right);
  } else {
    result = std::stol(term_left) * std::stol(term_right);
  }

  return std::to_string(result);
}

std::string WhereClauseHelper::handleTerm(SqlNode *term) {
  std::vector<SqlNode *> children = term->Children();
  if (children.size() == 0) {
    return term->Data();
  }

  return HandleColumnName(children[0]);
}
