#ifndef SRC_PQP_PROJECTION_
#define SRC_PQP_PROJECTION_

#include<iostream>
#include<string>
#include<vector>

#include "storage/storage_manager_headers.h"
#include "storage/storage_adapter.h"

class StorageAdapter;

class projection{
 public:
  void SubroutineProjection(std::string TableName, std::vector<std::string> FieldNames);
}
