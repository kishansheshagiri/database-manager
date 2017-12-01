#ifndef SRC_PQP_QUERY_RUNNER_FACTORY_H
#define SRC_PQP_QUERY_RUNNER_FACTORY_H

#include "lqp/statement.h"
#include "lqp/query_node.h"
#include "pqp/query_runner.h"

class QueryRunnerFactory {
 public:
  QueryRunnerFactory(QueryNode *query_node);
  virtual ~QueryRunnerFactory() { }

  QueryRunner *Create() const;
 private:
  QueryRunnerFactory();

  QueryNode *query_node_;
};

#endif // SRC_PQP_QUERY_RUNNER_FACTORY_H
