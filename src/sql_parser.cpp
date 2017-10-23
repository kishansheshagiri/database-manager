#include "sql_parser.h"

void SqlParser::SetQuery(std::string query) {
  input_query = query;
}

bool SqlParser::Parse() {
  return true;
}
