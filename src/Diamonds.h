#include "Blocks.h"
#include <unistd.h>
#include <array>
using namespace std;

// WE MOST LIKELY WON'T NEED A WRAPPER FOR SUPERBLOCK
//
// class Superblock
// {
// public:
// 	Superblock(); // Constructor

// 	array<bool, BLOCK_SIZE> get_bitmap(); // Get entire bitmap
// 	bool get_bit(int index);							// Get bit at index
// 	void set_bit(int index, bool value);	// Set bit at index

// private:
// 	superblock_t superblock_struct;
// };

class Dirblock
{
public:
	Dirblock(unsigned int magic); // Constructor
	unsigned int get_magic();			// gets magic number from dirblock_struct

	unsigned int get_num_entries();
	array<DirblockEntry, MAX_DIR_ENTRIES> get_dirblock_entries();
	DirblockEntry get_dirblock_entry(int index);
	void add_dirblock_entry(DirblockEntry entry);
	void remove_dirblock_entry(int index);

private:
	dirblock_t dirblock_s;
};

class DirblockEntry
{
public:
	DirblockEntry(string name, short block_num); // Constructor

	string get_name();
	short get_block_num();

private:
	struct // Struct copied from "Block.h"'s dirblock_t struct
	{
		char name[MAX_FNAME_SIZE + 1]; // file name (extra space for null)
		short block_num;							 // block number of file (0 - unused)
	} dirblock_entry_s;
};

class DataBlock
{
public:
	DataBlock(array<char, BLOCK_SIZE> data, short id); // Constructor

	short get_id();
	array<char, BLOCK_SIZE> get_data(); // gets data from block
	void set_data(array<char, BLOCK_SIZE> data);

private:
	short id;
	datablock_t data_block_s;
};

class Inode
{
public:
	Inode(unsigned int magic); // Constructor
	unsigned int get_magic();	 // gets magic number from inode
	unsigned int get_size();	 // gets size from inode

	array<short, MAX_DATA_BLOCKS> get_blocks(); // Get all blocks
	short get_block(int index);									// Get block at index
	short add_block(DataBlock block);
	void remove_block(int index);

private:
	inode_t inode_s;
};