// See `WrappedFileSys.h` for more details regarding the purpose of this file.

#include "WrappedFileSys.h"

using namespace std;

namespace WrappedFileSys
{
	BasicFileSys *bfs;
}

// =============================================================================
// ------------------------------- BLOCK ---------------------------------------
// =============================================================================
template <typename T>
Block<T>::Block(short id) // Retrieve an existing block from disk
{
	this->id = id;

	// Read block from disk
	WrappedFileSys::bfs->read_block(this->id, (void *)&this->raw);
}
template <typename T>
Block<T>::Block() // Create a new block without data
{
	// cout << "vvvvvvvvvv" << endl
	// 		 << "| "
	// 		 << "CREATING NEW BLOCK" << endl
	// 		 << "^^^^^^^^^^"
	// 		 << endl;

	// Find a free block
	short id = WrappedFileSys::bfs->get_free_block();

	// Check return value of get_free_block. A return value of 0 means the disk
	// is full.
	if (id == 0)
	{
		throw WrappedFileSys::DiskFullException();
	}

	// Set block id
	this->id = id;

	// It's the subclasses' responsibility to clear/intialize the block and write
	// it back to memory.
}

template <typename T>
T Block<T>::get_raw()
{
	return this->raw;
}

template <typename T>
short Block<T>::get_id()
{
	return this->id;
}

template <typename T>
void Block<T>::write_and_set_raw_block(T tempRaw)
{
	// Write block to disk
	WrappedFileSys::bfs->write_block(this->id, (void *)&tempRaw);

	// Update the class data member AFTER updating the disk
	this->raw = tempRaw;
}

// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
DataBlock::DataBlock(short id) : Block<datablock_t>(id)
{
	// Deserialize data
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		this->data[i] = this->raw.data[i];
	}
}

DataBlock::DataBlock(array<char, BLOCK_SIZE> data) : Block<datablock_t>() // Create a new block from data
{
	this->set_data(data);
}

array<char, BLOCK_SIZE> DataBlock::get_data()
{
	return this->data;
}
void DataBlock::set_data(array<char, BLOCK_SIZE> data) // vector length can't be longer than BLOCK_SIZE
{
	// Serialize data to temporary struct
	struct datablock_t tempRaw;
	for (int i = 0; i < data.size(); i++)
	{
		tempRaw.data[i] = data[i];
	}

	// Write to disk and set class's raw
	this->write_and_set_raw_block(tempRaw);

	// Update the class data members AFTER updating the disk
	this->data = data;
}

// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================
template <typename T>
Inode<T>::Inode(short id) : Block<T>(id) // Retrieve an existing inode from disk
{
	// Deserialize data
	this->magic = this->raw.magic;
}

template <typename T>
Inode<T>::Inode() : Block<T>() // Create a new inode
{
	// Literally doesn't do anything besides call the parent constructor (`Block`)
	// to allocate a new block.
}

template <typename T>
unsigned int Inode<T>::get_magic()
{
	return this->magic;
}

template <typename T>
bool Inode<T>::is_dir()
{
	return this->magic == DIR_MAGIC_NUM;
}

template <typename T>
bool Inode<T>::is_file()
{
	return this->magic == INODE_MAGIC_NUM;
}

// =============================================================================
// ------------------------------- FILE INODE ----------------------------------
// =============================================================================
FileInode::FileInode(short id) : Inode(id) // Retrieve an existing file inode from disk
{
	// Validate that this is in fact a File Inode
	if (!this->is_file())
	{
		cerr << "ERROR (FileInode::FileInode): Inode is not a file inode." << endl;
		throw WrappedFileSys::FileSystemException();
	}

	// Deserialize data
	this->magic = this->raw.magic;
	this->size = this->raw.size;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		short block_id = this->raw.blocks[i];
		if (block_id != UNUSED_ID)
		{
			this->blocks.push_back(DataBlock(block_id));
		}
	}
}

FileInode::FileInode() : Inode<inode_t>() // Create a new file inode
{
	// Set up data in a temporary struct
	struct inode_t tempRaw;
	tempRaw.magic = INODE_MAGIC_NUM;
	tempRaw.size = 0;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		tempRaw.blocks[i] = UNUSED_ID;
	}

	// Write to disk and set class's raw
	this->write_and_set_raw_block(tempRaw);

	// Update the class data members AFTER updating the disk
	this->magic = tempRaw.magic;
	this->size = tempRaw.size;
	// this->blocks should already be empty
}

unsigned int FileInode::get_size()
{
	return this->size;
}

vector<DataBlock> FileInode::get_blocks()
{
	return this->blocks;
}

void FileInode::add_block(DataBlock block, unsigned int size)
{
	// The block has already been written to the disk, so the responsiblity of
	// this function is to provide a pointer to that block

	// Check that the size is not greater than what a single block can hold
	if (size > BLOCK_SIZE)
	{
		cerr << "ERROR (FileInode::add_block): DataBlock size is greater than BLOCK_SIZE." << endl;
	}

	// Create a temp copy of the raw block
	inode_t tempRaw = this->get_raw();

	// Add the block id (index) in an unused spot
	bool written = false;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		if (tempRaw.blocks[i] != UNUSED_ID)
		{
			continue;
		}

		// We've found an empty spot
		tempRaw.blocks[i] = block.get_id();
		tempRaw.size += size;
		written = true;
		break;
	}
	if (!written)
	{
		// No empty spot was found
		throw WrappedFileSys::FileFullException();
	}

	// Write the temp block to the disk
	this->write_and_set_raw_block(tempRaw);

	// Update the class data member AFTER updating the disk
	this->blocks.push_back(block);
	this->size = this->raw.size;
}

void FileInode::remove_block(DataBlock block)
{
	// When removing a block, we must shift all the blocks to the left. Otherwise,
	// file data may be returned in the wrong order.

	short block_id = block.get_id();

	// Create a temp copy of the raw block
	inode_t tempRaw = this->get_raw();

	// Find the index of the block in the blocks array
	int index = -1;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
	{
		if (tempRaw.blocks[i] == block_id)
		{
			index = i;
			break;
		}
	}

	// Check if the block was found (it should be)
	if (index == -1)
	{
		cerr << "ERROR (FileInode::remove_block): Block #" << block_id << " was not found in the File Inode." << endl;
		throw WrappedFileSys::FileSystemException();
	}

	// Shift the block ids over (overriding the block we want to remove)
	for (int i = index; i < MAX_DATA_BLOCKS - 1; i++)
	{
		tempRaw.blocks[i] = tempRaw.blocks[i + 1];
	}
	tempRaw.blocks[MAX_DATA_BLOCKS - 1] = UNUSED_ID;

	// Determine the size of this datablock by figuring out if this the last block
	// in the file
	DataBlock lastBlock = this->blocks.at(this->blocks.size() - 1);
	unsigned int size = BLOCK_SIZE;
	if (lastBlock.get_id() == block.get_id())
	{
		// This is the last block, it might contain fragmentation
		size = this->internal_frag_size();
	}

	// Update the size
	tempRaw.size -= size;
	cout << "sub size: " << size << ". now " << tempRaw.size << endl;

	// Update the class data member BEFORE updating the disk
	this->size = tempRaw.size;
	this->blocks.erase(remove_if(this->blocks.begin(), this->blocks.end(),
															 [=](DataBlock block) -> bool
															 { return block.get_id() == block_id; }),
										 this->blocks.end());

	// Write the temp block to disk
	this->write_and_set_raw_block(tempRaw);
}

bool FileInode::has_free_block()
{
	return MAX_DATA_BLOCKS - this->blocks.size() > 0;
}

unsigned int FileInode::internal_frag_size()
{
	// int num_blocks = this->blocks.size();
	// cout << "num_blocks: " << num_blocks << endl;
	// unsigned int capacity = num_blocks * BLOCK_SIZE;
	// cout << "cap: " << capacity << endl;
	// cout << "this size: " << this->size << endl;
	// return capacity - this->size;

	return this->size % BLOCK_SIZE;
}

// =============================================================================
// ------------------------------- DIRECTORY INODE -----------------------------
// =============================================================================
DirInode::DirInode(short id) : Inode(id) // Retrieve an existing directory inode from disk
{
	// Validate that this is in fact a Dir Inode
	if (!this->is_dir())
	{
		cerr << "ERROR (DirInode::DirInode): Inode is not a directory inode." << endl;
		throw WrappedFileSys::FileSystemException();
	}

	// Deserialize data
	this->magic = this->raw.magic;
	this->num_entries = this->raw.num_entries;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		char *name = this->raw.dir_entries[i].name;
		short block_id = this->raw.dir_entries[i].block_num;

		// Skip block entires that unused
		if (block_id == UNUSED_ID)
		{
			continue;
		}

		// Determine type of Inode (file or directory)
		Inode inode = Inode(block_id);
		if (inode.is_file())
		{
			FileInode file_inode(block_id);
			DirEntry<FileInode> entry(name, file_inode);
			this->file_entries.push_back(entry);
		}
		else if (inode.is_dir())
		{
			DirInode dir_inode(block_id);
			DirEntry<DirInode> entry(name, dir_inode);
			this->dir_entries.push_back(entry);
		}
		else
		{
			cerr << "ERROR (DirInode::DirInode): Inode is not a file or directory inode." << endl;

			// Assume it's a file inode
			FileInode file_inode(block_id);
			DirEntry<FileInode> entry(name, file_inode);
			this->file_entries.push_back(entry);
		}
	}
}

DirInode::DirInode() : Inode() // Create a new directory inode
{
	// Set up data in a temporary struct
	struct dirblock_t tempRaw;
	tempRaw.magic = DIR_MAGIC_NUM;
	tempRaw.num_entries = 0;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		tempRaw.dir_entries[i].block_num = UNUSED_ID;
	}

	// Write to disk and set class's raw
	this->write_and_set_raw_block(tempRaw);

	// Update the class data members AFTER updating the disk
	this->magic = tempRaw.magic;
	this->num_entries = tempRaw.num_entries;
}

unsigned int DirInode::get_num_entries()
{
	return this->num_entries;
}

vector<DirEntry<FileInode>> DirInode::get_file_inode_entires()
{
	return this->file_entries;
}
vector<DirEntry<DirInode>> DirInode::get_dir_inode_entries()
{
	return this->dir_entries;
}

void DirInode::add_entry(DirEntry<FileInode> entry)
{
	this->add_entry_base(entry, this->file_entries);
}

void DirInode::add_entry(DirEntry<DirInode> entry)
{
	this->add_entry_base(entry, this->dir_entries);
}

template <typename T>
void DirInode::add_entry_base(DirEntry<T> entry, vector<DirEntry<T>> &vec)
{
	// Create a temp copy of the raw block
	dirblock_t tempRaw = this->get_raw();

	// Add the inode (block id) in an unused spot
	bool written = false;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (tempRaw.dir_entries[i].block_num != UNUSED_ID)
		{
			continue;
		}

		// We've found an empty spot
		// Copy block num
		tempRaw.dir_entries[i].block_num = entry.get_inode().get_id();

		// Copy name
		int name_size = min(int(entry.get_name().size()), MAX_FNAME_SIZE);
		for (int j = 0; j < name_size; j++)
		{
			tempRaw.dir_entries[i].name[j] = entry.get_name()[j];
		}
		tempRaw.dir_entries[i].name[name_size] = '\0';

		written = true;
		break;
	}
	if (!written)
	{
		// No empty spot was found
		throw WrappedFileSys::DirFullException();
	}

	// Write the temp block to the disk
	this->write_and_set_raw_block(tempRaw);

	// Update the class data member AFTER updating the disk
	this->num_entries = tempRaw.num_entries;
}

void DirInode::remove_entry(DirEntry<FileInode> entry)
{
	this->remove_entry_base(entry, this->file_entries);
}

void DirInode::remove_entry(DirEntry<DirInode> entry)
{
	this->remove_entry_base(entry, this->dir_entries);
}

template <typename T>
void DirInode::remove_entry_base(DirEntry<T> entry, vector<DirEntry<T>> &vec)
{
	short block_id = entry.get_inode().get_id();

	// Create a temp copy of the raw block
	dirblock_t tempRaw = this->get_raw();

	// Find the index of the block in the blocks array
	int index = -1;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (tempRaw.dir_entries[i].block_num == block_id)
		{
			index = i;
			break;
		}
	}

	// Check if the block was found (it should be)
	if (index == -1)
	{
		cerr << "ERROR (DirInode::remove_entry): DirEntry for Block #" << block_id << " was not found in the Dir Inode #" << this->get_id() << "." << endl;
		throw WrappedFileSys::FileSystemException();
	}

	// Shift the block ids over (overriding the block we want to remove)
	for (int i = index; i < MAX_DIR_ENTRIES - 1; i++)
	{
		tempRaw.dir_entries[i] = tempRaw.dir_entries[i + 1];
	}
	tempRaw.dir_entries[MAX_DIR_ENTRIES - 1].block_num = UNUSED_ID;

	// Update the size
	tempRaw.num_entries--;

	// Remove from data members BEFORE removing from disk
	this->num_entries = tempRaw.num_entries;
	vec.erase(remove_if(vec.begin(), vec.end(),
											[=](DirEntry<T> curr_entry) -> bool
											{ return curr_entry.get_inode().get_id() == block_id; }),
						vec.end());

	// Write the temp block to disk
	this->write_and_set_raw_block(tempRaw);
}

bool DirInode::has_free_entry()
{
	// cout << this->get_id() << endl;
	// cout << this->get_num_entries() << endl;
	// cout << to_string((MAX_DIR_ENTRIES - this->get_num_entries()) > 0) << endl;
	return (MAX_DIR_ENTRIES - this->get_num_entries()) > 0;
}

// =============================================================================
// ------------------------------- DIR ENTRY -----------------------------------
// =============================================================================
template <typename T>
DirEntry<T>::DirEntry(string name, T inode)
{
	this->name = name;
	this->inode_id = inode.get_id();

	// Check if the given name is too long
	if (name.size() > MAX_FNAME_SIZE)
	{
		throw WrappedFileSys::FileNameTooLongException();
	}
}

template <typename T>
string DirEntry<T>::get_name()
{
	return this->name;
}

template <typename T>
T DirEntry<T>::get_inode()
{
	return T(this->inode_id);
}