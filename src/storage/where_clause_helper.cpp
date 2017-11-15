#include "storage/where_clause_helper.h"

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

  if (expression->Data().empty()) {
    if (children.size() != 1) {
      DEBUG_MSG("Expression should have only 1 child node");
      return false;
    }

    return isValidTerm(children[0]);
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
    if (term->Data().empty()) {
      DEBUG_MSG("Term must have data for comparison");
      return false;
    }

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
