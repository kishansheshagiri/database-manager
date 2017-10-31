#ifndef SRC_STORAGE_ADAPTER_H
#define SRC_STORAGE_ADAPTER_H

#include <string>
#include <vector>

#include "storage_manager_headers.h"

class StorageAdapter {
 public:
  // Singleton method
  static StorageAdapter *Get();

  bool Initialize();

  bool CreateRelation(const std::string& name,
                      const std::vector<std::string>& fields,
                      const std::vector<enum FIELD_TYPE>& field_types,
                      Relation *relation);
 private:
  StorageAdapter();
  virtual ~StorageAdapter();

  Disk *disk_;
  MainMemory *main_memory_;
  SchemaManager *schema_manager_;

  // Singleton instance
  static StorageAdapter *storage_adapter_;
};

#endif // SRC_STORAGE_ADAPTER_H
