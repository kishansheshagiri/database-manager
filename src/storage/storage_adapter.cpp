#include "storage_adapter.h"

#include <iomanip>
#include <regex>

#include "base/debug.h"

bool inline IsInteger(const std::string integer) {
  if (!std::regex_match(integer, std::regex("^[0-9]+$")) && integer != "NULL") {
    return false;
  }

  return true;
}

bool inline MatchFieldType(const Schema schema, const int offset,
    const std::string value) {
  std::string field_name = schema.getFieldName(offset);
  FIELD_TYPE field_type = schema.getFieldType(offset);

  if (field_type == INT) {
    if (!IsInteger(value)) {
      ERROR_MSG("Invalid integer value for attribute: " << field_name);
      return false;
    }
  } else if (field_type == STR20) {
    if (value.length() > 22) {
      ERROR_MSG("Length mismatch for value of attribtue: " << field_name);
    }
  } else {
    ERROR_MSG("Invalid field name");
    return false;
  }

  return true;
}

bool inline MatchFieldType(const Schema schema,
    const std::string field_name, const std::string value) {

  return MatchFieldType(schema, schema.getFieldOffset(field_name), value);
}

StorageAdapter *StorageAdapter::storage_adapter_ = nullptr;

StorageAdapter *StorageAdapter::Get() {
  if (!storage_adapter_)
    storage_adapter_ = new StorageAdapter();

  return storage_adapter_;
}

StorageAdapter::StorageAdapter()
  : available_memory_index_(0),
    disk_(new Disk()),
    main_memory_(new MainMemory()),
    schema_manager_(new SchemaManager(main_memory_, disk_)) {

}

StorageAdapter::~StorageAdapter() {
  DEBUG_MSG("Real elapse time = " << \
      ((double) (clock() - disk_start_time_) / CLOCKS_PER_SEC * 1000) << " ms");
  DEBUG_MSG("Calculated elapse time = " << disk_->getDiskTimer() << " ms");
  DEBUG_MSG("Calculated Disk I/Os = " << disk_->getDiskIOs());

  reset();
}

bool StorageAdapter::Initialize() {
  disk_->resetDiskIOs();
  disk_->resetDiskTimer();

  disk_start_time_ = clock();

  return true;
}

bool StorageAdapter::CreateRelation(const std::string& name,
    const std::vector<std::string>& fields,
    const std::vector<enum FIELD_TYPE>& field_types) const {
  if (name.empty()) {
    DEBUG_MSG("Empty relation name");
    return false;
  }

  if (fields.empty() || fields.size() != field_types.size()) {
    DEBUG_MSG("Number of fields not matching.");
    return false;
  }

  Schema schema(fields, field_types);
  Relation *relation = schema_manager_->createRelation(name, schema);
  if (relation == nullptr) {
    DEBUG_MSG("Unable to create relation");
    return false;
  }

  return true;
}

bool StorageAdapter::DeleteRelation(const std::string& name) const {
  DeleteAllTuples(name);
  return schema_manager_->deleteRelation(name);
}

bool StorageAdapter::CreateTupleAndAppend(const std::string& relation_name,
    const std::vector<std::string>& values) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  Schema schema = relation->getSchema();
  if (values.size() != schema.getNumOfFields()) {
    DEBUG_MSG("Inserted list of values is empty");
    return false;
  }

  int offset = 0;
  Tuple tuple = relation->createTuple();
  for (auto value : values) {
    if (!MatchFieldType(schema, offset, values[offset])) {
      return false;
    }

    if (schema.getFieldType(offset) == INT &&
        !tuple.setField(offset, std::stoi(values[offset]))) {
      DEBUG_MSG("Invalid values for relation");
      return false;
    } else if (!tuple.setField(offset, values[offset])) {
      DEBUG_MSG("Invalid values for relation");
      return false;
    }

    offset++;
  }

  appendTupleToRelation(relation, tuple);
  return true;
}

bool StorageAdapter::CreateTupleAndAppend(const std::string& relation_name,
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& values) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  Schema schema = relation->getSchema();
  if (field_names.size() != values.size() ||
      field_names.size() != schema.getNumOfFields()) {
    DEBUG_MSG("Invalid fields for relation");
    return false;
  }

  int index = 0;
  Tuple tuple = relation->createTuple();
  for (auto field : field_names) {
    if (!MatchFieldType(schema, field, values[index])) {
      return false;
    }

    if (schema.getFieldType(field) == INT) {
      std::string integer_value = values[index];
      if (values[index] == "NULL") {
        integer_value = std::to_string(-1);
      }

      if (!tuple.setField(field, std::stoi(integer_value))) {
        DEBUG_MSG("Invalid values for relation");
        return false;
      }
    } else if (schema.getFieldType(field) == STR20 &&
        !tuple.setField(field, values[index])) {
      return false;
    }

    index++;
  }

  appendTupleToRelation(relation, tuple);
  return true;
}

bool StorageAdapter::DeleteTuples(const std::string& relation_name,
    const int start_index) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  relation->deleteBlocks(start_index);
  return true;
}

bool StorageAdapter::DeleteAllTuples(const std::string& relation_name) const {
  return DeleteTuples(relation_name, 0);
}

bool StorageAdapter::IsValidColumnName(const std::string& table_name,
    const std::string& attribute_name) const {
  Relation *relation = schema_manager_->getRelation(table_name);
  if (relation == nullptr) {
    ERROR_MSG("Invalid table name: " << table_name);
    return false;
  }

  Schema schema = relation->getSchema();
  if (!schema.fieldNameExists(attribute_name)) {
    ERROR_MSG("Invalid attribute name '" << attribute_name << \
        "' for table '" << table_name << "'");
    return false;
  }

  return true;
}

int StorageAdapter::MainMemorySize() const {
  return main_memory_->getMemorySize();
}

void StorageAdapter::SetMainMemoryBlock(int memory_index, Block *block) const {
  main_memory_->setBlock(memory_index, *block);
}

int StorageAdapter::RelationBlockSize(const std::string relation_name) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name: " << relation_name);
    return 0;
  }

  return relation->getNumOfBlocks();
}

bool StorageAdapter::ReadRelationBlocks(const std::string relation_name,
    const int relation_start_index, const int memory_start_index,
    const int num_blocks, std::vector<Block *>& blocks) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name: " << relation_name);
    return false;
  }

  int adjusted_relation_num_blocks = relation_start_index + num_blocks >=
      relation->getNumOfBlocks() ?
          relation->getNumOfBlocks() - relation_start_index : num_blocks;

  int adjusted_memory_num_blocks = memory_start_index + num_blocks >=
      MainMemorySize() ?
          MainMemorySize() - memory_start_index : num_blocks;

  int adjusted_num_blocks = std::min(adjusted_relation_num_blocks,
      adjusted_memory_num_blocks);

  if (!relation->getBlocks(relation_start_index, memory_start_index,
      adjusted_num_blocks)) {
    DEBUG_MSG("Index out of bound. Index: " << relation_start_index <<
        ", Relation: " << relation_name);
    return false;
  }

  for (int index = 0; index < adjusted_num_blocks; index++) {
    blocks.push_back(main_memory_->getBlock(index));
  }

  return true;
}

bool StorageAdapter::InsertBlocksToRelation(const std::string relation_name,
    const int memory_start_index, const int relation_start_index,
    const int num_blocks) const {
  if (memory_start_index > MainMemorySize()) {
    ERROR_MSG("Invalid main memory index: " << memory_start_index);
    return false;
  }

  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    ERROR_MSG("Invalid relation name: " << relation_name);
    return false;
  }

  return relation->setBlocks(relation_start_index,
      memory_start_index, num_blocks);
}

bool StorageAdapter::AppendBlocksToRelation(const std::string relation_name,
    const int memory_start_index, const int num_blocks) const {
  if (memory_start_index > MainMemorySize()) {
    ERROR_MSG("Invalid main memory index: " << memory_start_index);
    return false;
  }

  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    ERROR_MSG("Invalid relation name: " << relation_name);
    return false;
  }

  return relation->setBlocks(relation->getNumOfBlocks(),
      memory_start_index, num_blocks);
}

bool StorageAdapter::DeleteRelationBlocks(const std::string relation_name,
    const int start_index) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    ERROR_MSG("Invalid relation name: " << relation_name);
    return false;
  }

  return relation->deleteBlocks(start_index);
}

// Debug APIs
bool StorageAdapter::Tuples(const std::string relation_name,
    TupleList& tuples_as_string) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    ERROR_MSG("Invalid relation name: " << relation_name);
    return false;
  }

  int block_count = relation->getNumOfBlocks();
  int last_count = 0;

  while (block_count > 0) {
    int current_block_count = block_count > MainMemorySize() ?
        MainMemorySize() : block_count;
    if (!relation->getBlocks(last_count, 0, current_block_count)) {
      DEBUG_MSG("");
      return false;
    }

    std::vector<Tuple> tuples = main_memory_->getTuples(0, current_block_count);
    for (auto tuple : tuples) {
      std::vector<std::string> fields;
      for (int index = 0; index < tuple.getNumOfFields(); index++) {
        if (tuple.getSchema().getFieldType(index) == INT) {
          int value = tuple.getField(index).integer;
          std::string integer_value = std::to_string(value);
          if (value == -1) {
            integer_value = "NULL";
          }

          fields.push_back(integer_value);
        } else if (tuple.getSchema().getFieldType(index) == STR20) {
          fields.push_back(*(tuple.getField(index).str));
        } else {
          DEBUG_MSG("");
          return false;
        }
      }

      tuples_as_string.push_back(fields);
    }

    last_count += current_block_count;
    block_count -= current_block_count;
    clearMainMemoryBlocks();
  }

  return true;
}

void StorageAdapter::PrintTupleList(const std::string relation_name,
    const TupleList tuples) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid table name: " << relation_name);
    return;
  }

  std::vector<std::string> field_names = relation->getSchema().getFieldNames();

  DEBUG_MSG_SINGLE_LINE("\n");
  DEBUG_MSG_SINGLE_LINE(
      "+" << std::string(19 * field_names.size(), '-') << "+");
  DEBUG_MSG_SINGLE_LINE("\n");

  for (auto field_name : field_names) {
    DEBUG_MSG_SINGLE_LINE("|" << setw(18) << std::left << field_name);
  }

  DEBUG_MSG_SINGLE_LINE(" |\n");
  DEBUG_MSG_SINGLE_LINE(
      "+" << std::string(19 * field_names.size(), '-') << "+");
  DEBUG_MSG_SINGLE_LINE("\n");

  for (auto tuple : tuples) {
    for (auto field : tuple) {
      DEBUG_MSG_SINGLE_LINE("|" << setw(18) << std::left << field);
    }

    DEBUG_MSG_SINGLE_LINE(" |\n");
  }

  DEBUG_MSG_SINGLE_LINE(
      "+" << std::string(19 * field_names.size(), '-') << "+");
  DEBUG_MSG_SINGLE_LINE("\n");
}

// Private methods
void StorageAdapter::reset() {
  available_memory_index_ = 0;
  delete disk_;
  delete main_memory_;
  delete schema_manager_;
}

void StorageAdapter::appendTupleToRelation(Relation* relation,
    Tuple& tuple) const {
  if (relation == nullptr) {
    DEBUG_MSG("Relation is null");
    return;
  }

  Block *block;
  if (relation->getNumOfBlocks() == 0) {
    LOG_MSG("The relation is empty. creating new block...");
    block = main_memory_->getBlock(available_memory_index_);
    block->clear();
    block->appendTuple(tuple);
    relation->setBlock(relation->getNumOfBlocks(), available_memory_index_);
  } else {
    LOG_MSG("The relation is not empty.");
    relation->getBlock(relation->getNumOfBlocks() - 1, available_memory_index_);
    block = main_memory_->getBlock(available_memory_index_);

    if (block->isFull()) {
      LOG_MSG("Block is full, creating new block...");
      block->clear();
      block->appendTuple(tuple);
      relation->setBlock(relation->getNumOfBlocks(), available_memory_index_);
    } else {
      LOG_MSG("Block is not full, appending...");
      block->appendTuple(tuple);
      relation->setBlock(relation->getNumOfBlocks() - 1,
          available_memory_index_);
    }
  }

  block->clear();
}

void StorageAdapter::clearMainMemoryBlocks() const {
  for (int index = 0; index < MainMemorySize(); index++) {
    Block *block = main_memory_->getBlock(index);
    block->clear();
  }
}
