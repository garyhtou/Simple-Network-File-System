#include "Blocks.h"
#include <unistd.h>
#include <array>
using namespace std;

class Superblock
{
public:
	Superblock(); // Constructor

	array<bool, BLOCK_SIZE> get_bitmap(); // Get entire bitmap
	bool get_bit(int index);							// Get bit at index
	void set_bit(int index, bool value);	// Set bit at index

private:
	superblock_t superblock_struct;
};

class Dirblock
{
public:
	Dirblock(unsigned int magic); // Constructor
	unsigned int get_magic();			// gets magic number from dirblock_struct

	unsigned int get_num_entries();
	array<DirblockEntry, MAX_DIR_ENTRIES> get_dirblock_entries();
	DirblockEntry get_dirblock_entry(int index);

private:
	dirblock_t dirblcok_struct;

	void set_num_entries(unsigned int value);
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
	} dir_entry_struct;							 // list of directory entries
};

class DataBlock
{
public:
	DataBlock(array<char, BLOCK_SIZE>); // Constructor

	array<char, BLOCK_SIZE> get_data(); // gets data from block
	void set_data(array<char, BLOCK_SIZE>);

private:
	datablock_t data_block_struct;
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
	inode_t inode_struct;
	void set_size(unsigned int size); // sets size of inode array
};