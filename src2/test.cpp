
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

void testing()
{
	FileInode file = FileInode();
	cout << "FILE 1 ID " << file.get_id() << endl;
	file.destroy();

	FileInode file2 = FileInode();
	cout << "FILE 2 ID " << file2.get_id() << endl;
	file2.destroy();
}

void unit_tests()
{
	// Using lambdas to hide test variables within their own scope. There's
	// probably a better way to do this
	[]()
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
}

// Command for running tests: `make clean; make test && ./test`
int main(int argc, char *argv[])
{
	FileSys fs;
	fs.mount(1); // Giving it a bogus sock int for now

	// testing();

	unit_tests();

	fs.unmount();
	return 0;
}