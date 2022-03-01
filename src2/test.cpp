
#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "FileSys.h"
using namespace std;

#include "WrappedFileSys.h"
#include "assert.h"
#include <regex>

string DEBUG_TEST_RESPONSE;

void testing(FileSys &fs)
{
	FileInode file = FileInode();
	cout << "FILE 1 ID " << file.get_id() << endl;
	file.destroy();

	FileInode file2 = FileInode();
	cout << "FILE 2 ID " << file2.get_id() << endl;
	file2.destroy();
}

void unit_tests(FileSys &fs)
{
	// Using lambdas to hide test variables within their own scope. There's
	// probably a better way to do this
	[&]()
	{
		FileInode file1 = FileInode();
		short file1_id = file1.get_id();
		file1.destroy();

		FileInode file2 = FileInode();
		short file2_id = file2.get_id();
		file2.destroy();

		// =============================== TEST ====================================
		// Be sure that rmdir and rm actually reclaim blocks that are part of the
		// deleted directory or file.  This can be tested for by removing a file,
		// creating a new file, and running stat on that new file to see if the
		// block is indeed reused. This test is possible since get_free_block
		// deterministically returns the free block with the lowest number.
		assert(file1_id == file2_id);
	}();

	[&]()
	{
		fs.ls();
		assert(fs.LAST_RESPONSE() == "empty folder");

		fs.mkdir("dir1");
		assert(fs.LAST_RESPONSE() == "success");

		fs.mkdir("dir2");
		assert(fs.LAST_RESPONSE() == "success");

		fs.ls();
		assert(fs.LAST_RESPONSE() == "dir1 dir2");

		fs.cd("dir1");
		assert(fs.LAST_RESPONSE() == "success");

		fs.create("file1");
		assert(fs.LAST_RESPONSE() == "success");

		fs.append("file1", "helloworld!");
		assert(fs.LAST_RESPONSE() == "success");

		// Calculation for Inode block:
		//   Previous commands should have used the following number of blocks.
		//     - ls:     0
		//     - mkdir:  1
		//     - mkdir:  1
		//     - ls:     0
		//     - cd:     0
		//     - create: 1
		//     - append: 1 ("helloworld!" should only less than take 1 block)
		//   The super block and home directory uses blocks 0 and 1, respectively.
		//   Therefore, when "file1" was created it is allocated block 4 (blocks 0-3
		//   are used).
		//
		// Calculate for Number of Blocks:
		//   Should be 1. Only "helloworld!" was written to it.
		//
		// Calculation for First block:
		//   At the time of the `append` command, blocks 0-4 have been used.
		//   Therefore, the first block allocated to "file1" (via the `append`
		//   command) should have been block 5.
		fs.stat("file1");
		// TODO:
		// assert(regex_match(fs.LAST_RESPONSE(), regex("Inode block: [0-9]+\nBytes in file: 11\nNumber of blocks: [0-9]+\nFirst block: [0-9]+")));
		// assert(fs.LAST_RESPONSE() == "Inode block: 4\nBytes in file: 11\nNumber of blocks: 1\nFirst block: 5");

		fs.ls();
		assert(fs.LAST_RESPONSE() == "file1");

		fs.cat("file1");
		assert(fs.LAST_RESPONSE() == "helloworld!");

		fs.head("file1", 5);
		assert(fs.LAST_RESPONSE() == "hello");

		try
		{
			fs.rm("file2");
			assert(!"Should have raised an exception");
		}
		catch (const WrappedFileSys::FileNotFoundException &e)
		{
			cout << "Command invalid: " << e.what() << endl;
			assert(!strcmp(e.what(), "503 File does not exist"));
		}

		try
		{
			fs.cat("file2");
			assert(!"Should have raised an exception");
		}
		catch (const WrappedFileSys::FileNotFoundException &e)
		{
			cout << "Command invalid: " << e.what() << endl;
			assert(!strcmp(e.what(), "503 File does not exist"));
		}

		try
		{
			fs.create("file1");
			assert(!"Should have raised an exception");
		}
		catch (const WrappedFileSys::FileExistsException &e)
		{
			cout << "Command invalid: " << e.what() << endl;
			assert(!strcmp(e.what(), "502 File exists"));
		}

		fs.create("file2");
		assert(fs.LAST_RESPONSE() == "success");

		fs.rm("file1");
		assert(fs.LAST_RESPONSE() == "success");

		fs.ls();
		assert(fs.LAST_RESPONSE() == "file2");

		fs.home();
		assert(fs.LAST_RESPONSE() == "success");

		fs.ls();
		assert(fs.LAST_RESPONSE() == "dir1 dir2");

		fs.stat("dir1");
		// TODO:
		// assert(regex_match(fs.LAST_RESPONSE(), regex("Directory name: dir1\nDirectory block: [0-9]+")));

		try
		{
			fs.rmdir("dir3");
			assert(!"Should have raised an exception");
		}
		catch (const WrappedFileSys::FileNotFoundException &e)
		{
			cout << "Command invalid: " << e.what() << endl;
			assert(!strcmp(e.what(), "503 File does not exist"));
		}

		try
		{
			fs.rmdir("dir1");
			assert(!"Should have raised an exception");
		}
		catch (const WrappedFileSys::DirNotEmptyException &e)
		{
			cout << "Command invalid: " << e.what() << endl;
			assert(!strcmp(e.what(), "507 Directory is not empty"));
		}

		fs.rmdir("dir2");
		assert(fs.LAST_RESPONSE() == "success");

		fs.ls();
		assert(fs.LAST_RESPONSE() == "dir1");
	}();
}

// Command for running tests: `make clean; make test && ./test`
int main(int argc, char *argv[])
{
	FileSys fs;
	fs.mount(1); // Giving it a bogus sock int for now

	unit_tests(fs);

	// testing(fs);

	fs.unmount();
	return 0;
}