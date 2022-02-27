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

class Dirblock
{
public:
	Dirblock();								// Constructor
	unsigned int get_magic(); // gets magic number from dirblock_struct

	unsigned int get_num_entries();
	array<DirblockEntry, MAX_DIR_ENTRIES> get_dirblock_entries();
	DirblockEntry get_dirblock_entry(int index);
	void add_dirblock_entry(DirblockEntry entry);
	void remove_dirblock_entry(int index);

private:
	dirblock_t dirblock_s;
};

class DataBlock
{
public:
	DataBlock(array<char, BLOCK_SIZE> data, short id, unsigned int size_bytes); // Constructor

	short get_id();
	array<char, BLOCK_SIZE> get_data(); // gets data from block
	void set_data(array<char, BLOCK_SIZE> data, unsigned int size_bytes);
	unsigned int get_size_bytes();

private:
	short id;
	unsigned int size_bytes;
	datablock_t data_block_s;
};

class Inode
{
public:
	Inode(short id, short block_id);					// Constructor
	unsigned int get_magic(); // gets magic number from inode
	unsigned int get_size();	// gets size from inode

	array<short, MAX_DATA_BLOCKS> get_blocks(); // Get all blocks
	short get_block(int index);									// Get block at index
	short add_block(DataBlock block);
	void remove_block(int index);
	short get_id();
	short get_block_id();
	inode_t get_raw_struct();

private:
	short id;
	short block_id;
	inode_t inode_s;
};
