#include "parser/sql_node.h"

#include <algorithm>

#include "base/debug.h"
#include "base/duplicate_finder.h"

SqlNode::SqlNode(NodeType type, std::string data)
    : type_(type),
      data_(data) {
}

SqlNode::~SqlNode() {
  for (auto child : children_) {
    if (child != nullptr) {
      delete child;
      child = nullptr;
    }
  }
}

void SqlNode::AppendChild(SqlNode* child) {
  children_.push_back(child);
}

void SqlNode::RemoveChild(SqlNode* child) {
  if (!child) {
    return;
  }

  // TODO(Kishan): We need to use an efficient container which avoid duplicates.
  auto item = std::find(children_.begin(), children_.end(), child);
  if (item != children_.end())
    children_.erase(item);
}

SqlNode *SqlNode::Child(int index) const {
  if (index < 0 || index >= children_.size()) {
    DEBUG_MSG("Invalid index");
    return nullptr;
  }

  return children_[index];
}

void SqlNode::RemoveAllChildren() {
  children_.clear();
}

std::vector<SqlNode *> SqlNode::Children() const {
  return children_;
}

int SqlNode::ChildrenCount() const {
  return children_.size();
}

SqlNode::NodeType SqlNode::Type() const {
  return type_;
}

void SqlNode::SetType(NodeType type) {
  type_ = type;
}

std::string SqlNode::Data() const {
  return data_;
}

void SqlNode::SetData(const std::string data) {
  data_ = data;
}

bool SqlNode::TableName(std::string& table_name) const {
  if (type_ != NODE_TYPE_TABLE_NAME) {
    DEBUG_MSG("");
    return false;
  }

  if (!isValidTableName()) {
    DEBUG_MSG("");
    return false;
  }

  table_name = Data();
  return true;
}

bool SqlNode::AttributeName(std::string& name) const {
  if (type_ != NODE_TYPE_ATTRIBUTE_NAME) {
    DEBUG_MSG("");
    return false;
  }

  name = Data();
  return true;
}

bool SqlNode::Datatype(FIELD_TYPE& type) const {
  if (type_ != NODE_TYPE_DATA_TYPE) {
    DEBUG_MSG("");
    return false;
  }

  std::string data = Data();
  if (!data.compare("INT")) {
    type = INT;
    return true;
  } else if (!data.compare("STR20")) {
    type = STR20;
    return true;
  } else {
    DEBUG_MSG("");
    return false;
  }
}

bool SqlNode::AttributeTypeList(std::vector<std::string>& attribute_names,
    std::vector<FIELD_TYPE>& attribute_types) const {
  if (type_ != NODE_TYPE_ATTRIBUTE_TYPE_LIST || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  attribute_names.clear();
  attribute_types.clear();
  for (auto attribute : children_) {
    if (attribute->Type() == NODE_TYPE_ATTRIBUTE_NAME) {
      std::string name;
      if (!attribute->AttributeName(name)) {
        DEBUG_MSG("");
        return false;
      }

      attribute_names.push_back(name);
    } else if (attribute->Type() == NODE_TYPE_DATA_TYPE) {
      FIELD_TYPE type;
      if (!attribute->Datatype(type)) {
        DEBUG_MSG("");
        return false;
      }

      attribute_types.push_back(type);
    } else {
      DEBUG_MSG("Invalid data type");
      return false;
    }
  }

  DuplicateFinder<std::string> duplicate_finder(attribute_names);
  if (duplicate_finder.HasDuplicates()) {
    DEBUG_MSG("Contains duplicate attribute names");
    return false;
  }

  if (attribute_names.size() == attribute_types.size()) {
    return true;
  } else {
    DEBUG_MSG("Mismatch in number of attributes");
    return false;
  }
}

bool SqlNode::SelectList(std::vector<std::string>& select_list) const {
  if (type_ != NODE_TYPE_SELECT_LIST || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto column : children_) {
    std::string column_name;
    if (!column->ColumnName(column_name)) {
      DEBUG_MSG("");
      return false;
    }
    select_list.push_back(column_name);
  }

  DuplicateFinder<std::string> duplicate_finder(select_list);
  if (duplicate_finder.HasDuplicates()) {
    DEBUG_MSG("Contains duplicate attribute names in SELECT list");
    return false;
  }

  return true;
}

bool SqlNode::ColumnName(std::string& column_name) const {
  if (type_ != NODE_TYPE_COLUMN_NAME) {
    DEBUG_MSG("");
    return false;
  }

  if (!isValidColumnName()) {
    DEBUG_MSG("");
    return false;
  }

  if (children_.size() == 0) {
    column_name = Data();
    return true;
  } else if (children_.size() == 1) {
    return children_[0]->AttributeName(column_name);
  }

  std::string table_name, attribute_name;
  if (!children_[0]->TableName(table_name) ||
      !children_[1]->AttributeName(attribute_name)) {
    return false;
  }

  column_name = table_name + "." + attribute_name;
  return true;
}

bool SqlNode::TableList(std::vector<std::string>& table_list) const {
  if (type_ != NODE_TYPE_TABLE_LIST || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto table : children_) {
    std::string table_name;
    if (!table->TableName(table_name)) {
      DEBUG_MSG("");
      return false;
    }
    table_list.push_back(table_name);
  }

  DuplicateFinder<std::string> duplicate_finder(table_list);
  if (duplicate_finder.HasDuplicates()) {
    DEBUG_MSG("Contains duplicate table names");
    return false;
  }

  return true;
}

bool SqlNode::AttributeList(std::vector<std::string>& attribute_list) const {
  if (type_ != NODE_TYPE_ATTRIBUTE_LIST || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto attribute : children_) {
    std::string attribute_name;
    if (!attribute->AttributeName(attribute_name)) {
      DEBUG_MSG("");
      return false;
    }

    attribute_list.push_back(attribute_name);
  }

  DuplicateFinder<std::string> duplicate_finder(attribute_list);
  if (duplicate_finder.HasDuplicates()) {
    DEBUG_MSG("Contains duplicate attribute names");
    return false;
  }

  return true;
}

bool SqlNode::Value(std::string& value) const {
  if (type_ != NODE_TYPE_VALUE) {
    DEBUG_MSG("");
    return false;
  }

  value = Data();
  return true;
}

bool SqlNode::ValueList(std::vector<std::string>& value_list) const {
  if (type_ != NODE_TYPE_VALUE_LIST || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto value : children_) {
    std::string value_string;
    if (!value->Value(value_string)) {
      DEBUG_MSG("");
      return false;
    }
    value_list.push_back(value_string);
  }

  return true;
}

bool SqlNode::InsertTuples(std::vector<std::string>& tuples) const {
  if (type_ != NODE_TYPE_INSERT_TUPLES || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  if (children_[0]->Type() == NODE_TYPE_VALUE_LIST) {
    return children_[0]->ValueList(tuples);
  }

  DEBUG_MSG("");
  return false;
}

bool SqlNode::ValidateSearchCondition() const {
  if (type_ != NODE_TYPE_SEARCH_CONDITION || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto boolean_term : children_) {
    if (!boolean_term->isValidBooleanTerm()) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool SqlNode::isValidBooleanTerm() const {
  if (type_ != NODE_TYPE_BOOLEAN_TERM || children_.size() <= 0) {
    DEBUG_MSG("");
    return false;
  }

  for (auto boolean_factor : children_) {
    if (!boolean_factor->isValidBooleanFactor()) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool SqlNode::isValidBooleanFactor() const {
  if (type_ != NODE_TYPE_BOOLEAN_FACTOR || children_.size() != 2) {
    DEBUG_MSG("");
    return false;
  }

  if (data_ != "<" && data_ != ">" && data_ != "=") {
    DEBUG_MSG("Invalid comparison operator");
    return false;
  }

  for (auto expression : children_) {
    if (!expression->isValidExpression()) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool SqlNode::isValidExpression() const {
  if (type_ != NODE_TYPE_EXPRESSION || children_.size() > 2) {
    DEBUG_MSG("");
    return false;
  }

  if (data_.empty()) {
    if (children_.size() != 1) {
      DEBUG_MSG("Expression should have exactly one child node");
      return false;
    }

    return children_[0]->isValidTerm();
  }

  if (data_ != "+" && data_ != "-" && data_ != "*") {
    DEBUG_MSG("Invalid operator in expression");
    return false;
  }

  for (auto term : children_) {
    if (!term->isValidTerm()) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool SqlNode::isValidTerm() const {
  if (type_ != NODE_TYPE_TERM || children_.size() > 1) {
    DEBUG_MSG("");
    return false;
  }

  if (data_.empty()) {
    return children_[0]->isValidColumnName();
  }

  return true;
}

bool SqlNode::isValidColumnName() const {
  if (type_ != NODE_TYPE_COLUMN_NAME) {
    DEBUG_MSG("Invalid node type for column");
    return false;
  }

  if (children_.size() == 0) {
    return Data() == "*";
  } else if (children_.size() == 1) {
    return children_[0]->isValidAttributeName();
  } else if (children_.size() == 2) {
    return children_[0]->isValidTableName() &&
        children_[1]->isValidAttributeName();
  } else {
    DEBUG_MSG("Invalid number of children for column name node");
    return false;
  }
}

bool SqlNode::isValidTableName() const {
  return (type_ == NODE_TYPE_TABLE_NAME) &&
      (children_.size() == 0) && !data_.empty();
}

bool SqlNode::isValidAttributeName() const {
  return (type_ == NODE_TYPE_ATTRIBUTE_NAME) &&
      (children_.size() == 0) && !data_.empty();
}
