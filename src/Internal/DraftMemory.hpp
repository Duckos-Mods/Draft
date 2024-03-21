#pragma once
#include <memory>
#include "DraftUtils.hpp"
#include "DraftASMTypes.hpp"

namespace Draft 
{
    enum class MemoryProtection : ProtectionType
    {
        Read = 0x01,
        Write = 0x02,
        Execute = 0x04,

        ReadWrite = Read | Write,
        ReadWriteExecute = Read | Write | Execute
    };

    class DraftProtection
    {
    public:
        DraftProtection() = default;
        DraftProtection(MemoryProtection protectionFlags) { m_protectionFlags = EnumToSystemFlags(protectionFlags); }
        DraftProtection(ProtectionType protectionFlags) : m_protectionFlags(protectionFlags) {}


        operator ProtectionType() const {
			return m_protectionFlags;
		}

        operator MemoryProtection() const {
			return SystemFlagsToEnum(m_protectionFlags);
		}

        DraftProtection operator=(ProtectionType protectionFlags) {
			m_protectionFlags = protectionFlags;
			return m_protectionFlags;
		}

        static ProtectionType EnumToSystemFlags(MemoryProtection protectionFlags) {
            ProtectionType flags = 0;
            uint64_t protection = static_cast<uint64_t>(protectionFlags);

            if (protection == 7)
                return PXW;

            if (protection & static_cast<uint64_t>(MemoryProtection::Read)) {
                flags |= PR;
            }
            if (protection & static_cast<uint64_t>(MemoryProtection::Write)) {
                flags |= PW;
            }
            if (protection & static_cast<uint64_t>(MemoryProtection::Execute)) {
                flags |= PX;
            }
            
            return flags;
        }

        static MemoryProtection SystemFlagsToEnum(ProtectionType protectionFlags) {
			MemoryProtection flags = MemoryProtection::Read;
            if (protectionFlags & PR) {
				flags = static_cast<MemoryProtection>(static_cast<uint64_t>(flags) | static_cast<uint64_t>(MemoryProtection::Read));
			}
            if (protectionFlags & PW) {
				flags = static_cast<MemoryProtection>(static_cast<uint64_t>(flags) | static_cast<uint64_t>(MemoryProtection::Write));
			}
            if (protectionFlags & PX) {
				flags = static_cast<MemoryProtection>(static_cast<uint64_t>(flags) | static_cast<uint64_t>(MemoryProtection::Execute));
			}
			return flags;
		}

        static ProtectionType Read() {
            return EnumToSystemFlags(MemoryProtection::Read);
        }

        static ProtectionType Write() {
            return EnumToSystemFlags(MemoryProtection::Write);
        }

        static ProtectionType Execute() {
            return EnumToSystemFlags(MemoryProtection::Execute);
        }

        ProtectionType operator=(MemoryProtection protectionFlags) {
            m_protectionFlags = EnumToSystemFlags(protectionFlags);
            return m_protectionFlags;
        }

    private:
        ProtectionType m_protectionFlags;
    };

    // ABSTRACTION LAYER

    void SetProtection(void* ptr, size_t size, DraftProtection protections);

    template<typename T>
    void SetToRWE(T* ptr, size_t size) {
		SetProtection(ptr, size, MemoryProtection::ReadWriteExecute);
	}

    void* NearAlloc(uintptr_t targetAddress, size_t size, DraftProtection protections = MemoryProtection::ReadWriteExecute, size_t alignment = 0x10);

    template<typename T>
    T* NearAlloc(uintptr_t targetAddress, size_t size, DraftProtection protections = MemoryProtection::ReadWriteExecute, size_t alignment = 0x10) {
        return static_cast<T*>(NearAlloc(targetAddress, size * sizeof(T), protections, alignment));
    }
    
    void OsFree(void* ptr, size_t size);

    template<typename T>
    void OsFree(T* ptr, size_t size) {
		OsFree(ptr, size * sizeof(T));
	}

    // RAW MEMORY ALLOCATION
    template<typename T>
    T* RawAlloc(size_t count) {
        return static_cast<T*>(operator new(sizeof(T) * count));
    }

    template<typename T>
    T* RawAlineAlloc(size_t count, size_t alignment) {
        return static_cast<T*>(_aligned_malloc(sizeof(T) * count, alignment));
    }

    template<typename T>
    T* RawRealloc(T* ptr, size_t count) {
        return static_cast<T*>(operator new(sizeof(T) * count));
    }

    template<typename T>
    T* RawCalloc(size_t count) {
        return static_cast<T*>(operator new(sizeof(T) * count));
    }

    template<typename T>
    void RawFree(T* ptr) {
        operator delete(ptr);
    }

    template<typename T>
    T* RawRWEAlloc(size_t count, size_t alignment = 0x10) {
        T* ptr = RawAlineAlloc<T>(count, alignment);
        DRAFT_THROW_IF(ptr == nullptr, "Failed to allocate memory");
        SetProtection(ptr, count, MemoryProtection::ReadWriteExecute);
        return ptr;
    }

    // SHARED MEMORY ALLOCATION

    template<typename T>
    std::shared_ptr<T> SharedAlloc(size_t count) {
        return std::shared_ptr<T>(RawAlloc<T>(count), RawFree<T>);
    }

    template<typename T>
    std::shared_ptr<T> SharedAlineAlloc(size_t count, size_t alignment) {
        return std::shared_ptr<T>(RawAlineAlloc<T>(count, alignment), RawFree<T>);
    }

    template<typename T>
    std::shared_ptr<T> SharedRealloc(T* ptr, size_t count) {
        return std::shared_ptr<T>(RawRealloc<T>(ptr, count), RawFree<T>);
    }

    template<typename T>
    std::shared_ptr<T> SharedCalloc(size_t count) {
        return std::shared_ptr<T>(RawCalloc<T>(count), RawFree<T>);
    }

    template<typename T>
    std::shared_ptr<T> SharedRWEAlloc(size_t count, size_t alignment = 0x10) {
        return std::shared_ptr<T>(RawRWEAlloc<T>(count, alignment), RawFree<T>);
    }

    // UNIQUE MEMORY ALLOCATION
    template<typename T>
    std::unique_ptr<T> UniqueAlloc(size_t count) {
        return std::unique_ptr<T>(RawAlloc<T>(count), RawFree<T>);
    }

    template<typename T>
    std::unique_ptr<T> UniqueAlineAlloc(size_t count, size_t alignment) {
        return std::unique_ptr<T>(RawAlineAlloc<T>(count, alignment), RawFree<T>);
    }

    template<typename T>
    std::unique_ptr<T> UniqueRealloc(T* ptr, size_t count) {
        return std::unique_ptr<T>(RawRealloc<T>(ptr, count), RawFree<T>);
    }

    template<typename T>
    std::unique_ptr<T> UniqueCalloc(size_t count) {
        return std::unique_ptr<T>(RawCalloc<T>(count), RawFree<T>);
    }

    template<typename T>
    std::unique_ptr<T> UniqueRWEAlloc(size_t count, size_t alignment = 0x10) {
        return std::unique_ptr<T>(RawRWEAlloc<T>(count, alignment), RawFree<T>);
    }

    // UTILITIES


}