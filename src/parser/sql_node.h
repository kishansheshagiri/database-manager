#ifndef SRC_SQL_NODE_H
#define SRC_SQL_NODE_H

#include <string>
#include <vector>

#include "storage/storage_manager_headers.h"

class SqlNode {
 public:
  enum NodeType {
    NODE_TYPE_STATEMENT = 0,
    NODE_TYPE_CREATE_TABLE_STATEMENT,
    NODE_TYPE_DROP_TABLE_STATEMENT,
    NODE_TYPE_SELECT_STATEMENT,
    NODE_TYPE_DELETE_STATEMENT,
    NODE_TYPE_INSERT_STATEMENT,

    NODE_TYPE_TABLE_LIST,
    NODE_TYPE_TABLE_NAME,

    NODE_TYPE_ATTRIBUTE_TYPE_LIST,
    NODE_TYPE_ATTRIBUTE_NAME,
    NODE_TYPE_DATA_TYPE,

    NODE_TYPE_DISTINCT,
    NODE_TYPE_SELECT_LIST,
    NODE_TYPE_COLUMN_NAME,

    NODE_TYPE_ATTRIBUTE_LIST,
    NODE_TYPE_INSERT_TUPLES,
    NODE_TYPE_VALUE_LIST,
    NODE_TYPE_VALUE,

    NODE_TYPE_SEARCH_CONDITION,
    NODE_TYPE_BOOLEAN_TERM,
    NODE_TYPE_BOOLEAN_FACTOR,
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_TERM
  };

  SqlNode(NodeType type, std::string data = "");
  virtual ~SqlNode();

  void AppendChild(SqlNode* child);
  void RemoveChild(SqlNode* child);
  SqlNode *Child(int index) const;
  void RemoveAllChildren();
  std::vector<SqlNode *> Children() const;

  NodeType Type() const;
  void SetType(NodeType type);
  std::string Data() const;
  void SetData(const std::string data);

  bool TableName(std::string& table_name) const;
  bool AttributeName(std::string& name) const;
  bool Datatype(FIELD_TYPE& type) const;
  bool AttributeTypeList(std::vector<std::string>& attribute_names,
      std::vector<FIELD_TYPE>& attribute_types) const;
  bool SelectList(std::vector<std::string>& select_list) const;
  bool ColumnName(std::string& column_name) const;
  bool TableList(std::vector<std::string>& table_list) const;
  bool AttributeList(std::vector<std::string>& attribute_list) const;
  bool Value(std::string& value) const;
  bool ValueList(std::vector<std::string>& value_list) const;
  bool InsertTuples(std::vector<std::string>& tuples) const;

 private:
  NodeType type_;
  std::string data_;
  std::vector<SqlNode *> children_;
};

#endif // SRC_SQL_NODE_H
