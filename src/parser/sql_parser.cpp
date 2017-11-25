#include "sql_parser.h"

#include <regex>

#include "base/debug.h"
#include "base/tokenizer.h"

void SqlParser::SetQuery(std::string query) {
  input_query_ = query;
}

void SqlParser::Parse(SqlNode *node, SqlErrors::Type& error_code) {
  if (input_query_.size() == 0 || !node ||
      node->Type() != SqlNode::NODE_TYPE_STATEMENT) {
    DEBUG_MSG("");
    error_code = SqlErrors::EMPTY_STATEMENT;
    return;
  }

  current_query_position_ = 0;
  if (!handleStatement(node)) {
    DEBUG_MSG("");
		error_code = SqlErrors::UNACCEPTABLE_STATEMENT;
	}
}

// Private methods
bool SqlParser::readWord(std::string& word) {
  if (current_query_position_ >= input_query_.size()) {
    LOG_MSG("");
    return false;
  }

  char separator = '\0';
  int leading_spaces = 0;
  if (!Tokenizer::ReadOneWord(
      std::string(input_query_.substr(current_query_position_)), word,
      separator, leading_spaces)) {
    DEBUG_MSG("");
    return false;
  }

  if (!word.compare(" ")) {
    return Tokenizer::ReadOneWord(
        std::string(input_query_.substr(current_query_position_ + 1)), word,
        separator, leading_spaces);
  }

  return true;
}

bool SqlParser::consumeWord(std::string& word) {
  if (current_query_position_ >= input_query_.size()) {
    DEBUG_MSG("");
    return false;
  }

  char separator = '\0';
  int leading_spaces = 0;
  if (!Tokenizer::ReadOneWord(
      std::string(input_query_.substr(current_query_position_)), word,
      separator, leading_spaces)) {
    DEBUG_MSG("");
    return false;
  }

  current_query_position_ += word.length();
  if (separator != '\0' && (word[0] == separator)) {
    current_query_position_ += leading_spaces;
  }

  if (!word.compare(" ")) {
    current_query_position_--;
    return consumeWord(word);
  }

  return true;
}

bool SqlParser::consumeLiteral(std::string& literal) {
  if (current_query_position_ >= input_query_.size()) {
    DEBUG_MSG("");
    return false;
  }

  consumeSpaceCharacters();
  if (Tokenizer::ReadLiteral(
      std::string(input_query_.substr(current_query_position_)), literal)) {
    current_query_position_ += literal.length();
    return true;
  }

  LOG_MSG("");
  return false;
}

void SqlParser::consumeSpaceCharacters() {
  while (current_query_position_ < input_query_.size() &&
      input_query_[current_query_position_] == ' ') {
    current_query_position_++;
  }
}

bool SqlParser::isTableName(const std::string& table_name) {
  return std::regex_match(table_name, std::regex("^[a-z]([a-z0-9])*$"));
}

bool SqlParser::isInteger(const std::string& integer) {
  return std::regex_match(integer, std::regex("^[0-9]+$"));
}

bool SqlParser::isCompOp(const std::string& comp_op) {
  return std::regex_match(comp_op, std::regex("^[<>=]$"));
}

bool SqlParser::isOperatorSign(const std::string& sign) {
  return std::regex_match(sign, std::regex("^[+-*]$"));
}

bool SqlParser::isEndOfStatement() {
  if (current_query_position_ >= input_query_.length()) {
    return true;
  } else {
    LOG_MSG("");
    consumeSpaceCharacters();
    return (current_query_position_ == input_query_.length());
  }
}

SqlNode *SqlParser::createNodeAndAppendAsChild(SqlNode *node,
    const SqlNode::NodeType type, const std::string& name) {
  SqlNode *create_node = new SqlNode(type, name);
  node->AppendChild(create_node);
  return create_node;
}

bool SqlParser::handleStatement(SqlNode *node) {
  std::string first_word;
  if (!readWord(first_word)) {
    DEBUG_MSG("");
    return false;
  }

  if (!first_word.compare("CREATE")) {
    return handleCreateTableStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_CREATE_TABLE_STATEMENT));
  }

  if (!first_word.compare("DROP")) {
    return handleDropTableStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_DROP_TABLE_STATEMENT));
  }

  if (!first_word.compare("SELECT")) {
    return handleSelectStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_SELECT_STATEMENT));
  }

  if (!first_word.compare("DELETE")) {
    return handleDeleteStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_DELETE_STATEMENT));
  }

  if (!first_word.compare("INSERT")) {
    return handleInsertStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_INSERT_STATEMENT));
  }

  DEBUG_MSG("");
	return false;
}

bool SqlParser::handleCreateTableStatement(SqlNode *node) {
  std::string word;
  if (!consumeWord(word) || word.compare("CREATE")) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("TABLE")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("(")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleAttributeTypeList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_ATTRIBUTE_TYPE_LIST))) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare(")")) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleDropTableStatement(SqlNode *node) {
  std::string word;
  if (!consumeWord(word) || word.compare("DROP")) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("TABLE")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleSelectStatement(SqlNode *node) {
  std::string word;
  if (!consumeWord(word) || word.compare("SELECT")) {
    DEBUG_MSG("");
    return false;
  }

  if (readWord(word) && !word.compare("DISTINCT")) {
    consumeWord(word);
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_DISTINCT);
  }

  if (!handleSelectList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_SELECT_LIST))) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("FROM")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleTableList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_LIST))) {
    DEBUG_MSG("");
    return false;
  }

  if (readWord(word) && !word.compare("WHERE")) {
    consumeWord(word);
    if (!handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_SEARCH_CONDITION))) {
      DEBUG_MSG("");
      return false;
    }
  }

  if (readWord(word) && !word.compare("ORDER")) {
    consumeWord(word);
    if (consumeWord(word) && !word.compare("BY")) {
      if (!handleColumnName(createNodeAndAppendAsChild(
          node, SqlNode::NODE_TYPE_COLUMN_NAME))) {
        DEBUG_MSG("");
        return false;
      }
    } else {
      DEBUG_MSG("");
      return false;
    }
  }

  return isEndOfStatement();
}

bool SqlParser::handleDeleteStatement(SqlNode *node) {
  std::string word;
  if (!consumeWord(word) || word.compare("DELETE")) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("FROM")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  if (readWord(word) && !word.compare("WHERE")) {
    consumeWord(word);
    if (!handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_SEARCH_CONDITION))) {
      return false;
    }
  }

  return isEndOfStatement();
}

bool SqlParser::handleInsertStatement(SqlNode *node) {
  std::string word;
  if (!consumeWord(word) || word.compare("INSERT")) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("INTO")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare("(")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleAttributeList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_ATTRIBUTE_LIST))) {
    DEBUG_MSG("");
    return false;
  }

  if (!consumeWord(word) || word.compare(")")) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleInsertTuples(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_INSERT_TUPLES))) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleAttributeTypeList(SqlNode *node) {
  if (!handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_ATTRIBUTE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleDataType(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_DATA_TYPE))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare(",")) {
    consumeWord(word);
    return handleAttributeTypeList(node);
  }

  return true;
}

bool SqlParser::handleDataType(SqlNode *node) {
  std::string data_type;
  if (!consumeWord(data_type)) {
    DEBUG_MSG("");
    return false;
  }

  if (data_type.compare("INT") && data_type.compare("STR20")) {
    DEBUG_MSG("");
    return false;
  }

  node->SetData(data_type);
  return true;
}

bool SqlParser::handleSelectList(SqlNode *node) {
  std::string word;
  if (readWord(word) && !word.compare("*")) {
    consumeWord(word);
    createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_COLUMN_NAME, word);
    return true;
  } else {
    return handleSelectSublist(node);
  }
}

bool SqlParser::handleSelectSublist(SqlNode *node) {
  if (!handleColumnName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_COLUMN_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare(",")) {
    consumeWord(word);
    return handleSelectSublist(node);
  }

  return true;
}

bool SqlParser::handleTableList(SqlNode *node) {
  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TABLE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare(",")) {
    consumeWord(word);
    return handleTableList(node);
  }

  return true;
}

bool SqlParser::handleInsertTuples(SqlNode *node) {
  std::string word;
  if (readWord(word) && !word.compare("VALUES")) {
    consumeWord(word);

    if (!consumeWord(word) || word.compare("(")) {
      DEBUG_MSG("");
      return false;
    }

    if (!handleValueList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_VALUE_LIST))) {
      DEBUG_MSG("");
      return false;
    }

    if (!consumeWord(word) || word.compare(")")) {
      DEBUG_MSG("");
      return false;
    }

    return true;
  }

  return handleSelectStatement(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_SELECT_STATEMENT));
}

bool SqlParser::handleAttributeList(SqlNode *node) {
  if (!handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_ATTRIBUTE_NAME))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare(",")) {
    consumeWord(word);
    return handleAttributeList(node);
  }

  return true;
}

bool SqlParser::handleValue(SqlNode *node) {
  std::string word;
  if (readWord(word) && (!word.compare("NULL") || isInteger(word))) {
    consumeWord(word);
    node->SetData(word);
    return true;
  }

  if (consumeLiteral(word)) {
    node->SetData(word);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleValueList(SqlNode *node) {
  if (!handleValue(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_VALUE))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare(",")) {
    consumeWord(word);
    return handleValueList(node);
  }

  return true;
}

bool SqlParser::handleSearchCondition(SqlNode *node) {
  if (!handleBooleanTerm(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_BOOLEAN_TERM))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare("OR")) {
    consumeWord(word);
    node->SetData(word);
    return handleSearchCondition(node);
  }

  return true;
}

bool SqlParser::handleBooleanTerm(SqlNode *node) {
  if (!handleBooleanFactor(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_BOOLEAN_FACTOR))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  if (readWord(word) && !word.compare("AND")) {
    consumeWord(word);
    node->SetData(word);
    return handleBooleanTerm(node);
  }

  return true;
}

bool SqlParser::handleBooleanFactor(SqlNode *node) {
  if (!handleExpression(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_EXPRESSION))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleCompOp(node)) {
    DEBUG_MSG("");
    return false;
  }

  return handleExpression(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_EXPRESSION));
}

bool SqlParser::handleExpression(SqlNode *node) {
  std::string word;
  if (readWord(word) && !word.compare("(")) {
    consumeWord(word);
    if (!handleTerm(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_TERM))) {
      DEBUG_MSG("");
      return false;
    }

    if (handleOperatorSign(node)) {
      DEBUG_MSG("");
      return false;
    }

    if (!handleTerm(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_TERM))) {
      DEBUG_MSG("");
      return false;
    }

    if (!consumeWord(word) || word.compare(")")) {
      DEBUG_MSG("");
      return false;
    }

    return true;
  }

  return handleTerm(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_TERM));
}

bool SqlParser::handleTerm(SqlNode *node) {
  std::string word;
  if (consumeLiteral(word)) {
    node->SetData(word);
    return true;
  }

  if (readWord(word) && (isInteger(word) || word == "NULL")) {
    consumeWord(word);
    node->SetData(word);
    return true;
  }

  return handleColumnName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_COLUMN_NAME));
}

bool SqlParser::handleColumnName(SqlNode *node) {
  std::string word;
  consumeSpaceCharacters();
  if (readWord(word) &&
      input_query_[current_query_position_ + word.length()] == '.') {
    if (!handleTableName(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_TABLE_NAME))) {
      DEBUG_MSG("");
      return false;
    }

    // To skip the '.'
    if (!consumeWord(word) || word.compare(".")) {
      DEBUG_MSG("");
      return false;
    }
  }

  return handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_ATTRIBUTE_NAME));
}

bool SqlParser::handleTableName(SqlNode *node) {
  std::string table_name;
  if (!consumeWord(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  if (isTableName(table_name)) {
    node->SetData(table_name);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleAttributeName(SqlNode *node) {
  if (!handleTableName(node)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}

bool SqlParser::handleCompOp(SqlNode *node) {
  std::string comp_op;
  if (consumeWord(comp_op) && isCompOp(comp_op)) {
    node->SetData(comp_op);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleOperatorSign(SqlNode *node) {
  std::string sign;
  if (consumeWord(sign) && isOperatorSign(sign)) {
    node->SetData(sign);
    return true;
  }

  DEBUG_MSG("");
  return false;
}
