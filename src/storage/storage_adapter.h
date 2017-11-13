#ifndef SRC_STORAGE_ADAPTER_H
#define SRC_STORAGE_ADAPTER_H

#include <ctime>
#include <string>
#include <vector>

#include "storage/storage_manager_headers.h"

class StorageAdapter {
 public:
  // Singleton method
  static StorageAdapter *Get();

  bool Initialize();
  bool CreateRelation(const std::string& name,
                      const std::vector<std::string>& fields,
                      const std::vector<enum FIELD_TYPE>& field_types) const;
  bool DeleteRelation(const std::string& name) const;
  bool CreateTuple() const;
  template <typename Value> bool CreateTupleAndAppend(
      const std::string& relation_name,
      const std::vector<Value>& values) const;
  template <typename Value> bool CreateTupleAndAppend(
      const std::string& relation_name,
      const std::vector<std::string>& field_names,
      const std::vector<Value>& values) const;
  bool DeleteAllTuples(const std::string& relation_name) const;

 private:
  StorageAdapter();
  ~StorageAdapter();

  void reset();
  void appendTupleToRelation(Relation* relation, Tuple& tuple) const;

  // Memory manager
  int available_memory_index_;

  clock_t disk_start_time_;

  Disk *disk_;
  MainMemory *main_memory_;
  SchemaManager *schema_manager_;

  // Singleton instance
  static StorageAdapter *storage_adapter_;
};

#endif // SRC_STORAGE_ADAPTER_H
