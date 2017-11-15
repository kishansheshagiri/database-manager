#include "statement_factory.h"

#include <memory>

#include "lqp/create_table_statement.h"
#include "lqp/drop_table_statement.h"
#include "lqp/select_statement.h"
#include "lqp/delete_statement.h"
#include "lqp/insert_statement.h"

StatementFactory::StatementFactory(SqlNode *root)
    : root_(root) {
}

Statement *StatementFactory::Create() const {
  if (root_ == nullptr) {
    return nullptr;
  }

  SqlNode::NodeType node_type = root_->Type();
  switch (node_type) {
    case SqlNode::NODE_TYPE_CREATE_TABLE_STATEMENT:
      return new CreateTableStatement(root_);
    case SqlNode::NODE_TYPE_DROP_TABLE_STATEMENT:
      return new DropTableStatement(root_);
    case SqlNode::NODE_TYPE_SELECT_STATEMENT:
      return new SelectStatement(root_);
    case SqlNode::NODE_TYPE_DELETE_STATEMENT:
      return new DeleteStatement(root_);
    case SqlNode::NODE_TYPE_INSERT_STATEMENT:
      return new InsertStatement(root_);
    default:
      return nullptr;
  }
}
