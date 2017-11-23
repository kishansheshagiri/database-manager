#include "statement_factory.h"

#include <memory>

#include "lqp/statement_create_table.h"
#include "lqp/statement_delete.h"
#include "lqp/statement_drop_table.h"
#include "lqp/statement_insert.h"
#include "lqp/statement_select.h"

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
      return new StatementCreateTable(root_);
    case SqlNode::NODE_TYPE_DELETE_STATEMENT:
      return new StatementDelete(root_);
    case SqlNode::NODE_TYPE_DROP_TABLE_STATEMENT:
      return new StatementDropTable(root_);
    case SqlNode::NODE_TYPE_INSERT_STATEMENT:
      return new StatementInsert(root_);
    case SqlNode::NODE_TYPE_SELECT_STATEMENT:
      return new StatementSelect(root_);
    default:
      return nullptr;
  }
}
