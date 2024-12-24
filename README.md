# MemoryPool and Allocator

This library provides a lightweight, **fixed-size memory pool** (`MemoryPool`) and a custom C++ allocator (`Allocator`) designed to minimize or eliminate usage of the global heap. When properly configured, **all allocations and deallocations can be handled within the pool**, preventing unexpected or fragmented heap usage.

---

## Project Goal

**Primary objective**: **Remove (or drastically reduce) dynamic memory allocations on the global heap.** By setting a sufficient `Pool_Size` and `Max_Entries`, you can store allocations in a contiguous buffer instead of repeatedly calling the default `new` operator.

## File Overview

- **MemoryPool.h**  
  Declares and defines the `MemoryPool<Pool_Size, Max_Entries>` template:
  - Maintains a **fixed-size buffer** of `Pool_Size` bytes.  
  - Tracks up to `Max_Entries` simultaneous allocations.  
  - Provides thread safety via a `std::mutex`.  

- **Allocator.h**  
  Declares and defines the `Allocator<T, pool>` template, a **custom STL-compliant allocator**:
  - Routes all allocations and deallocations to the linked `MemoryPool` when possible.  
  - If you want to entirely avoid the global heap, remove or alter the fallback call to `::operator new`.

- **main.cpp**  
  Demonstrates usage of the memory pool with the custom allocator:
  - Creates `MemoryPool<4096, 100>` and an `Allocator<char>` that references it.  
  - Shows how to use a `std::basic_string<char>` or `std::vector<int>` with the custom allocator.  
  - Includes a debug override of `operator new` to catch any unwanted heap allocations (for demonstration).

## Basic Usage

1. **Include Headers**  
   ```cpp
   #include "MemoryPool.h"
   #include "Allocator.h"
   ```

2. **Create a MemoryPool**  
   ```cpp
   MemoryPool<4096, 100> myPool; 
   // 4096 bytes, up to 100 allocated blocks simultaneously
   ```

3. **Create and Link the Allocator**  
   ```cpp
   Allocator<char, MemoryPool<4096, 100>> poolAlloc;
   poolAlloc.Link(myPool);  // Associate the allocator with the MemoryPool
   ```

4. **Use With STL Containers**  
   ```cpp
   // Example: string
   auto myString = std::basic_string<char, std::char_traits<char>, 
                     Allocator<char, MemoryPool<4096, 100>>>(poolAlloc);
   myString = "Hello from the pool!";

   // Example: vector
   Allocator<int, MemoryPool<4096, 100>> intAlloc;
   intAlloc.Link(myPool);
   std::vector<int, Allocator<int, MemoryPool<4096, 100>>> myVec(intAlloc);

   myVec.push_back(42); 
   ```

5. **Avoiding Heap Allocations Entirely**  
   - **Set your pool size large enough** for all intended allocations.  
   - If you don’t want any fallback, remove or customize the code path in `Allocator::allocate()` and `Allocator::deallocate()` that calls the global `new`/`delete`.  
   - In `_DEBUG` mode, a custom global `operator new` is provided in `main.cpp` that triggers a breakpoint whenever the heap is used. This helps you detect any unintentional fallback allocations.

6. **Clearing the Pool**  
   ```cpp
   myPool.Clear(); 
   // Resets all tracked allocations in the pool
   ```

## Important Considerations

1. **Fixed Pool Size**  
   If you run out of space in the pool (`Pool_Size`) or exceed `Max_Entries`, allocations may fail. By default, this sample code might fall back to the global heap—**remove or modify that fallback** if you truly want zero heap usage.

2. **Thread Safety**  
   - All allocations/deallocations are guarded by a `std::mutex`.  
   - For multi-threaded applications, ensure you account for concurrency patterns to avoid collisions beyond just memory safety (e.g., do not repeatedly re-clear the pool while other threads are active).

3. **Debug and Error Handling**  
   - In `_DEBUG` mode, `THROW_ERROR` triggers an assertion; otherwise, it’s a no-op.  
   - `DBG_PRINT` prints debugging info.  
   - You can remove, replace, or expand these macros based on your project’s needs.

4. **Custom `operator new` for Debug**  
   - The example `main.cpp` overrides `operator new` to highlight calls to the global heap.  
   - Use it for debugging to ensure your pool is successfully handling all allocations. In production code, you may omit or disable this override.

## Example Build and Run

```bash
# On Linux or macOS using g++:
# g++ -std=c++17 -o test main.cpp
# ./test

# On Windows with Visual Studio:
# 1. Add MemoryPool.h, Allocator.h, and main.cpp to your project.
# 2. Compile in Debug mode to catch any default new calls.
# 3. Run your application.
```

## License

MIT

## Contributing

- PRs to enhance or refine the memory pool logic, debugging features, or allocation strategies are welcome.  
- If you find an issue, open a ticket in the repository.  

