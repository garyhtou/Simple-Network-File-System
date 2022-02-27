// This Diamonds file contains class wrappers for the extremely terrible and
// basic structs defined in "Blocks.h". (Diamonds are the superior blocks)

#include "Diamonds.h"

#include "Blocks.h"
#include <unistd.h>
#include <string>
#include <array>
#include <cstring>
#include <iostream>
using namespace std;

// WE MOST LIKELY WON'T NEED A WRAPPER FOR SUPERBLOCK
//
// Superblock::Superblock() // Constructor
// {
// }

// array<bool, BLOCK_SIZE> Superblock::get_bitmap() // Get entire bitmap
// {
// }
// bool Superblock::get_bit(int index) // Get bit at index
// {
// }
// void Superblock::set_bit(int index, bool value) // Set bit at index
// {
// }

const short UNUSED = 0;

Dirblock::Dirblock() // Constructor
{
	this->dirblock_s.magic = DIR_MAGIC_NUM;
	this->dirblock_s.num_entries = 0;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		this->dirblock_s.dir_entries[i].name[i] = '\0'; // Set to empty string
		this->dirblock_s.dir_entries[i].block_num = UNUSED;
	}

	// TODO: Do we need to have the current and parent directory as entires?
}

unsigned int Dirblock::get_magic()
{ // gets magic number from dirblock_s
	return this->dirblock_s.magic;
}

unsigned int Dirblock::get_num_entries()
{
	return this->dirblock_s.num_entries;
}

array<DirblockEntry, MAX_DIR_ENTRIES> Dirblock::get_dirblock_entries()
{
	array<DirblockEntry, MAX_DIR_ENTRIES> entries;

	// Loop through all entries in dirblock_s and create a DirblockEntry for each
	for (int i = 0; i < this->get_num_entries(); i++)
	{
		DirblockEntry entry = DirblockEntry(
				this->dirblock_s.dir_entries[i].name,
				this->dirblock_s.dir_entries[i].block_num);

		entries[i] = entry;
	}

	return entries;
}

DirblockEntry Dirblock::get_dirblock_entry(int index)
{
	// Check if index is valid
	if (index < 0 || index >= this->get_num_entries())
	{
		cerr << "ERROR (Dirblock::get_dirblock_entry): Invalid index. index=" << index << endl;
		return;
	}

	DirblockEntry entry = DirblockEntry(
			this->dirblock_s.dir_entries[index].name,
			this->dirblock_s.dir_entries[index].block_num);

	return entry;
}

void Dirblock::add_dirblock_entry(DirblockEntry entry)
{
	unsigned int num_entries = this->get_num_entries();

	// Check if we have reached max entries
	if (num_entries >= MAX_DIR_ENTRIES)
	{
		cerr << "ERROR (Dirblock::add_dirblock_entry): Max entries reached. Max=" << MAX_DIR_ENTRIES << endl;
		return;
	}

	// Add the name and block_num to the dir_entires array
	for (int i = 0; i < MAX_FNAME_SIZE; i++)
	{
		this->dirblock_s.dir_entries[num_entries].name[i] = entry.get_name()[i];
	}
	this->dirblock_s.dir_entries[num_entries].block_num = entry.get_block_num();

	// Increment num_entries AFTER copying in values
	this->dirblock_s.num_entries++;
}

void Dirblock::remove_dirblock_entry(int index)
{
	unsigned int num_entries = this->get_num_entries();

	// Check if entry exists (index is valid)
	if (index < 0 || index >= num_entries)
	{
		cerr << "ERROR (Dirblock::remove_dirblock_entry): Invalid index. index=" << index << endl;
		return;
	}

	// Increment num_entries BEFORE removing entry
	this->dirblock_s.num_entries--;

	// Shift entries to the left (without going out of bounds)
	for (int i = index; i < min(int(num_entries), MAX_DIR_ENTRIES - 1); i++)
	{
		for (int i = 0; i < MAX_FNAME_SIZE; i++)
		{
			this->dirblock_s.dir_entries[num_entries].name[i] = this->dirblock_s.dir_entries[num_entries + 1].name[i];
		}
		this->dirblock_s.dir_entries[i].block_num = this->dirblock_s.dir_entries[i + 1].block_num;
	}
}

DirblockEntry::DirblockEntry(string name, short block_num) // Constructor
{
	// Check if name is too long
	if (name.length() > MAX_FNAME_SIZE)
	{
		cerr << "ERROR (DirblockEntry::DirblockEntry): Name too long. Name=" << name << endl;
		return;
	}

	// Use a loop to copy name into dirblock_entry_s.name
	for (int i = 0; i < MAX_FNAME_SIZE; i++)
	{
		this->dirblock_entry_s.name[i] = name.at(i);
	}
	this->dirblock_entry_s.block_num = block_num;
}

string DirblockEntry::get_name()
{
	return this->dirblock_entry_s.name;
}

short DirblockEntry::get_block_num()
{
	return this->dirblock_entry_s.block_num;
}

DataBlock::DataBlock(array<char, BLOCK_SIZE> data, short id, unsigned int size_bytes) // Constructor
{
	this->id = id;
	this->set_data(data, size_bytes);
}

short DataBlock::get_id()
{
	return this->id;
}

array<char, BLOCK_SIZE> DataBlock::get_data() // gets data from block
{
	array<char, BLOCK_SIZE> data;
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		data[i] = this->data_block_s.data[i];
	}

	return data;
}

void DataBlock::set_data(array<char, BLOCK_SIZE> data, unsigned int size_bytes)
{
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		this->data_block_s.data[i] = data.at(i);
	}

	// Copy size_bytes AFTER copying data
	this->size_bytes = size_bytes;
}

unsigned int DataBlock::get_size_bytes()
{
	return this->size_bytes;
}

Inode::Inode(short id, short block_id) // Constructor
{
	this->id = id;
	this->block_id = block_id;
	this->inode_s.magic = INODE_MAGIC_NUM;
	this->inode_s.size = 0;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		this->inode_s.blocks[i] = UNUSED;
	}
}

unsigned int Inode::get_magic() // gets magic number from inode
{
	return this->inode_s.magic;
}

unsigned int Inode::get_size() // gets size from inode
{
	return this->inode_s.size;
}

array<short, MAX_DATA_BLOCKS> Inode::get_blocks() // Get all blocks
{
	array<short, MAX_DATA_BLOCKS> blocks;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		blocks[i] = this->inode_s.blocks[i];
	}
	return blocks;
}

short Inode::get_block(int index) // Get block at index
{
	// Check index
	if (index < 0 || index >= MAX_DATA_BLOCKS)
	{
		cerr << "ERROR (Inode::get_block): Invalid index. index=" << index << endl;
		return;
	}
	return this->inode_s.blocks[index];
}

short Inode::add_block(DataBlock block)
{
	// Check max size
	if (this->get_size() >= MAX_DATA_BLOCKS)
	{
		cerr << "ERROR (Inode::add_block): Max size reached. Max=" << MAX_DATA_BLOCKS << endl;
		return;
	}

	// Add the block's id (index)
	this->inode_s.blocks[this->get_size()] = block.get_id();

	// Add the block's bytes to size AFTER adding block
	this->inode_s.size += block.get_size_bytes();
}

void Inode::remove_block(int index)
{
	// Check index
	if (index < 0 || index >= this->get_size())
	{
		cerr << "ERROR (Inode::remove_block): Invalid index. index=" << index << endl;
		return;
	}

	// Decrement size BEFORE removing block
	this->inode_s.size--;

	// Shift blocks to the left (without going out of bounds)
	for (int i = index; i < min(int(this->get_size()), MAX_DATA_BLOCKS); i++)
	{
		this->inode_s.blocks[i] = this->inode_s.blocks[i + 1];
	}
}

short Inode::get_id()
{
	return this->id;
}

short Inode::get_block_id()
{
	return this->block_id;
}

inode_t Inode::get_raw_struct()
{
	return this->inode_s;
}