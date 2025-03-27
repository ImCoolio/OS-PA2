Concurrent Hash Table Implementation

This project implements a concurrent hash table data structure that supports multiple threads performing operations on a shared collection of key-data pairs. The implementation uses a reader-writer lock to ensure thread safety while allowing concurrent reads.

Features:
- Thread-safe operations using reader-writer locks
- Jenkins one-at-a-time hash function for key hashing
- Support for insert, delete, search, and print operations
- Sorted output of records by hash value
- Detailed logging of lock operations and timestamps

Build Instructions:
1. Run 'make' to build the project
2. Place your commands.txt file in the same directory
3. Run './chash' to execute the program

AI Usage Disclosure:
This project was developed with assistance from AI tools. The following aspects were aided by AI:
1. Initial code structure and organization
2. Implementation of the reader-writer lock mechanism
3. Hash table operations and thread safety considerations
4. Error handling and memory management patterns

The AI was used as a development tool to help structure the code and implement standard patterns, but all final decisions and implementations were made by the developer. The code has been thoroughly tested and verified to ensure correctness and thread safety.

Author: [Your Name]
Date: [Current Date] 