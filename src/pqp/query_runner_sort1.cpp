#include<iostream>
#include<string>
#include<vector>
#include<climits>

#include "storage/storage_manager_headers.h"
#include "storage/storage_adapter.h"
#include "lqp/statement_create_table.h"
#include "pqp/query_runner_sort.h"

QueryRunnerSort::QueryRunnerSort(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerSort::~QueryRunnerSort() {
}

bool QueryRunnerSort::Run(QueryResultCallback callback,
     SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SELECTION;
    return false;
  }

  SetCallback(callback);

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerSelection::ResultCallback,
          this, std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_SELECTION;
    }

    error_code = error_code_;
    return false;
  }

  return true;
}

bool QueryRunnerSort::ResultCallback(std::vector<Tuple>& tuples,
    bool headers) {
  if (headers) {
    return Callback()(tuples, headers);
  }

  WhereClauseHelperSelect *where_helper;
  if (!Node()->WhereHelper(where_helper) || where_helper == nullptr) {
    DEBUG_MSG("");
    return false;
  }

  int num_of_sublist = RelationBlockSize(table_name) / (Sorage()->MainMemorySize() - 2);
  int[] blocks_in_sublist = new int [num_of_sublist];

  for (int i = 0; i < num_of_sublist - 1; i++)
  {
    blocks_in_sublist[i] = Storage()->MainMemorySize() - 2;
  }
  blocks_in_sublist[num_of_sublist - 1] = RelationBlockSize(table_name) - (num_of_sublist - 1) * (Storage()->MainMemorySize() - 2);

  Relation* relation = schema_manager_->getRelation(table_name);
  std::vector<Block *> blocks;
  std::vector<std::string> values = new std::vector<std::string>();

  //first pass
  std::string temp_rel = table_name + "_temp1";
  CreateRelation(temp_rel, relation->getSchema.field_names, relation->getSchema.field_types);

  total_tuples = relation->getNumTuples();
  
  for (int i = 0; i < num_of_sublist; i++)
  {
    int min_oall, min_iter, sort_indb, sort_indt = 0;
    int loop = 0;

    while (loop < total_tuples)
    {
      min_oall = INT_MAX;
      int count = 0;
      std::vector<int> similar_indb;
      std::vector<int> similar_indt; 

      for (int j = 0; j < num_in_sublist; j++)
      {
        std::vector<Tuple> tuples = blocks[j]->getTuples();

        for (int k = 0; k < blocks[j]->getNumTuples(); k++)
        {
          union Field sortfield = tuples[k]->getField(field_name);
          if (tuples[k].getSchema().getFieldType(field_name) == INT)
          {
            int flag = false;

            if (sortfield.integer == min_oall)
            {
              count++;
              similar_indt.push_back(k);
              similar_indb.push_back(j);
            }
          
            if (sortfield.integer > min_oall)
            {
              if (sortfield.integer < min_iter)
              {
                min_iter = sortfield.integer;
                sort_indb = j;
                sort_indt = k;
              }
            }
          }
        }
        if (count > 1)
        {
          for (int j = 0; j < similar_indb; j++)
          { 
            for (int k = 0; k < similar_indt; k++)
            {
              std::vector<Tuple> tuples = blocks[sort_indb]->getTuples();
              for (int j = 0; j < tuples[sort_indt].getNumOfFields(); j++)
              {
                if (tuples[k].getSchema().getFieldType(j) == INT)
                {
                  values.push_back(std::to_string(tuples[sort_indt].getField(j).integer));
                }
                else
                {
                  values.push_back(*(tuples[sort_indt].getField(j).str));
                }
              }
              Storage()->CreateTupleAndAppend(temp_rel, values);
              count = 0;
            }
          }
        }
        else
        {
          std::vector<Tuple> tuples = blocks[sort_indb]->getTuples();
          for (int j = 0; j < tuples[sort_indt].getNumOfFields(); j++)
          {
            if (tuples[sort_indt].getSchema().getFieldType(j) == INT)
            {
              values.push_back(std::to_string(tuples[sort_indt].getField(j).integer));
            }
            else
            {
              values.push_back(*(tuples[sort_indt].getField(j).str));
            }
          }
          Storage()->CreateTupleAndAppend(temp_rel, values);
        }
      }
    }
  }

  //second pass
  std::string rel_temp = table_name + "_temp2";
  std::vector<Block *>block;
  std::vector<Block *>allblocks;

  for (int i = 0; i < num_of_sublist; i++)
  {
    bool hasReadRelationBlocks = Storage()->ReadRelationBlocks(temp_rel, 0, i, i + 1, block);
    allblocks.push_back(block[0]);
  }

  int nblock[num_of_sublist];
  int ntuple[num_of_sublist];
  int sublist_ind;

  // initiallize ntuple and nblock
  
  int loop = 0;
  int min = INT_MAX;
  while (loop < total_tuples)
  {
    for (int i = 0; i < num_of_sublist; i++)
    {
      if (ntuple[i] == allblocks[i]->getNumTuples() - 1)
      {
        if (nblock[i] < blocks_in_sublist[i])
        {
          bool hasReadRelationBlocks = Storage()->ReadRelationBlocks(temp_rel, 0, nblock[i], nblock[i] + 1, block);
          ntuple[i]++;
        }
      }
      
      if (allblocks[nblock[i]]->getTuple(ntuple[i]).getField(field_name).integer < min)
      {
        min = allblocks[nblock[i]]->getTuple(ntuple[i]).getField(field_name).integer;
        sublist_ind = i;
      }
    }
    Storage()->CreateTupleAndAppend(rel_temp, values);
  }

  if (!Callback()(tuples, headers)) {
    DEBUG_MSG("");
    return false;
  }

  return true;
}
