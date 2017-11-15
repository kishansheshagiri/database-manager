#ifndef SRC_LQP_STATEMENT_FACTORY_H
#define SRC_LQP_STATEMENT_FACTORY_H

#include "lqp/statement.h"
#include "parser/sql_node.h"

class StatementFactory {
 public:
  StatementFactory(SqlNode *root);
  virtual ~StatementFactory() { }

  Statement *Create() const;
 private:
  StatementFactory();

  SqlNode *root_;
};

#endif // SRC_LQP_STATEMENT_FACTORY_H
