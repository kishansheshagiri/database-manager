#include "sql_parser.h"

void SqlParser::SetQuery(std::string query) {
  input_query_ = query;
}

void SqlParser::Parse(SqlErrors::Type &error_code) {
  tokenizer_ = std::make_unique<Tokenizer>(input_query_);
  tokenizer_->Tokenize(error_code);
}
