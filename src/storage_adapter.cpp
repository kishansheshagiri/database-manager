#include "storage_adapter.h"

#include "debug.h"

StorageAdapter *StorageAdapter::storage_adapter_ = nullptr;

StorageAdapter *StorageAdapter::Get() {
  if (!storage_adapter_)
    storage_adapter_ = new StorageAdapter();

  return storage_adapter_;
}

StorageAdapter::StorageAdapter()
  : disk_(new Disk()),
    main_memory_(new MainMemory()),
    schema_manager_(new SchemaManager(main_memory_, disk_)) {

}

StorageAdapter::~StorageAdapter() {

}

bool StorageAdapter::Initialize() {
  disk_->resetDiskIOs();
  disk_->resetDiskTimer();

  return true;
}

bool StorageAdapter::CreateRelation(const std::string& name,
    const std::vector<std::string>& fields,
    const std::vector<enum FIELD_TYPE>& field_types,
    Relation *relation) {
  if (name.empty()) {
    DEBUG_MSG("Empty relation name");
    return false;
  }

  if (fields.empty() || fields.size() != field_types.size()) {
    DEBUG_MSG("Number of fields not matching.");
    return false;
  }

  Schema schema(fields, field_types);
  relation = schema_manager_->createRelation(name, schema);
  return true;
}
