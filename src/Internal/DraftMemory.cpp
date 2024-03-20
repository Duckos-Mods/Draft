#include "DraftMemory.hpp"

namespace Draft
{
    void SetProtection(void* ptr, size_t size, DraftProtection protections)
    {
        #ifdef _WIN32
        DWORD oldProtect;
#ifndef ENABLE_DEBUG
        DRAFT_THROW_IF(!VirtualProtect(ptr, size, protections, &oldProtect), "Failed to set memory to requested protections");
#else
        if (!VirtualProtect(ptr, size, protections, &oldProtect))
        {
			std::string error = "Failed to set memory to requested protections: ";
			error += std::to_string(GetLastError());
            std::cout << error << std::endl;
			DRAFT_THROW(error);
		}
#endif
        #else
        long pageSize = sysconf(_SC_PAGESIZE)
        DRAFT_THROW_IF(pageSize == -1, "Failed to get page size");
        uintptr_t alignedPtr = reinterpret_cast<uintptr_t>(ptr) & ~(pageSize - 1);
        uintptr_t endPtr = reinterpret_cast<uintptr_t>(ptr) + size;
        uintptr_t endAlignedPtr = endPtr & ~(pageSize - 1);
        size_t alignedSize = endAlignedPtr - alignedPtr;
        int result = mprotect(reinterpret_cast<void*>(alignedPtr), alignedSize, protections);
        DRAFT_THROW_IF(result == -1, "Failed to set memory to RWE");
        #endif

    }

    void* NearAlloc(uintptr_t targetAddress, size_t size, DraftProtection protections, size_t alignment)
    {
        void* ptr = nullptr;
        #ifdef _WIN32
        ptr = VirtualAllocEx(GetCurrentProcess(), reinterpret_cast<void*>(targetAddress), size, MEM_COMMIT | MEM_RESERVE, protections);
        DRAFT_THROW_IF(ptr == nullptr, "Failed to allocate memory");
        #else
        ptr = mmap(reinterpret_cast<void*>(targetAddress), size, protections, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
        DRAFT_THROW_IF(ptr == MAP_FAILED, "Failed to allocate memory");
        #endif
        return ptr;
    }

    void OsFree(void* ptr, size_t size)
    {
        #ifdef _WIN32
		DRAFT_THROW_IF(!VirtualFree(ptr, 0, MEM_RELEASE), "Failed to free memory");
		#else
        DRAFT_THROW_IF(munmap(ptr, size) == -1, "Failed to free memory");
		#endif
    }
}
