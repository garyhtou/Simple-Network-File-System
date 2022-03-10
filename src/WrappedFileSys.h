// and functions provided by `blocks.h` and `BasicFileSys`.
//
// The goal is to provide an object oriented interface to the underlying
// C-like architecture. Wrapped File System will also handle error checking
// using custom exceptions.

// =============================================================================
// ----------------------------- CLASS DIAGRAM ---------------------------------
// =============================================================================
//
//    Classes that are labeled         +--------------+
//    as ABSTRACT shouldn't be         |              |
//    used directly. Although,         |   Block<T>   |
//    you can use Inode<T> to          |   ABSTRACT   |
//    check the type of a Inode        |              |
//    (File or Dir).                   +------^-------+
//                                            |
//                            +---------------+-------------+
//                            |                             |
//                            |                      +------+-------+
//                            |                      |              |
//                            |                      |   Inode<T>   |
//                            |                      |   ABSTRACT   |
//                            |                      |              |
//                            |                      +------^-------+
//                            |                             |
//                            |                    +--------+---------+
//                            |                    |                  |
//                    +-------+------+     +-------+------+    +------+-------+
//                    |              |     |              |    |              |
//  TYPES OF BLOCKS:  |  DataBlock   |     |   FileInode  |    |   DirInode   |
//                    |              |     |              |    |              |
//                    +--------------+     +--------------+    +--------------+
//
//
//     DirEntry is a class generic          +-----------------+
//     (template) which takes in            |                 |
//     either a FileInode or                |   DirEntry<T>   |
//     DirInode.                            |                 |
//                                          +-----------------+
//     These entries can be added
//     to a DirInode using                  EXAMPLE:
//     DirInode::add_entry();               DirEntry<FileInode>
//                                          DirEntry<DirInode>

#ifndef WRAPPEDFILESYS_H
#define WRAPPEDFILESYS_H

#include "Blocks.h"
#include "BasicFileSys.h"

#include <unistd.h>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

const short UNUSED_ID = 0;
const short HOME_DIR_ID = 1;

namespace WrappedFileSys
{
	// This BasicFileSys instance is shared by all the wrapper classes. It must be
	// initialized before any wrapper classes are created/used.
	extern BasicFileSys *bfs;

	// ===========================================================================
	// -------------------------- CUSTOM EXCEPTIONS ------------------------------
	// ===========================================================================
	class FileSystemException : public exception
	{
	public:
		// Applies to: cd, rmdir
		const char *what() const throw()
		{
			return "A File System error has occured";
		}
	};
	class NotADirException : public FileSystemException
	{
	public:
		// Applies to: cd, rmdir
		const char *what() const throw()
		{
			return "500 File is not a directory";
		}
	};
	class NotAFileException : public FileSystemException
	{
	public:
		// Applies to: cat, head, append, rm
		const char *what() const throw()
		{
			return "501 File is a directory";
		}
	};
	class FileExistsException : public FileSystemException
	{
	public:
		// Applies to: create, mkdir
		const char *what() const throw()
		{
			return "502 File exists";
		}
	};
	class FileNotFoundException : public FileSystemException
	{
	public:
		// Applies to: cd, rmdir, cat, head, append, rm, stat
		const char *what() const throw()
		{
			return "503 File does not exist";
		}
	};
	class FileNameTooLongException : public FileSystemException
	{
	public:
		// Applies to: create, mkdir
		const char *what() const throw()
		{
			return "504 File name is too long";
		}
	};
	class DiskFullException : public FileSystemException
	{
	public:
		// Applies to: create, mkdir, append
		const char *what() const throw()
		{
			return "505 Disk is full";
		}
	};
	class DirFullException : public FileSystemException
	{
	public:
		// Applies to: create, mkdir
		const char *what() const throw()
		{
			return "506 Directory is full";
		}
	};
	class DirNotEmptyException : public FileSystemException
	{
	public:
		// Applies to: rmdir
		const char *what() const throw()
		{
			return "507 Directory is not empty";
		}
	};
	class FileFullException : public FileSystemException
	{
	public:
		// Applies to: append
		const char *what() const throw()
		{
			return "508 Append exceeds maximum file size";
		}
	};
};

// =============================================================================
// No class wrapper is needed for the superblock as none of our code will need
// to interact directly with it.

// =============================================================================
// -------------------------------- BLOCK --------------------------------------
// =============================================================================
// `Block` is an fairly abstract class meant to be used by it's sub classes for
// reading and writing "raw" structs from and to the disk. It is a class
// template to allow for different types of "raw" struct (e.g. `inode_t`).
template <typename T>
class Block
{
public:
	Block(short id); // Retrieve an existing block from disk
	Block();				 // Create a new block without data

	T get_raw();
	short get_id();

	// IMPORTANT! You must be very careful to not use (e.g. calling member
	// functions) this object after destroying it. Otherwise, you will edit data
	// which no longer belongs to this block.
	// This will only destory the block/inode that it is called on. It won't
	// destroy the blocks which belong to the inode. You must do that!
	void destroy()
	{
		// Delete block from disk
		WrappedFileSys::bfs->reclaim_block(this->get_id());
	}

protected:
	short id; // This is the index of the block on disk

	T raw;
	void write_and_set_raw_block(T block);
};

// =============================================================================
// ----------------------------- DATA BLOCK ------------------------------------
// =============================================================================
class DataBlock : public Block<datablock_t>
{
public:
	DataBlock(short id);										 // Retrieve an existing data block from disk
	DataBlock(array<char, BLOCK_SIZE> data); // Create a new block from data
	// There is no constructor to create an empty DataBlock because datablocks
	// should NEVER be empty.

	array<char, BLOCK_SIZE> get_data();
	void set_data(array<char, BLOCK_SIZE> data);

	using Block<datablock_t>::get_raw;
	using Block<datablock_t>::get_id;
	using Block<datablock_t>::destroy;

protected:
	// Block uses an array instead of vector because there is no way to tell the
	// data size from the datablock_t struct (some bits of the data block may be
	// unused).
	array<char, BLOCK_SIZE> data;
};

// =============================================================================
// ------------------------------- INODE ---------------------------------------
// =============================================================================
// `Inode` is a class template to allow for different types inode "raw" structs.
// For example, `inode_t` and `dirblock_t` which represent a file inode and a
// directory inode respectively.
// `Inode` shouldn't really ever be used directly besides to check for the type
// for a given inode using `is_dir()`.
template <typename T>
class Inode : protected Block<T>
{
public:
	Inode(short id); // Retrieve an existing inode from disk
	Inode();				 // Should never be used directly

	unsigned int get_magic();

	bool is_dir();
	bool is_file();

	using Block<T>::get_raw;
	using Block<T>::get_id;
	using Block<T>::destroy;

protected:
	unsigned int magic;
};

// =============================================================================
// ----------------------------- FILE INODE ------------------------------------
// =============================================================================
class FileInode : public Inode<inode_t>
{
public:
	FileInode(short id); // Retrieve an existing file inode from disk
	FileInode();				 // Create a new file inode

	unsigned int get_size();

	vector<DataBlock> get_blocks();
	void add_block(DataBlock block, unsigned int size);
	void remove_block(DataBlock block);

	bool has_free_block();
	unsigned int internal_frag_size();

protected:
	unsigned int size;
	vector<DataBlock> blocks;
};

// =============================================================================
// ----------------------------- DIRECTORY INODE -------------------------------
// =============================================================================

// Forward declaration for `DirEntry`
template <typename T>
class DirEntry;

class DirInode : public Inode<dirblock_t>
{
public:
	DirInode(short id); // Retrieve an existing directory inode from disk
	DirInode();					// Create a new directory inode

	unsigned int get_num_entries();

	vector<DirEntry<FileInode>> get_file_inode_entires();
	vector<DirEntry<DirInode>> get_dir_inode_entries();

	void add_entry(DirEntry<FileInode> entry);
	void add_entry(DirEntry<DirInode> entry);

	// `remove_entry` will only remove the entry from the Dir Inode. It don't
	// destroy the blocks associated with that Inode. You must manually do that!
	void remove_entry(DirEntry<FileInode> entry);
	void remove_entry(DirEntry<DirInode> entry);

	bool has_free_entry();

protected:
	// File and Dir Inodes are stored separately since vectors can only hold a
	// single type (even if they have a common base class). This means there are
	// also different functions for getting and adding entires.
	vector<DirEntry<FileInode>> file_entries;
	vector<DirEntry<DirInode>> dir_entries;

	unsigned int num_entries;

private:
	template <typename T>
	void add_entry_base(DirEntry<T> entry, vector<DirEntry<T>> &vec);
	template <typename T>
	void remove_entry_base(DirEntry<T> entry, vector<DirEntry<T>> &vec);
};

// =====================
// ----- DIR ENTRY -----
// =====================
// Dir entries are stored within a directory inode and serve as pointers to
// other inodes (either file or directory). In order to use this `DirEntry`
// class, you must specify the type of the inode that the entry points to.
//
// Usage example:
//   DirInode home = DirInode(1);
//   FileInode file = FileInode(2);
//   DirEntry<FileInode> file_entry(home, file);
template <typename T>
class DirEntry
{
	// static_assert(is_base_of<Inode<T>, T>::value, "DirEntry must be used with an Inode");

public:
	DirEntry(string name, T inode);

	string get_name();
	T get_inode();

	// DirEntires have no `destroy` function. Instead, call `remove_entry` on
	// the entry's `DirInode` object.

protected:
	string name;
	short inode_id;
};

#endif