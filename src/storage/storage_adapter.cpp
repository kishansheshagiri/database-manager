#include "storage_adapter.h"

#include <iomanip>
#include <regex>

#include "base/debug.h"

bool inline IsInteger(const std::string integer) {
  return std::regex_match(integer, std::regex("^[0-9]+$"));
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
    if (value.length() > 20) {
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

    if (schema.getFieldType(field) == INT &&
        !tuple.setField(field, std::stoi(values[index]))) {
      DEBUG_MSG("Invalid values for relation");
      return false;
    } else if (schema.getFieldType(field) == STR20 &&
        !tuple.setField(field, values[index])) {
      return false;
    }

    index++;
  }

  appendTupleToRelation(relation, tuple);
  return true;
}

bool StorageAdapter::DeleteAllTuples(const std::string& relation_name) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  relation->deleteBlocks(0);
  return true;
}

bool StorageAdapter::DeleteTuples(const std::string& relation_name,
    const WhereClauseHelper where_clause_helper) const {
  DEBUG_MSG("NOT IMPLEMENTED");
  return false;
}

bool StorageAdapter::Tuples(const std::string relation_name,
    TupleList& tuples_as_string) const {
  Relation *relation = schema_manager_->getRelation(relation_name);

  int block_count = relation->getNumOfBlocks();
  int last_count = 0;

  while (block_count > 0) {
    int current_block_count = block_count > main_memory_->getMemorySize() ?
        main_memory_->getMemorySize() : block_count;
    if (!relation->getBlocks(last_count, 0, current_block_count)) {
      DEBUG_MSG("");
      return false;
    }

    std::vector<Tuple> tuples = main_memory_->getTuples(0, current_block_count);
    for (auto tuple : tuples) {
      std::vector<std::string> fields;
      for (int index = 0; index < tuple.getNumOfFields(); index++) {
        if (tuple.getSchema().getFieldType(index) == INT) {
          fields.push_back(std::to_string(tuple.getField(index).integer));
        } else if (tuple.getSchema().getFieldType(index) == STR20) {
          fields.push_back(*(tuple.getField(index).str));
        } else {
          DEBUG_MSG("");
          return false;
        }
      }

      tuples_as_string.push_back(fields);
    }

    last_count += block_count;
    block_count -= current_block_count;
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
  for (auto field_name : field_names) {
    DEBUG_MSG_SINGLE_LINE(setw(19) << field_name);
  }

  DEBUG_MSG_SINGLE_LINE("\n");

  for (auto tuple : tuples) {
    for(auto field : tuple) {
      DEBUG_MSG_SINGLE_LINE(setw(19) << field);
    }

    DEBUG_MSG_SINGLE_LINE("\n");
  }
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
