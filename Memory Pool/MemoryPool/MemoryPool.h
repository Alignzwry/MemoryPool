#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <string>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <thread>

#ifdef _DEBUG
#include <crtdbg.h>
#include <iostream>
_ACRTIMP void __cdecl _wassert(
    _In_z_ wchar_t const* _Message,
    _In_z_ wchar_t const* _File,
    _In_   unsigned       _Line
);
#define THROW_ERROR(message) _wassert(_CRT_WIDE(message), _CRT_WIDE(__FILE__), (unsigned)(__LINE__))
#define DBG_PRINT(message) std::cout << message << std::endl
#else
#define THROW_ERROR(message) ((void)0)
#define DBG_PRINT(message)
#endif

struct Entry
{
    int      size = 0;
    uint64_t address = 0;
};

template <int Pool_Size, int Max_Entries>
class MemoryPool
{
public:
    MemoryPool()
    {
        // Not Really Necessary
        std::memset(entries, 0, sizeof(entries));
    }

    // Removes (deallocates) the memory at the given address from the pool
    void Remove(uint64_t address)
    {
        // Lock the mutex for thread safety
        std::lock_guard<std::mutex> lock(pool_mutex);

        for (int i = 0; i < Max_Entries; i++)
        {
            Entry& entry = entries[i];
            if (entry.address == address)
            {
                // Mark this entry as free
                entry.address = 0;
                entry.size = 0;
                return;
            }
        }
        THROW_ERROR("Address not found");
    }

    // Finds and reserves a contiguous segment of memory of given length in the pool
    void* FindMemory(int length)
    {
        // Lock the mutex for the duration of allocation
        std::lock_guard<std::mutex> lock(pool_mutex);

        // Start searching from beginning of buffer
        uint64_t currentAddress = reinterpret_cast<uint64_t>(buffer);

        bool   hasConflict = false;
        Entry* emptyEntrySlot = nullptr;

        // First pass: see if we can place our new block
        for (int i = 0; i < Max_Entries; ++i)
        {
            Entry& entry = entries[i];
            if (entry.address == 0)
            {
                // Keep track of the first empty slot
                if (!emptyEntrySlot) {
                    emptyEntrySlot = &entry;
                }
                continue;
            }

            // If new block overlaps with this existing one, 
            // move currentAddress just beyond it
            uint64_t entryStart = entry.address;
            uint64_t entryEnd = entry.address + entry.size;
            uint64_t newBlockEnd = currentAddress + length;

            if (currentAddress <= entryEnd && newBlockEnd > entryStart)
            {
                // We have overlap, so move currentAddress
                currentAddress = entryEnd;
                // Check if there's enough space left in buffer
                if (currentAddress + length > reinterpret_cast<uint64_t>(buffer + Pool_Size))
                {
                    hasConflict = true;
                    break; // no space
                }
            }
        }

        if (hasConflict)
        {
            THROW_ERROR("No suitable address found in pool");
            return nullptr;
        }

        // If still no empty slot, the pool is effectively full
        if (!emptyEntrySlot)
        {
            THROW_ERROR("No Entry. Increase Max_Entries or Pool_Size.");
            return nullptr;
        }

        // Commit the allocation
        emptyEntrySlot->address = currentAddress;
        emptyEntrySlot->size = length;

        // Return the pointer to the allocated portion of the buffer
        return reinterpret_cast<void*>(currentAddress);
    }

    // Clears all entries in the pool
    void Clear()
    {

        std::lock_guard<std::mutex> lock(pool_mutex);
        std::memset(entries, 0, sizeof(entries));
    }

    const char* getBufferStart() const { return buffer; }
    const char* getBufferEnd()   const { return buffer + Pool_Size; }

private:
    char   buffer[Pool_Size];
    Entry  entries[Max_Entries];
    std::mutex pool_mutex;
};

#endif // MEMORY_POOL_H
