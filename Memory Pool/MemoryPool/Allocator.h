#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <cstddef>
#include "MemoryPool.h"

template <typename T, typename pool>
class Allocator
{
public:
    using value_type = T;

    // Constructors
    // In a per-instance design, we can store a pointer/reference to a MemoryPool.
    // If you have only one global pool, you might not need a pointer.
    explicit Allocator() = default;

    void Link(pool& pool) noexcept {
        pool_ = &pool;
    }

    // For containers that need to rebind the allocator to a different type:
    template <class U, class Pool>
    Allocator(const Allocator<U, Pool>& other) noexcept : pool_(other.pool_) {}

    // Allocate memory for n objects of type T
    T* allocate(std::size_t n)
    {
        if (!pool_)
        {
            // If you never set a pool, fallback to global operator new
            T* p = static_cast<T*>(::operator new(n * sizeof(T)));
            DBG_PRINT("Allocated from heap (no MemoryPool): " << n * sizeof(T) << " bytes");
            return p;
        }

        // Attempt to allocate from our MemoryPool
        T* p = static_cast<T*>(pool_->FindMemory(static_cast<int>(n * sizeof(T))));

        if (!p)
        {
            // Fallback to the global heap if the pool is full
            p = static_cast<T*>(::operator new(n * sizeof(T)));
            DBG_PRINT("Allocated from heap: " << n * sizeof(T) << " bytes");
        }
        else
        {
            DBG_PRINT("Allocated from pool: " << n * sizeof(T) << " bytes");
        }
        return p;
    }

    // Deallocate memory for n objects
    void deallocate(T* p, std::size_t n) noexcept
    {

        if (!pool_)
        {
            // If we have no pool, just delete from global
            ::operator delete(p);
            DBG_PRINT("Deallocated from heap (no MemoryPool): " << n * sizeof(T) << " bytes");
            return;
        }

        // Check if the pointer is within the pool buffer range
        const char* poolStart = pool_->getBufferStart();
        const char* poolEnd = pool_->getBufferEnd();

        auto ptrAddress = reinterpret_cast<const char*>(p);
        if (ptrAddress >= poolStart && ptrAddress < poolEnd)
        {
            // Deallocate from the pool
            pool_->Remove(reinterpret_cast<uint64_t>(p));
            DBG_PRINT("Deallocated from pool: " << n * sizeof(T) << " bytes");
        }
        else
        {
            // Otherwise, free from the global heap
            ::operator delete(p);
            DBG_PRINT("Deallocated from heap: " << n * sizeof(T) << " bytes");
        }
    }

private:
    pool* pool_ = nullptr;  // Owned externally; not the Allocator's job to destroy it

    // Let other typed Allocators access our pool_
    template <class U, class A> friend class Allocator;
};

#endif // ALLOCATOR_H