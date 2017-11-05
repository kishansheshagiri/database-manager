#include "query_manager.h"

#include "debug.h"
#include "sql_node.h"

QueryManager *QueryManager::manager_ = nullptr;

QueryManager::QueryManager() {
  parser_ = std::make_unique<SqlParser>();
}

QueryManager *QueryManager::Get() {
  if (!manager_)
    manager_ = new QueryManager();

  return manager_;
}

void QueryManager::SetQuery(std::string query, SqlErrors::Type &error_code) {
  if (query.empty()) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    error_code = SqlErrors::EMPTY_STATEMENT;
  } else {
    sql_query_ = query;
  }
}

void QueryManager::ExecuteQuery(SqlErrors::Type &error_code) {
  parser_->SetQuery(sql_query_);

  SqlNode *sql_node = new SqlNode(SqlNode::NODE_TYPE_STATEMENT);
  parser_->Parse(sql_node, error_code);
  if (error_code != SqlErrors::NO_ERROR) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    delete sql_node;
    return;
  }
}
