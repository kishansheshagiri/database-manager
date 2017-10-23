#include "sql_parser.h"

void SqlParser::SetQuery(std::string query) {
  input_query_ = query;
}

bool SqlParser::Parse() {
  return true;
}
