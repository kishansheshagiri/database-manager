#include "pqp/query_runner_sort.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"
#include "storage/storage_manager_headers.h"

typedef struct CompareTuples {
  CompareTuples(const QueryRunnerSort *query_runner)
      : query_runner_(query_runner) { }
  const QueryRunnerSort *query_runner_;

  bool operator() (
      const Tuple& first, const Tuple& second) const {
    std::string sort_column = query_runner_->SortColumn();
    Schema schema = first.getSchema();

    if (sort_column == "*" || sort_column.empty()) {
      sort_column = schema.getFieldName(0);
    }

    if (schema.getFieldType(sort_column) == INT) {
      int field_value_first = first.getField(sort_column).integer;
      int field_value_second = second.getField(sort_column).integer;

      return field_value_first < field_value_second;
    } else if (schema.getFieldType(sort_column) == STR20) {
      std::string field_value_first = *(first.getField(sort_column).str);
      std::string field_value_second = *(second.getField(sort_column).str);

      return field_value_first < field_value_second;
    }

    return true;
  }
} CompareTuples;

QueryRunnerSort::QueryRunnerSort(QueryNode *query_node)
  : QueryRunner(query_node),
    block_size_(-1),
    tuples_per_block_(-1),
    memory_constraint_(Storage()->MainMemorySize() - 1),
    scan_params_passed_(false),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerSort::~QueryRunnerSort() {
}

bool QueryRunnerSort::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SORT;
    return false;
  }

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));
  if (!Node()->SortColumn(sort_column_)) {
    DEBUG_MSG("");
    return false;
  }

  return ChildRunner()->Initialize(error_code);
}

bool QueryRunnerSort::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  if (scan_params_passed_ && (scan_params_.num_blocks_ == 1 ||
      scan_params_.num_blocks_ + scan_params_.start_index_ - 1 >
          memory_constraint_)) {
    scan_params_.num_blocks_ = memory_constraint_ - scan_params_.start_index_;
    memory_constraint_ = scan_params_.num_blocks_;
  }

  if (!scan_params_passed_) {
    scan_params_.num_blocks_ = memory_constraint_;
  }

  ChildRunner()->PassScanParams(scan_params_);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerSort::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SORT;
    return false;
  }

  if (block_size_ >= 0 && block_size_ <= memory_constraint_) {
    return true;
  }

  std::vector<int> sublist_block_indices(sublist_size_list_.size(), 0);
  std::vector<Block *> blocks;
  int memory_index = scan_params_.start_index_;
  int relation_index = 0;
  for (auto index = 0; index < sublist_size_list_.size(); index++) {
    std::vector<Block *> sublist_blocks;
    if (!Storage()->ReadRelationBlocks(intermediate_relation_name_,
        relation_index, memory_index++, 1, sublist_blocks)) {
      DEBUG_MSG("");
      return false;
    }

    relation_index += sublist_size_list_[index];
    blocks.push_back(sublist_blocks[0]);
  }

  std::vector<int> block_tuple_indices(sublist_size_list_.size(), 0);
  while (!sublistIterated(sublist_size_list_, sublist_block_indices)) {
    std::vector<Tuple> minimum_tuples;
    for (auto index = 0; index < sublist_block_indices.size(); index++) {
      if (block_tuple_indices[index] == blocks[index]->getNumTuples()) {
        if (sublist_block_indices[index] < sublist_size_list_[index] - 1) {
          std::vector<Block *> sublist_blocks;
          if (!Storage()->ReadRelationBlocks(intermediate_relation_name_,
              ++sublist_block_indices[index], scan_params_.start_index_ + index,
              1, sublist_blocks)) {
            DEBUG_MSG("");
            return false;
          }

          blocks[index] = sublist_blocks[0];
          block_tuple_indices[index] = 0;
        }
      }

      if (sublist_block_indices[index] < sublist_size_list_[index] &&
          block_tuple_indices[index] < blocks[index]->getNumTuples()) {
        minimum_tuples.push_back(
            blocks[index]->getTuple(
                block_tuple_indices[index]++));
      }
    }

    if (minimum_tuples.size()) {
      std::sort(minimum_tuples.begin(),
          minimum_tuples.end(), CompareTuples(this));
      Tuple minimum_tuple = minimum_tuples[0];
    }
    Callback()(this, minimum_tuples);
  }

  return true;
}

void QueryRunnerSort::PassScanParams(ScanParams params) {
  scan_params_ = params;
  if (ChildRunner()) {
    ChildRunner()->PassScanParams(params);
  }

  scan_params_passed_ = true;
}

bool QueryRunnerSort::HasSortNode() const {
  return true;
}

bool QueryRunnerSort::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  if (tuples.empty()) {
    return true;
  }

  if (tuples_per_block_ == -1) {
    tuples_per_block_ = tuples[0].getSchema().getTuplesPerBlock();
  }

  int tuple_size;
  if (TableSize(block_size_, tuple_size) && block_size_ <= memory_constraint_) {
    std::sort(tuples.begin(), tuples.end(), CompareTuples(this));
    return Callback()(this, tuples);
  }

  if (intermediate_relation_name_.empty()) {
    if (!createIntermediateRelation(tuples[0])) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_SORT;
      return false;
    }
  }

  std::sort(tuples.begin(), tuples.end(), CompareTuples(this));

  for (auto& tuple : tuples) {
    Storage()->AppendTupleUsing(intermediate_relation_name_, tuple,
        memory_constraint_);
  }

  Storage()->PushLastBlock(intermediate_relation_name_, memory_constraint_);
  int block_count = tuples.size() / tuples_per_block_;
  int adjusted_block_count = tuples.size() % tuples_per_block_ ?
      block_count + 1 : block_count;

  sublist_size_list_.push_back(adjusted_block_count);
  if (adjusted_block_count > memory_constraint_ ||
      sublist_size_list_.size() > memory_constraint_) {
    ERROR_MSG("Out of memory");
    return false;
  }

  return true;
}

std::string QueryRunnerSort::SortColumn() const {
  return sort_column_;
}

bool QueryRunnerSort::createIntermediateRelation(Tuple& tuple) {
  Schema schema = tuple.getSchema();
  std::vector<std::string> field_names = schema.getFieldNames();
  std::vector<enum FIELD_TYPE> field_types = schema.getFieldTypes();

  if (!Storage()->CreateDummyRelation("Sort_", field_names, field_types,
      intermediate_relation_name_)) {
    DEBUG_MSG("");
    return false;
  }

  MarkTemporaryRelation(intermediate_relation_name_);

  return true;
}

bool QueryRunnerSort::sublistIterated(std::vector<int>& sublist_size_list,
    std::vector<int>& sublist_block_indices) {
  if (sublist_size_list.size() != sublist_block_indices.size()) {
    return false;
  }

  for (auto index = 0; index < sublist_size_list.size(); index++) {
    if (sublist_block_indices[index] < sublist_size_list[index] - 1) {
      return false;
    }
  }

  return true;
}
