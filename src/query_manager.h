#ifndef SRC_QUERY_MANAGER_H
#define SRC_QUERY_MANAGER_H

#include <memory>
#include <string>

#include "sql_parser.h"

class QueryManager {
 public:
  // Singleton method
  static QueryManager *Get();

  // Set SQL Query
  void SetQuery(std::string query, bool &success);
  void ExecuteQuery(bool &success);

 private:
  QueryManager();

  std::unique_ptr<SqlParser> parser;
  std::string sql_query;

  // Singleton instance
  static QueryManager *manager;
};

#endif //SRC_QUERY_MANAGER_H
