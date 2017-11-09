#include "sql_node.h"

#include <algorithm>

#include "debug.h"

SqlNode::SqlNode(NodeType type, std::string data)
    : type_(type),
      data_(data) {
}

SqlNode::~SqlNode() {

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
  if (index <= 0 || children_.size() < index) {
    DEBUG_MSG("Invalid index");
    return nullptr;
  }

  return children_[index - 1];
}

void SqlNode::RemoveAllChildren() {
  children_.clear();
}

std::vector<SqlNode *> SqlNode::Children() const {
  return children_;
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

  return true;
}

bool SqlNode::ColumnName(std::string& column_name) const {
  if (type_ != NODE_TYPE_COLUMN_NAME) {
    DEBUG_MSG("");
    return false;
  }

  column_name = Data();
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
    return ValueList(tuples);
  }

  DEBUG_MSG("");
  return false;
}
