#ifndef SRC_SQL_PARSER_H
#define SRC_SQL_PARSER_H

#include <string>
#include <memory>

#include "tokenizer.h"

class SqlParser {
 public:
  void SetQuery(std::string query);
  void Parse(SqlErrors::Type &error_code);

 private:
  void ValidateTableName(const std::string& name,
      SqlErrors::Type& error_code);
  void ValidateAttributeName(const std::string& name,
      SqlErrors::Type& error_code);
  void ValidateColumnName(const std::string& name,
      SqlErrors::Type& error_code);
  void ValidateLiteral(const std::string& literal,
      SqlErrors::Type& error_code);
  void CreateAndValidateAttributeList(const std::string& name,
      SqlErrors::Type& error_code, std::vector<std::string>& attributes);
  void ValidateValue(const std::string& value,
      SqlErrors::Type& error_code);
  void CreateAndValidateValueList(const std::string& name,
      SqlErrors::Type& error_code, std::vector<std::string>& values);
  void ValidateTerm(const std::string& term,
      SqlErrors::Type& error_code);
  void ValidateExpression(const std::string& expression,
      SqlErrors::Type& error_code);
  void ValidateBooleanFactor(const std::string& boolean_factor,
      SqlErrors::Type& error_code);
  void ValidateBooleanTerm(const std::string& boolean_term,
      SqlErrors::Type& error_code);
  void ValidateSearchCondition(const std::string& condition,
      SqlErrors::Type& error_code);

  std::string input_query_;
  std::unique_ptr<Tokenizer> tokenizer_;
};

#endif //SRC_SQL_PARSER_H
