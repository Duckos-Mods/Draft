#pragma once
#include <stdexcept>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
using ProtectionType = DWORD;
constexpr ProtectionType PX = PAGE_EXECUTE;
constexpr ProtectionType PW = PAGE_READWRITE;
constexpr ProtectionType PR = PAGE_READONLY;
constexpr ProtectionType PXW = PAGE_EXECUTE_READWRITE;
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
using ProtectionType = int;
constexpr ProtectionType PX = PROT_EXEC;
constexpr ProtectionType PW = PROT_WRITE;
constexpr ProtectionType PR = PROT_READ;
constexpr ProtectionType PXW = PAGE_EXECUTE_READWRITE;
#else
#error "Unsupported platform"
#endif


#if DRAFT_NOEXCEPT == 1
#define DRAFT_THROW(x) exit(1)
#define DRATF_THROW_IF(x, y) if(x) exit(1)
#else
#define DRAFT_THROW(x) throw std::runtime_error(x)
#define DRAFT_THROW_IF(x, y) if(x) throw std::runtime_error(y)
#endif

#define DRAFT_HANDLE_ERR(x, y) if(x) y
#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#include <iostream>
#include <string>
#define DRAFT_DEBUG_LOG(x) std::cout << x << std::endl
#define DRAFT_DEBUG_CODE(x) x
#else
#define DRAFT_DEBUG_LOG(x)
#define DRAFT_DEBUG_CODE(x)
#endif

namespace Draft
{
	template<typename Enum, typename ReturnType = int>
	constexpr ReturnType EnumToUnderlying(Enum e) noexcept
	{
		return static_cast<ReturnType>(e);
	}


	template <typename FunctionPtr>
	void* LocalFunctionPointerToRawAddress(FunctionPtr ptr)
	{
		return *reinterpret_cast<void**>(&ptr);
	}
}