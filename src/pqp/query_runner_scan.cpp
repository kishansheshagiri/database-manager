#include "pqp/query_runner_scan.h"

#include <string>
#include <vector>

#include "base/debug.h"
#include "storage/storage_manager_headers.h"

QueryRunnerScan::QueryRunnerScan(QueryNode *query_node)
  : QueryRunner(query_node),
    next_relation_start_index_(0),
    error_code_(SqlErrors::NO_ERROR) {
  if (!Node()->TableName(table_name_)) {
    ERROR_MSG("");
  }
}

QueryRunnerScan::~QueryRunnerScan() {
}

bool QueryRunnerScan::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 0) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_TABLE_SCAN;
    return false;
  }

  return true;
}

bool QueryRunnerScan::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  bool respond_once = false;
  std::vector<Block *> blocks;

  if (scan_params_.start_index_ > Storage()->MainMemorySize()) {
    ERROR_MSG("Main memory full");
    error_code = SqlErrors::ERROR_TABLE_SCAN;
    return false;
  }

  int memory_start_index = scan_params_.start_index_;
  int relation_start_index = 0;
  int num_blocks = scan_params_.num_blocks_ <= 0 ?
      Storage()->MainMemorySize() : scan_params_.num_blocks_;
  while (!respond_once &&
      Storage()->ReadRelationBlocks(table_name_, relation_start_index,
          memory_start_index, num_blocks, blocks)) {
    std::vector<Tuple> tuples;
    for (auto block : blocks) {
      std::vector<Tuple> insert_tuples = block->getTuples();
      tuples.insert(tuples.end(), insert_tuples.begin(), insert_tuples.end());
      block->clear();
    }

    if (!Callback()(this, tuples)) {
      DEBUG_MSG("");
      return false;
    }

    relation_start_index += blocks.size();
    next_relation_start_index_ = relation_start_index;
    respond_once = scan_params_.multi_scan_ ? false : true;
  }

  return true;
}

void QueryRunnerScan::PassScanParams(ScanParams params) {
  scan_params_ = params;
}

bool QueryRunnerScan::TableName(std::string& table_name) {
  if (Node() == nullptr || Node()->ChildrenCount() != 0) {
    DEBUG_MSG("");
    return false;
  }

  if (!Node()->TableName(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}

bool QueryRunnerScan::TableSize(int& blocks, int& tuples) {
  if (Node() == nullptr || Node()->ChildrenCount() != 0) {
    DEBUG_MSG("");
    return false;
  }

  std::string table_name;
  if (!Node()->TableName(table_name) ||
      !Storage()->IsValidRelation(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  blocks = Storage()->RelationBlockSize(table_name);
  tuples = Storage()->RelationTupleSize(table_name);
  return true;
}

bool QueryRunnerScan::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  DEBUG_MSG("EMPTY FUNCTION");
  return true;
}
