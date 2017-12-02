#include "pqp/query_runner_scan.h"

#include <chrono>
#include <string>
#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"
#include "storage/storage_manager_headers.h"

QueryRunnerScan::QueryRunnerScan(QueryNode *query_node)
  : QueryRunner(query_node),
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

  std::chrono::milliseconds time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch());
  std::string temp_relation_name = "Scan_" + std::to_string(time_ms.count());

  std::vector<enum FIELD_TYPE> field_types(field_names.size(), STR20);
  if (!Storage()->CreateRelation(temp_relation_name,
      field_names, field_types)) {
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

  std::vector<Block *> blocks;

  int memory_start_index = 0;
  int relation_start_index = 0;
  int num_blocks = Storage()->MainMemorySize();
  while (Storage()->ReadRelationBlocks(table_name, relation_start_index,
      memory_start_index, num_blocks, blocks)) {
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
  }

  return true;
}

bool QueryRunnerScan::ResultCallback(std::vector<Tuple>& tuples,
    bool headers) {
  DEBUG_MSG("EMPTY FUNCTION");
  return true;
}
