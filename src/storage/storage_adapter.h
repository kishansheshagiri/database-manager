#ifndef SRC_STORAGE_ADAPTER_H
#define SRC_STORAGE_ADAPTER_H

#include <ctime>
#include <string>
#include <vector>

#include "storage/storage_manager_headers.h"

typedef std::vector<std::vector<std::string> > TupleList;

class StorageAdapter {
 public:
  // Singleton method
  static StorageAdapter *Get();

  bool Initialize();
  bool CreateRelation(const std::string& name,
                      const std::vector<std::string>& fields,
                      const std::vector<enum FIELD_TYPE>& field_types) const;
  bool DeleteRelation(const std::string& name) const;
  bool CreateTupleAndAppend(const std::string& relation_name,
      const std::vector<std::string>& values) const;
  bool CreateTupleAndAppend(const std::string& relation_name,
      const std::vector<std::string>& field_names,
      const std::vector<std::string>& values) const;
  bool DeleteAllTuples(const std::string& relation_name) const;

  bool IsValidColumnName(const std::string& table_name,
      const std::string& attribute_name) const;

  int MainMemorySize() const;
  bool ReadRelationBlocks(const std::string relation_name,
      const int relation_start_index, const int num_blocks,
      std::vector<Block *>& blocks) const;
  bool InsertBlocksToRelation(const std::string relation_name,
      const int memory_start_index, const int relation_start_index,
      const int num_blocks) const;
  bool AppendBlocksToRelation(const std::string relation_name,
      const int memory_start_index, const int num_blocks) const;
  bool DeleteRelationBlocks(const std::string relation_name,
      const int start_index) const;

  // Debug APIs
  bool Tuples(const std::string relation_name,
      TupleList& tuples_as_string) const;
  void PrintTupleList(const std::string relation_name,
      const TupleList tuples) const;

 private:
  StorageAdapter();
  ~StorageAdapter();

  void reset();
  void appendTupleToRelation(Relation* relation, Tuple& tuple) const;
  void clearMainMemoryBlocks() const;

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
