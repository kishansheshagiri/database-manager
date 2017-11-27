#include "pqp/where_clause_helper_delete.h"

#include "base/debug.h"
#include "storage/storage_adapter.h"

WhereClauseHelperDelete::WhereClauseHelperDelete()
  : error_code_(SqlErrors::NO_ERROR) {
}

WhereClauseHelperDelete::~WhereClauseHelperDelete() {

}

bool WhereClauseHelperDelete::Initialize(SqlNode *where_node,
    std::string table_name) {
  table_name_ = table_name;

  return WhereClauseHelper::Initialize(where_node);
}

bool WhereClauseHelperDelete::Execute(SqlErrors::Type& error_code) {
  std::vector<Block *> blocks;
  int relation_start_index = 0;
  int num_blocks = Storage()->MainMemorySize();

  int empty_block_index = 0;
  int relation_insert_index = 0;
  int memory_start_index = 0;
  while (Storage()->ReadRelationBlocks(table_name_, relation_start_index,
      memory_start_index, num_blocks, blocks)) {
    Block *empty_block = blocks[empty_block_index];
    for (auto block : blocks) {
      std::vector<Tuple> modified_tuples;
      std::vector<Tuple> tuples = block->getTuples();
      block->clear();

      for (auto tuple : tuples) {
        bool evaluate_result = Evaluate(&tuple, error_code);
        if (error_code != SqlErrors::NO_ERROR) {
          DEBUG_MSG("");
          return false;
        }

        if (!evaluate_result) {
          modified_tuples.push_back(tuple);
        }
      }

      int tuple_index = 0;
      while (tuple_index < modified_tuples.size()) {
        for (; tuple_index < modified_tuples.size() && !empty_block->isFull();
            tuple_index++) {
          empty_block->appendTuple(modified_tuples[tuple_index]);
        }

        if (empty_block->isFull() && empty_block_index < blocks.size() - 1) {
          empty_block = blocks[++empty_block_index];
        }
      }
    }

    int insert_blocks_size = empty_block->isFull() ?
        empty_block_index + 1 : empty_block_index;
    if (insert_blocks_size > 0) {
      DEBUG_MSG("Inserting into " << table_name_ << " from " <<
          relation_start_index << " a total of " << insert_blocks_size);
      Storage()->InsertBlocksToRelation(table_name_, 0, relation_insert_index,
          insert_blocks_size);
    }

    relation_start_index += blocks.size();
    relation_insert_index += insert_blocks_size;

    Block *temp_block = nullptr;
    if (!empty_block->isFull() && !empty_block->isEmpty()) {
      temp_block = blocks[empty_block_index];
    }

    blocks.clear();
    if (temp_block != nullptr) {
      blocks.push_back(temp_block);
      Storage()->SetMainMemoryBlock(0, temp_block);
    }

    memory_start_index = blocks.size();
    empty_block_index = blocks.size();

    if (relation_start_index >= Storage()->RelationBlockSize(table_name_)) {
      if (!blocks.empty()) {
        Storage()->InsertBlocksToRelation(table_name_, 0,
            relation_insert_index++, 1);
      }
    }
  }

  Storage()->DeleteTuples(table_name_, relation_insert_index);

  return true;
}

bool WhereClauseHelperDelete::Evaluate(Tuple *tuple,
    SqlErrors::Type& error_code) {
  if (tuple == nullptr) {
    error_code = SqlErrors::UNKNOWN_ERROR;
    DEBUG_MSG("Invalid tuple");
    return false;
  }

  bool condition_result = HandleSearchCondition(tuple);
  if (error_code_ != SqlErrors::NO_ERROR) {
    error_code = error_code_;
    return false;
  }

  return condition_result;
}

// Private methods
std::string WhereClauseHelperDelete::HandleColumnName(
    SqlNode *column_name) {
  std::vector<SqlNode *> children = column_name->Children();
  if (children.size() == 2 && table_name_ != children[0]->Data()) {
    DEBUG_MSG("WHERE clause contains invalid table name");
    error_code_ = SqlErrors::INVALID_TABLE_NAME;
    return std::string();
  }

  std::string attribute_name;
  if (children.size() == 2) {
    attribute_name = children[1]->Data();
  } else {
    attribute_name = children[0]->Data();
  }

  if (!Storage()->IsValidColumnName(table_name_, attribute_name)) {
    DEBUG_MSG("Column name invalid for the table");
    error_code_ = SqlErrors::INVALID_COLUMN_NAME;
    return std::string();
  }

  std::string field_value;
  ValueFromTuple(attribute_name, field_value, error_code_);
  return field_value;
}
