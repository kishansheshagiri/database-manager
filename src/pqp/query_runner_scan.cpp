#include "pqp/query_runner_scan.h"

#include <string>
#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"
#include "storage/storage_manager_headers.h"

QueryRunnerScan::QueryRunnerScan(QueryNode *query_node)
  : QueryRunner(query_node),
    next_relation_start_index_(0),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerScan::~QueryRunnerScan() {
}

bool QueryRunnerScan::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 0) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_TABLE_SCAN;
    return false;
  }

  std::string table_name;
  if (!Node()->TableName(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  SetCallback(callback);

  std::vector<std::string> field_names;
  if (!Storage()->RelationFieldNames(table_name, field_names)) {
    DEBUG_MSG("");
    return false;
  }

  std::string temp_relation_name;
  if (!Storage()->CreateDummyRelation("Scan_", field_names,
      temp_relation_name)) {
    DEBUG_MSG("");
    return false;
  }

  bool created = false;
  Tuple field_name_tuple = Storage()->CreateTuple(
      temp_relation_name, field_names, created);
  if (!created) {
    DEBUG_MSG("");
    return false;
  }

  std::vector<Tuple> field_name_tuple_list;
  field_name_tuple_list.push_back(field_name_tuple);
  Callback()(field_name_tuple_list, true);

  Storage()->DeleteDummyRelation(temp_relation_name);

  bool respond_once = false;
  std::vector<Block *> blocks;

  int memory_start_index =
      !scan_params_.one_pass_ && !scan_params_.use_begin_blocks_ ?
          Storage()->MainMemorySize() / 2 : 0;
  int relation_start_index = 0;
  while (!respond_once &&
      Storage()->ReadRelationBlocks(table_name, relation_start_index,
          memory_start_index, Storage()->MainMemorySize(), blocks)) {
    std::vector<Tuple> tuples;
    for (auto block : blocks) {
      std::vector<Tuple> insert_tuples = block->getTuples();
      tuples.insert(tuples.end(), insert_tuples.begin(), insert_tuples.end());
    }

    if (!Callback()(tuples, false)) {
      DEBUG_MSG("");
      return false;
    }

    for (auto block : blocks) {
      block->clear();
    }

    relation_start_index += blocks.size();
    next_relation_start_index_ = relation_start_index;
    respond_once = scan_params_.one_pass_ ? false : true;
  }

  return true;
}

void QueryRunnerScan::PassScanParams(ScanParams params) {
  scan_params_ = params;
}

bool QueryRunnerScan::TableSize(int& size) {
  if (Node() == nullptr || Node()->ChildrenCount() != 0) {
    DEBUG_MSG("");
    return false;
  }

  std::string table_name;
  if (!Node()->TableName(table_name)) {
    DEBUG_MSG("");
    return false;
  }

  size = Storage()->RelationTupleSize(table_name);
  return true;
}

bool QueryRunnerScan::ResultCallback(std::vector<Tuple>& tuples,
    bool headers) {
  DEBUG_MSG("EMPTY FUNCTION");
  return true;
}
