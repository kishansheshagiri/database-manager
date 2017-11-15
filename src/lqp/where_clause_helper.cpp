#include "lqp/where_clause_helper.h"

#include "base/debug.h"

WhereClauseHelper::WhereClauseHelper() {

}

WhereClauseHelper::~WhereClauseHelper() {

}

bool WhereClauseHelper::Initialize(const SqlNode *where_node,
    const WhereClauseHelperClient *client, std::string table_name) {
  if (where_node == nullptr || client == nullptr) {
    DEBUG_MSG("Empty values in intialization");
    return false;
  }

  if (where_node->Type() != SqlNode::NODE_TYPE_SEARCH_CONDITION) {
    DEBUG_MSG("Invalid node, check WHERE node type");
    return false;
  }

  where_node_ = where_node;
  table_name_ = table_name;
  client_ = client;

  return isValidSearchCondition();
}

bool WhereClauseHelper::Evaluate(Tuple *tuple,
    SqlErrors::Type& error_code) {
  if (tuple == nullptr) {
    error_code = SqlErrors::UNKNOWN_ERROR;
    DEBUG_MSG("Invalid tuple");
    return false;
  }

  current_tuple_ = tuple;
  return handleSearchCondition();
}

// Private methods
bool WhereClauseHelper::isValidSearchCondition() const {
  std::vector<SqlNode *> children = where_node_->Children();
  if (children.size() == 0) {
    DEBUG_MSG("Search condition has zero children");
    return false;
  }

  for (auto boolean_term : children) {
    if (!isValidBooleanTerm(boolean_term)) {
      return false;
    }
  }

  return true;
}

bool WhereClauseHelper::isValidBooleanTerm(const SqlNode *boolean_term) const {
  std::vector<SqlNode *> children = boolean_term->Children();
  if (children.size() == 0) {
    DEBUG_MSG("Boolean term has zero children");
    return false;
  }

  for (auto boolean_factor : children) {
    if (!isValidBooleanFactor(boolean_factor)) {
      return false;
    }
  }

  return true;
}

bool WhereClauseHelper::isValidBooleanFactor(
    const SqlNode *boolean_factor) const {
  std::vector<SqlNode *> children = boolean_factor->Children();
  if (children.size() != 2) {
    DEBUG_MSG("Boolean factor has invalid number of children");
    return false;
  }

  std::string comp_op = boolean_factor->Data();
  if (comp_op != "<" && comp_op != ">" && comp_op != "=") {
    DEBUG_MSG("Invalid comparison operator");
    return false;
  }

  for (auto expression : children) {
    if (!isValidExpression(boolean_factor)) {
      return false;
    }
  }

  return true;
}

bool WhereClauseHelper::isValidExpression(const SqlNode *expression) const {
  std::vector<SqlNode *> children = expression->Children();
  if (children.size() != 1 || children.size() != 2) {
    DEBUG_MSG("Expression has invalid number of children");
    return false;
  }

  std::string data_operator = expression->Data();
  if (data_operator.empty()) {
    if (children.size() != 1) {
      DEBUG_MSG("Expression should have only 1 child node");
      return false;
    }

    return isValidTerm(children[0]);
  }

  if (data_operator != "+" && data_operator != "-" && data_operator != "*") {
    DEBUG_MSG("Invalid operator in expression");
    return false;
  }

  for (auto term : children) {
    if (!isValidTerm(term)) {
      return false;
    }
  }

  return true;
}

bool WhereClauseHelper::isValidTerm(const SqlNode *term) const {
  std::vector<SqlNode *> children = term->Children();
  if (children.size() == 0) {
    return true;
  }

  if (children.size() != 1) {
    DEBUG_MSG("Expecting only one child node");
    return false;
  }

  return isValidColumnName(term);
}

bool WhereClauseHelper::isValidColumnName(const SqlNode *column_name) const {
  std::vector<SqlNode *> children = column_name->Children();
  if (children.size() != 1 || children.size() != 2) {
    DEBUG_MSG("Column name has invalid number of children");
    return false;
  }

  if (table_name_.empty() && children.size() != 2) {
    ERROR_MSG("WHERE clause contains ambiguous column name/s");
    return false;
  }

  std::string table_name = table_name_;
  std::string attribute_name;
  if (children.size() == 2) {
    table_name = children[0]->Data();
    attribute_name = children[1]->Data();
  } else {
    attribute_name = children[0]->Data();
  }

  return client_->IsValidColumnName(table_name, attribute_name);
}

bool WhereClauseHelper::handleSearchCondition() const {
  std::vector<SqlNode *> children = where_node_->Children();
  bool search_predicate = handleBooleanTerm(children[0]);
  for (int index = 1; index < children.size(); index++) {
    search_predicate = search_predicate || handleBooleanTerm(children[index]);
  }

  return search_predicate;
}

bool WhereClauseHelper::handleBooleanTerm(const SqlNode *boolean_term) const {
  std::vector<SqlNode *> children = boolean_term->Children();
  bool boolean_term_predicate = handleBooleanFactor(children[0]);
  for (int index = 1; index < children.size(); index++) {
    boolean_term_predicate = boolean_term_predicate &&
        handleBooleanFactor(children[index]);
  }

  return boolean_term_predicate;
}

bool WhereClauseHelper::handleBooleanFactor(
    const SqlNode *boolean_factor) const {
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

std::string WhereClauseHelper::handleExpression(SqlNode *expression) const {
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

std::string WhereClauseHelper::handleTerm(SqlNode *term) const {
  std::vector<SqlNode *> children = term->Children();
  if (children.size() == 0) {
    return term->Data();
  }

  return handleColumnName(children[0]);
}

std::string WhereClauseHelper::handleColumnName(SqlNode *column_name) const {
  return std::string();
}
