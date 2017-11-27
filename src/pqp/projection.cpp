#include<iostream>
#include<string>
#include<vector>

#include "storage/storage_manager_headers.h"
#include "storage/storage_adapter.h"
#include "pqp/projection.h"

void SubroutineProjection(std::string TableName, std::vector<std::string> FieldNames)
{
	if (Storage()->MainMemorySize() > Storage()->RelationBlockSize(TableName))
	{
		std::vector<std::vector<std::string>> tuples = new std::vector<std::vector<std::string>>();
		std::vector<Block *> blocks = new std::vector<Block *>();	
		for (int i = 0; i < Storage()->RelationBlockSize(TableName); i++)
		{
			bool hasReadRelationBlocks = Storage()->ReadRelationBlocks(TableName, 0, 0, 1, blocks);
			std::vector<Tuple> t = blocks[0]->getTuples();
			int num_of_tuples = blocks[0]->getNumTuples();
			for (int i = 0; i < num_of_tuples; i++)
			{
				std::vector<std::string> tuples_row = new std::vector<std::string>;
				for (int j = 0; j < FieldNames.size(); j++)
				{
					tuples_row.push_back(t[i]->getField(FieldNames[j]));
				}
				tuples.push_back(tuples_row);
			}
		}
		
		for (auto tuple : tuples) {
			for (auto field : tuple) {
				DEBUG_MSG_SINGLE_LINE("|" << setw(18) << std::left << field);
			}
			DEBUG_MSG_SINGLE_LINE(" |\n");
		}
	}
}
