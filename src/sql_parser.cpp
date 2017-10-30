#include "sql_parser.h"

#include <regex>

#include "debug.h"

void SqlParser::SetQuery(std::string query) {
  input_query_ = query;
}

void SqlParser::Parse(SqlNode *node, SqlErrors::Type& error_code) {
  if (input_query_.size() == 0 || !node) {
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
    DEBUG_MSG("");
    return false;
  }

  std::string trailing_characters;
  if (!tokenizer_.ReadOneWord(
      std::string(input_query_.substr(current_query_position_)), word,
          trailing_characters)) {
    if (trailing_characters.size() > 0) {
      word = trailing_characters;
      return true;
    } else {
      return false;
    }
  }

  return true;
}

bool SqlParser::readWordAndUpdate(std::string& word) {
  DEBUG_MSG("");
  if (!readWord(word)) {
    DEBUG_MSG("");
    return false;
  }

  current_query_position_ += word.length() + 1;
  return true;
}

bool SqlParser::readWordAndUpdate(std::string& word, bool& is_list) {
  if (!readWordAndUpdate(word)) {
    return false;
  }

  if (word.back() == ',') {
    is_list = true;
    word.erase(word.back());
  }

  return true;
}

bool SqlParser::readLiteralAndUpdate(std::string& literal) {
  DEBUG_MSG("");
  return false;
}

void SqlParser::skipComma() {

}

bool SqlParser::isInteger(std::string& integer) {
  return std::regex_match(integer, std::regex("^[0-9]+$"));
}

bool SqlParser::isCompOp(std::string& comp_op) {
  return std::regex_match(comp_op, std::regex("^[<>=]$"));
}

bool SqlParser::isOperatorSign(std::string& sign) {
  return std::regex_match(sign, std::regex("^[+-*]$"));
}

bool SqlParser::isEndOfStatement() {
  if (current_query_position_ >= input_query_.length()) {
    DEBUG_MSG("");
    return true;
  } else {
    DEBUG_MSG("");
    return false;
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
    DEBUG_MSG("");
    return handleCreateTableStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "create-table-statement"));
  }

  if (!first_word.compare("DROP")) {
    DEBUG_MSG("");
    return handleDropTableStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "drop-table-statement"));
  }

  if (!first_word.compare("SELECT")) {
    DEBUG_MSG("");
    return handleSelectStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "select-statement"));
  }

  if (!first_word.compare("DELETE")) {
    DEBUG_MSG("");
    return handleDeleteStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "delete-statement"));
  }

  if (!first_word.compare("INSERT")) {
    DEBUG_MSG("");
    return handleInsertStatement(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "insert-statement"));
  }

  DEBUG_MSG("");
	return false;
}

bool SqlParser::handleCreateTableStatement(SqlNode *node) {
  std::string word;
  if (!readWordAndUpdate(word) || word.compare("CREATE")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!readWordAndUpdate(word) || word.compare("TABLE")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  std::size_t open_parenthesis = input_query_.find_first_of("(");
  std::size_t close_parenthesis = input_query_.find_first_of(")");
  if (open_parenthesis != std::string::npos &&
      close_parenthesis != std::string::npos) {
    input_query_[open_parenthesis] = ' ';
    input_query_[close_parenthesis] = ' ';
  } else {
    DEBUG_MSG("Missing paranthesis");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleAttributeTypeList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "attribute-type-list"))) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleDropTableStatement(SqlNode *node) {
  std::string word;
  if (!readWordAndUpdate(word) || word.compare("DROP")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!readWordAndUpdate(word) || word.compare("TABLE")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleSelectStatement(SqlNode *node) {
  std::string word;
  if (!readWordAndUpdate(word) || word.compare("SELECT")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("DISTINCT")) {
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
  } else {
    current_query_position_ = actual_position;
  }

  if (!handleSelectList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "select-list"))) {
    return false;
  }

  if (!readWordAndUpdate(word) || word.compare("FROM")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    DEBUG_MSG("");
    return false;
  }

  actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("WHERE")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
    if (!handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "search-condition"))) {
      return false;
    }
  } else {
    current_query_position_ = actual_position;
  }

  actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("ORDER")) {
    DEBUG_MSG("");
    if (readWordAndUpdate(word) && !word.compare("BY")) {
      DEBUG_MSG("");
      createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
      if (!handleColumnName(createNodeAndAppendAsChild(
          node, SqlNode::NODE_TYPE_OPERAND, "column-name"))) {
        return false;
      }
    } else {
      DEBUG_MSG("");
      return false;
    }
  } else {
    current_query_position_ = actual_position;
  }

  return isEndOfStatement();
}

bool SqlParser::handleDeleteStatement(SqlNode *node) {
  std::string word;
  if (!readWordAndUpdate(word) || word.compare("DELETE")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!readWordAndUpdate(word) || word.compare("FROM")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    return false;
  }

  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("WHERE")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
    if (!handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "search-condition"))) {
      return false;
    }
  } else {
    current_query_position_ = actual_position;
  }

  return isEndOfStatement();
}

bool SqlParser::handleInsertStatement(SqlNode *node) {
  std::string word;
  if (!readWordAndUpdate(word) || word.compare("INSERT")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  if (!readWordAndUpdate(word) || word.compare("INTO")) {
    DEBUG_MSG("");
    return false;
  }
  createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);

  std::size_t open_parenthesis = input_query_.find_first_of("(");
  std::size_t close_parenthesis = input_query_.find_first_of(")");
  if (open_parenthesis != std::string::npos &&
      close_parenthesis != std::string::npos) {
    input_query_[open_parenthesis] = ' ';
    input_query_[close_parenthesis] = ' ';
  } else {
    DEBUG_MSG("Missing paranthesis");
    return false;
  }

  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleAttributeList(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "attribute-list"))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleInsertTuples(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "insert-tuples"))) {
    DEBUG_MSG("");
    return false;
  }

  return isEndOfStatement();
}

bool SqlParser::handleAttributeTypeList(SqlNode *node) {
  if (!handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "attribute-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleDataType(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "data-type"))) {
    DEBUG_MSG("");
    return false;
  }

  if (input_query_[current_query_position_] == ',') {
    skipComma();

    if (!handleAttributeTypeList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "attribute-type-list"))) {
      DEBUG_MSG("");
      return false;
    }
  }

  return true;
}

bool SqlParser::handleDataType(SqlNode *node) {
  std::string data_type;
  if (readWordAndUpdate(data_type) && !data_type.compare("INT")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, data_type);
    return true;
  }

  if (readWordAndUpdate(data_type) && !data_type.compare("STR20")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, data_type);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleSelectList(SqlNode *node) {
  if (input_query_[current_query_position_] == '*') {
    std::string select_list;
    if (readWordAndUpdate(select_list) && !select_list.compare("*")) {
      DEBUG_MSG("");
      createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, select_list);
      return true;
    }
  } else {
    return handleSelectSublist(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "select-sublist"));
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleSelectSublist(SqlNode *node) {
  if (!handleColumnName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "column-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (input_query_[current_query_position_] == ',') {
    skipComma();

    if (!handleSelectSublist(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "select-sublist"))) {
      DEBUG_MSG("");
      return false;
    }
  }

  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleTableList(SqlNode *node) {
  if (!handleTableName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (input_query_[current_query_position_] == ',') {
    skipComma();

    return handleTableList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "table-list"));
  }

  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleInsertTuples(SqlNode *node) {
  std::string word;
  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("VALUES")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, word);

    std::size_t open_parenthesis = input_query_.find_first_of("(");
    std::size_t close_parenthesis = input_query_.find_first_of(")");
    if (open_parenthesis != std::string::npos &&
        close_parenthesis != std::string::npos) {
      input_query_[open_parenthesis] = ' ';
      input_query_[close_parenthesis] = ' ';
    } else {
      DEBUG_MSG("Missing paranthesis");
      return false;
    }

    return handleValueList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "value-list"));
  }

  current_query_position_ = actual_position;
  return handleSelectStatement(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "select-statement"));
}

bool SqlParser::handleAttributeList(SqlNode *node) {
  if (!handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "attribute-name"))) {
    DEBUG_MSG("");
    return false;
  }

  if (input_query_[current_query_position_] == ',') {
    skipComma();

    return handleAttributeList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "attribute-list"));
  }

  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleValue(SqlNode *node) {
  int actual_position = current_query_position_;

  std::string value;
  if (readWordAndUpdate(value)) {
    if (!value.compare("NULL") || isInteger(value)) {
      DEBUG_MSG("");
      createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, value);
      return true;
    }
  }

  current_query_position_ = actual_position;
  if (readLiteralAndUpdate(value)) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, value);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleValueList(SqlNode *node) {
  if (!handleValue(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "value"))) {
    DEBUG_MSG("");
    return false;
  }

  if (input_query_[current_query_position_] == ',') {
    skipComma();

    return handleValueList(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "value-list"));
  }

  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleSearchCondition(SqlNode *node) {
  if (!handleBooleanTerm(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "boolean-term"))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("OR")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
    return handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "search-condition"));
  }

  current_query_position_ = actual_position;
  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleBooleanTerm(SqlNode *node) {
  if (!handleBooleanFactor(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "boolean-factor"))) {
    DEBUG_MSG("");
    return false;
  }

  std::string word;
  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && !word.compare("AND")) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, word);
    return handleSearchCondition(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "boolean-term"));
  }

  current_query_position_ = actual_position;
  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleBooleanFactor(SqlNode *node) {
  if (!handleExpression(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "expression"))) {
    DEBUG_MSG("");
    return false;
  }

  if (!handleCompOp(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "comp-op"))) {
    DEBUG_MSG("");
    return false;
  }

  return handleExpression(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "expression"));
}

bool SqlParser::handleExpression(SqlNode *node) {
  if (input_query_[current_query_position_] == '(') {
    std::size_t close_parenthesis = input_query_.find_first_of(")");
    if (close_parenthesis != std::string::npos) {
      input_query_[current_query_position_] = ' ';
      input_query_[close_parenthesis] = ' ';
    } else {
      DEBUG_MSG("Missing paranthesis");
      return false;
    }

    if (!handleTerm(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "term"))) {
      DEBUG_MSG("");
      return false;
    }

    std::string operator_sign;
    if (!readWordAndUpdate(operator_sign) || !isOperatorSign(operator_sign)) {
      DEBUG_MSG("");
      return false;
    }
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_OPERAND, operator_sign);
  }

  if (!handleTerm(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "term"))) {
    DEBUG_MSG("");
    return false;
  }

  DEBUG_MSG("");
  return true;
}

bool SqlParser::handleTerm(SqlNode *node) {
  std::string word;
  if (readLiteralAndUpdate(word)) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, word);
    return true;
  }

  int actual_position = current_query_position_;
  if (readWordAndUpdate(word) && isInteger(word)) {
    DEBUG_MSG("");
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, word);
    return true;
  }

  current_query_position_ = actual_position;
  return handleColumnName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "column-name"));
}

bool SqlParser::handleColumnName(SqlNode *node) {
  std::string word;
  if (readWord(word) &&
      input_query_[current_query_position_ + word.length()] == '.') {
    if (!handleTableName(createNodeAndAppendAsChild(
        node, SqlNode::NODE_TYPE_OPERAND, "table-name"))) {
      DEBUG_MSG("");
      return false;
    }
  }

  return handleAttributeName(createNodeAndAppendAsChild(
      node, SqlNode::NODE_TYPE_OPERAND, "attribute-name"));
}

bool SqlParser::handleTableName(SqlNode *node) {
  std::string table_name;
  if (!readWordAndUpdate(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  if (std::regex_match(table_name, std::regex("^[a-z]([a-z0-9])*$"))) {
    createNodeAndAppendAsChild(node, SqlNode::NODE_TYPE_VALUE, table_name);
    return true;
  }

  DEBUG_MSG("");
  return false;
}

bool SqlParser::handleAttributeName(SqlNode *node) {
  return handleTableName(node);
}

bool SqlParser::handleCompOp(SqlNode *node) {
  std::string comp_op;
  if (readWordAndUpdate(comp_op) && isCompOp(comp_op)) {
    return true;
  }

  DEBUG_MSG("");
  return false;
}
