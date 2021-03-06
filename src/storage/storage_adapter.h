#ifndef SRC_STORAGE_ADAPTER_H
#define SRC_STORAGE_ADAPTER_H

#include <string>
#include <vector>

#include "storage/storage_manager_headers.h"

typedef std::vector<std::vector<std::string> > TupleList;

class StorageAdapter {
 public:
  // Singleton method
  static StorageAdapter *Get();

  bool Initialize();

  void ResetDiskStats();
  void PrintDiskStats();

  bool CreateRelation(const std::string& name,
                      const std::vector<std::string>& fields,
                      const std::vector<enum FIELD_TYPE>& field_types) const;
  bool DeleteRelation(const std::string& name) const;
  bool CreateEmptyTuple(const std::string& relation_name, Tuple& tuple) const;
  Tuple CreateTuple(const std::string& relation_name,
      const std::vector<std::string>& values, bool& created) const;
  bool CreateTupleAndAppend(const std::string& relation_name,
      const std::vector<std::string>& values) const;
  bool CreateTupleAndAppend(const std::string& relation_name,
      const std::vector<std::string>& field_names,
      const std::vector<std::string>& values) const;
  bool DeleteTuples(const std::string& relation_name,
      const int start_index) const;
  bool DeleteAllTuples(const std::string& relation_name) const;

  bool IsValidRelation(const std::string& table_name) const;
  bool IsValidColumnName(const std::string& table_name,
      const std::string& attribute_name) const;
  bool AttributeType(const std::string& table_name,
      const std::string& attribute_name, enum FIELD_TYPE& type);

  int MainMemorySize() const;
  void SetMainMemoryBlock(int memory_index, Block *block) const;
  int RelationBlockSize(const std::string relation_name) const;
  int RelationTupleSize(const std::string relation_name) const;
  bool ReadRelationBlocks(const std::string relation_name,
      const int relation_start_index, const int memory_start_index,
      const int num_blocks, std::vector<Block *>& blocks) const;
  bool InsertBlocksToRelation(const std::string relation_name,
      const int memory_start_index, const int relation_start_index,
      const int num_blocks) const;
  bool AppendBlocksToRelation(const std::string relation_name,
      const int memory_start_index, const int num_blocks) const;
  bool DeleteRelationBlocks(const std::string relation_name,
      const int start_index) const;

  bool CreateDummyRelation(const std::string name_prefix,
      std::vector<std::string> field_names, std::string& relation_name);
  bool CreateDummyRelation(const std::string name_prefix,
      std::vector<std::string>& field_names,
      std::vector<enum FIELD_TYPE>& field_types, std::string& relation_name);
  bool DeleteDummyRelation(std::string relation_name);
  void ClearBlock(int index) const;
  bool AppendTupleUsing(const std::string relation_name, Tuple& tuple,
      const int index) const;
  bool PushLastBlock(const std::string relation_name, const int index) const;
  bool RelationFieldNames(std::string relation_name,
      std::vector<std::string>& field_names);

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
