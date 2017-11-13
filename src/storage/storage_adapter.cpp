#include "storage_adapter.h"

#include "base/debug.h"

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
  reset();
}

bool StorageAdapter::Initialize() {
  disk_->resetDiskIOs();
  disk_->resetDiskTimer();

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

template <typename Value> bool StorageAdapter::CreateTupleAndAppend(
    const std::string& relation_name, const std::vector<Value>& values) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  if (values.size() == 0) {
    DEBUG_MSG("Inserted list of values is empty");
    return false;
  }

  int offset = 0;
  Tuple tuple = relation->createTuple();
  for (auto value : values) {
    if (offset == tuple.getNumOfFields() || !tuple.setField(offset, value)) {
      DEBUG_MSG("Invalid values for relation");
      return false;
    }

    offset++;
  }

  appendTupleToRelation(relation, tuple);
  return true;
}

template <typename Value> bool StorageAdapter::CreateTupleAndAppend(
    const std::string& relation_name,
    const std::vector<std::string>& field_names,
    const std::vector<Value>& values) const {
  Relation *relation = schema_manager_->getRelation(relation_name);
  if (relation == nullptr) {
    DEBUG_MSG("Invalid relation name");
    return false;
  }

  if (field_names.size() == 0 || field_names.size() != values.size()) {
    DEBUG_MSG("Invalid fields for relation");
    return false;
  }

  int index = 0;
  Tuple tuple = relation->createTuple();
  for (auto field : field_names) {
    if (!tuple.setField(field, values[index])) {
      DEBUG_MSG("Invalid values for relation.");
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
