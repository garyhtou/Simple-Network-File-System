## client and server

- user interacts with client through the console
  - For exmaple 'mkdir abc'
- The client then uses a socket to communiate with the server
  - network client stub talks to network server stub
  - client parse the command and translate to network message
- server has a port number
- server receives and parses the network message
- translate into local function calls
  - local functions interact with the file system
- File system is implemented on the server
  - `FileSys.cpp/.h`
    - That's built on top of `BasicFileSystem.cpp/.h` (DON'T TOUCH)
      - That is built on top of a virtual disk (DON'T TOUCH)
      - APIs: read, write, get block, release block
		- map filename to disk blocks
			- using inodes
  	- Manages the disk blocks using the APIs
- server sends the response back to the client


**components:**
- network communication (tcp)
- file system operations using provided api

