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

void QueryManager::SetQuery(std::string query, bool &success) {
  if (query.empty())
    success = false;
  else
    sql_query = query;
}

void QueryManager::ExecuteQuery(bool &success) {
  parser->SetQuery(sql_query);
}
