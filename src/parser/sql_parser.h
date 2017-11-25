#ifndef SRC_SQL_PARSER_H
#define SRC_SQL_PARSER_H

#include <string>

#include "base/sql_errors.h"
#include "base/tokenizer.h"
#include "parser/sql_node.h"

class SqlParser {
 public:
  void SetQuery(std::string query);
  void Parse(SqlNode *node, SqlErrors::Type& error_code);

 private:
  bool readWord(std::string& word);
  bool consumeWord(std::string& word);
  bool consumeLiteral(std::string& literal);
  void consumeSpaceCharacters();

  bool isTableName(const std::string & table_name);
  bool isInteger(const std::string& integer);
  bool isCompOp(const std::string& comp_op);
  bool isOperatorSign(const std::string& sign);
  bool isEndOfStatement();

  SqlNode *createNodeAndAppendAsChild(SqlNode *node,
      const SqlNode::NodeType type, const std::string& name = "");

  bool handleStatement(SqlNode *node);
  bool handleCreateTableStatement(SqlNode *node);
  bool handleDropTableStatement(SqlNode *node);
  bool handleSelectStatement(SqlNode *node);
  bool handleDeleteStatement(SqlNode *node);
  bool handleInsertStatement(SqlNode *node);
  bool handleAttributeTypeList(SqlNode *node);
  bool handleDataType(SqlNode *node);
  bool handleSelectList(SqlNode *node);
  bool handleSelectSublist(SqlNode *node);
  bool handleTableList(SqlNode *node);
  bool handleInsertTuples(SqlNode *node);
  bool handleAttributeList(SqlNode *node);
  bool handleValue(SqlNode *node);
  bool handleValueList(SqlNode *node);
  bool handleSearchCondition(SqlNode *node);
  bool handleBooleanTerm(SqlNode *node);
  bool handleBooleanFactor(SqlNode *node);
  bool handleExpression(SqlNode *node);
  bool handleTerm(SqlNode *node);
  bool handleColumnName(SqlNode *node);
  bool handleTableName(SqlNode *node);
  bool handleAttributeName(SqlNode *node);
  bool handleCompOp(SqlNode *node);
  bool handleOperatorSign(SqlNode *node);

  std::string input_query_;
  int current_query_position_;

  Tokenizer tokenizer_;
};

#endif //SRC_SQL_PARSER_H
