#ifndef SRC_SQL_PARSER_H
#define SRC_SQL_PARSER_H

#include <string>

class SqlParser {
 public:
  void SetQuery(std::string query);
  bool Parse();

 private:
  std::string input_query;
};

#endif //SRC_SQL_PARSER_H
