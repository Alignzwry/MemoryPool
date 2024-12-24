#include "MemoryPool/MemoryPool.h"
#include "MemoryPool/Allocator.h"
#include <iostream>
#include <vector>

// For testing, should not be called once
#ifdef _DEBUG
void* operator new(std::size_t size) {
	std::cout << "memory allocation: " << size << " bytes" << std::endl;
	__debugbreak();
	return malloc(size);
}
#endif

int main() {
	/// Use Global Pool with String
	Allocator<char> globalAlloc;

	// Create a std::basic_string that uses that Allocator
	auto string = std::basic_string<char, std::char_traits<char>, Allocator<char>>(globalAlloc);
	string = "afakwfjaw";

	for (int i = 0; i < 100; i++)
		string.append("12334124");

	/// Use Local Pool with String
	// Create a local instance of StringPool
	MemoryPool<4096, 32> myPool;

	// Create an Allocator that references that pool
	Allocator<char, MemoryPool<4096, 32>> poolAlloc;
	poolAlloc.Link(myPool);

	// Now create a std::basic_string that uses that Allocator
	auto string2 = std::basic_string<char, std::char_traits<char>, Allocator<char, MemoryPool<4096, 32>>>(poolAlloc);
	string2 = "afakwfjaw";

	for (int i = 0; i < 100; i++)
		string2.append("12334124");

	/// Use Local Pool with Vector
	// Create a pool of 4096 bytes with up to 100 entries
	MemoryPool<16000, 32> myPool2;

	// Create an allocator that will use that pool
	Allocator<int, MemoryPool<16000, 32>> myAlloc;
	myAlloc.Link(myPool2);  // Tell our allocator to use myPool

	// Create a vector of int that uses your custom allocator
	auto myVec = std::vector<int, Allocator<int, MemoryPool<16000, 32>>>(myAlloc);

	for (int i = 0; i < 1000; i++)
	{
		myVec.push_back(i);
	}

	return 0;
}