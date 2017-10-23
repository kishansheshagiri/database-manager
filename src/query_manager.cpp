#include "query_manager.h"

QueryManager *QueryManager::manager = nullptr;

QueryManager::QueryManager() {
  parser = std::make_unique<SqlParser>();
}

QueryManager *QueryManager::Get() {
  if (!manager)
    manager = new QueryManager();

  return manager;
}

void QueryManager::SetQuery(std::string query, SqlErrors::Type &error_code) {
  if (query.empty())
    error_code = SqlErrors::EMPTY_STATEMENT;
  else
    sql_query = query;
}

void QueryManager::ExecuteQuery(SqlErrors::Type &error_code) {
  parser->SetQuery(sql_query);
}
