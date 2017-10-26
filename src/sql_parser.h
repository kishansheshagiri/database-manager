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
  std::string input_query_;
  std::unique_ptr<Tokenizer> tokenizer_;
};

#endif //SRC_SQL_PARSER_H
