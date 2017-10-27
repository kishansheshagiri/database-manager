#include "sql_parser.h"

#include <regex>

void SqlParser::SetQuery(std::string query) {
  input_query_ = query;
}

void SqlParser::Parse(SqlErrors::Type& error_code) {
  tokenizer_ = std::make_unique<Tokenizer>(input_query_);
  tokenizer_->Tokenize(error_code);
}

void SqlParser::ValidateTableName(const std::string& name,
    SqlErrors::Type& error_code) {
  ValidateAttributeName(name, error_code);
}

void SqlParser::ValidateAttributeName(const std::string& name,
    SqlErrors::Type& error_code) {
  if (!std::regex_match(name, std::regex("^[a-z]([a-z0-9])*$"))) {
    error_code = SqlErrors::INVALID_CHARACTER;
  }
}

void SqlParser::ValidateColumnName(const std::string& name,
    SqlErrors::Type& error_code) {
  std::vector<std::string> split_column_name;
  Tokenizer::SplitIntoWords(name, ".", split_column_name);
  if (split_column_name.size() > 2) {
    error_code = SqlErrors::INVALID_COLUMN_NAME;
    return;
  }

  if (split_column_name.size() == 2) {
    ValidateTableName(split_column_name[0], error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;

    ValidateAttributeName(split_column_name[1], error_code);
  }

  if (split_column_name.size() == 1) {
    ValidateAttributeName(split_column_name[0], error_code);
  } else {
    error_code = SqlErrors::UNKNOWN_ERROR;
  }
}

void SqlParser::ValidateLiteral(const std::string& literal,
    SqlErrors::Type& error_code) {
  if (!std::regex_match(literal, std::regex("^\\\"[^\\\"]*\\\"$"))) {
    error_code = SqlErrors::INVALID_CHARACTER;
  }
}

void SqlParser::CreateAndValidateAttributeList(const std::string& name,
    SqlErrors::Type& error_code, std::vector<std::string>& attributes) {
  Tokenizer::SplitIntoWords(name, ",", attributes);
  if (attributes.size() <= 0) {
    error_code = SqlErrors::INVALID_ATTRIBUTES;
    return;
  }

  for (auto attribute : attributes) {
    ValidateAttributeName(attribute, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}

void SqlParser::ValidateValue(const std::string& value,
    SqlErrors::Type& error_code) {
  if (value == "NULL")
    return;

  if (std::regex_match(value, std::regex("^[0-9]$"))) {
    return;
  }

  ValidateLiteral(value, error_code);
}

void SqlParser::CreateAndValidateValueList(const std::string& name,
    SqlErrors::Type& error_code, std::vector<std::string>& values) {
  Tokenizer::SplitIntoWords(name, ",", values);
  if (values.size() <= 0) {
    error_code = SqlErrors::INVALID_VALUES;
    return;
  }

  for (auto value : values) {
    ValidateValue(value, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}

void SqlParser::ValidateTerm(const std::string& term,
    SqlErrors::Type& error_code) {
  ValidateColumnName(term, error_code);
  if (error_code == SqlErrors::NO_ERROR)
    return;

  ValidateLiteral(term, error_code);
  if (error_code == SqlErrors::NO_ERROR)
    return;

  if (!std::regex_match(term, std::regex("^[0-9]$"))) {
    error_code = SqlErrors::INVALID_CHARACTER;
  }
}

void SqlParser::ValidateExpression(const std::string& expression,
    SqlErrors::Type& error_code) {
  std::vector<std::string> terms;
  Tokenizer::SplitIntoWordsMultiDelimiters(expression, "+-*", terms);
  if (terms.size() !=1 || terms.size() != 2) {
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }

  for (auto term : terms) {
    ValidateTerm(term, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}

void SqlParser::ValidateBooleanFactor(const std::string& boolean_factor,
    SqlErrors::Type& error_code) {
  std::vector<std::string> expressions;
  Tokenizer::SplitIntoWordsMultiDelimiters(boolean_factor, "<>=", expressions);
  if (expressions.size() != 2) {
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }

  for (auto expression : expressions) {
    ValidateExpression(expression, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}

void SqlParser::ValidateBooleanTerm(const std::string& boolean_term,
    SqlErrors::Type& error_code) {
  std::vector<std::string> factors;
  Tokenizer::SplitIntoWordsMultiDelimiters(boolean_term, " AND ", factors);
  if (factors.size() <= 0) {
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }

  for (auto factor : factors) {
    ValidateBooleanFactor(factor, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}

void SqlParser::ValidateSearchCondition(const std::string& condition,
    SqlErrors::Type& error_code) {
  std::vector<std::string> boolean_terms;
  Tokenizer::SplitIntoWordsMultiDelimiters(condition, " OR ", boolean_terms);
  if (boolean_terms.size() <= 0) {
    error_code = SqlErrors::INVALID_SEARCH_CONDITION;
    return;
  }

  for (auto boolean_term : boolean_terms) {
    ValidateBooleanFactor(boolean_term, error_code);
    if (error_code != SqlErrors::NO_ERROR)
      return;
  }
}
