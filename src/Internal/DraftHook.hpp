#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include "DraftMemory.hpp"
#include "DraftASM.hpp"
#include "DraftTypes.hpp"

namespace Draft
{
    using UnkfuncPtr = void*;

    enum class HookInstallMethod
    {
        // Automatically chooses the best method for the target function this can be slower than choosing the method yourself due to it all 2 times and comparing addresses
        JumpAutomaticInstall, 

        // Installs a ret to the target function with 20 bytes. This is only here for completeness and is not recommended to be used. The 14 byte install method is faster
        Ret20byteInstall = 20,

        // Installs a jump to the target function with 14 bytes
        Jump14byteInstall = 14,

        // Installs a jump to the target function with 5 bytes
        Jump5byteInstall = 5,

        InstallMethodCount
    };

    enum class AllocationMethod
    {
        // Just calles AlinedAlloc with the specified alignment by default 0x10
        FarAlloc, 

        // Allocates memory near the target function and aligns it to the specified alignment by default 0x10. This aloows for a jump5byte install method to be used, 
        // with a near gurenteed chance of success becuase most of the time the target function is within 4GB of the current function
        NearAlloc, 

        AllocationMethodCount
    };

    struct UnkownFunctionData
    {
        void* m_function = nullptr;
        size_t m_size = -1;
    };


    /**
    * @brief This function attempts to take any function pointer and make a new function that calls it that is close to the requested address
    * @brief This is useful for when you have a 6 byte function you need to hook so you have to use a 5 byte jump but the target function is too far away
    * @param targetAddress The address of the target function
    * @param function The function to call
    * @param alignment The alignment of the memory to allocate
    * @return The address of the new function that calls the target function
    * @note This function is not thread safe and should only be called once per target function
    * @example
    * @code
    * void* targetFunction = reinterpret_cast<void*>(0x12345678);
    * void* newFunction = NearAddressFunction(targetFunction, &MyFunction);
    * @endcode
    */
    template<typename FuncPtrType>
    void* NearAddressFunction(void* targetAddress, FuncPtrType function, size_t alignment = 0x10) {
        void* ptr = RawAlineAlloc<uint8_t>(EnumToUnderlying(HookInstallMethod::Jump14byteInstall), alignment);
        DRAFT_THROW_IF(ptr == nullptr, "Failed to allocate memory");
        void* funcPtr = *reinterpret_cast<void**>(&function);
        jmp64 jmp(reinterpret_cast<uint64_t>(funcPtr));
        ASMHandler handler(ptr, EnumToUnderlying(HookInstallMethod::Jump14byteInstall));
        handler.Write(jmp);
        SetProtection(ptr, EnumToUnderlying(HookInstallMethod::Jump14byteInstall), MemoryProtection::Execute);
        return ptr;
    }

    /**
    * @brief This class is thread safe if the hooked function is thread safe. Although calling uninstall from multiple threads at the same time is undefined behavior
    * @brief This class is used to hook functions and call the original function from the hook. This class isnt used when inline hooking is being used.
    * @brief On destruction the hook is uninstalled so keep the hook object alive for the duration of the hook
    * 
    */
    class Hook
    {
    public:
        Hook() = default;
        ~Hook() {
			Uninstall();
		}

        bool Install(
            UnkfuncPtr targetFunction,
            UnkfuncPtr hookFunction,
            HookInstallMethod installMethod = HookInstallMethod::JumpAutomaticInstall,
            AllocationMethod allocationMethod = AllocationMethod::FarAlloc,
            size_t alignment = 0x10
        );

        template<typename funcPtrType>
        bool Install(
			funcPtrType targetFunction,
            funcPtrType hookFunction,
			HookInstallMethod installMethod = HookInstallMethod::JumpAutomaticInstall,
			AllocationMethod allocationMethod = AllocationMethod::FarAlloc,
			size_t alignment = 0x10
		) {
			return Install(*reinterpret_cast<UnkfuncPtr*>(&targetFunction), *reinterpret_cast<UnkfuncPtr*>(&hookFunction), installMethod, allocationMethod, alignment);
		}


        /**
        * @brief Uninstalls the hook
        */
        void Uninstall();

        bool IsInstalled() const {
			return m_isInstalled;
		}

        template<typename T, typename... Args>
        T fastCall(Args... args) {
            auto funcPtr = reinterpret_cast<T(__fastcall *)(Args...)>(m_trampoline);
            return funcPtr(args...);
		}

		template<typename T, typename... Args>
		T stdCall(Args... args) {
			auto funcPtr = reinterpret_cast<T(__stdcall *)(Args...)>(m_trampoline);
			return funcPtr(args...);
        }

        template<typename T, typename... Args>
		T cdeclCall(Args... args) {
            auto funcPtr = reinterpret_cast<T(__cdecl *)(Args...)>(m_trampoline);
            return funcPtr(args...);
        }

        template<typename T, typename... Args>
		T thisCall(Args... args) {
			auto funcPtr = reinterpret_cast<T(__thiscall *)(Args...)>(m_trampoline);
			return funcPtr(args...);
		}



    private:
        bool FindInstallMethod(UnkfuncPtr targetLocation, HookInstallMethod& installMethod, UnkfuncPtr hookFunction);
        bool GenerateTrampoline(UnkfuncPtr targetFunction, AllocationMethod allocationMethod, size_t alignment, HookInstallMethod installMethod);
        bool IsBigEnough(UnkfuncPtr targetFunction, size_t size);
    private:
        void* m_location = nullptr; // The location of the function we are hooking
        void* m_trampoline = nullptr; // Used to call the original function head that we overwrote then jump back to the original function
        UnkownFunctionData m_originalFunction; // The original function data
        bool m_isInstalled = false;
    };

    using InlineHookFunction = void(*)(CPURegRepresentation* regs);

    class InlineHook
    {
    public:
        InlineHook() = default;
		~InlineHook() {
			Uninstall();
		}

        bool Install(
            UnkfuncPtr targetAddress,
            InlineHookFunction hookFunction,
            AllocationMethod allocationMethod = AllocationMethod::FarAlloc,
            size_t alignment = 0x10
        );

        template<typename funcPtrType>
        bool Install(
            funcPtrType targetAddress,
            InlineHookFunction hookFunction,
            AllocationMethod allocationMethod = AllocationMethod::FarAlloc,
            size_t alignment = 0x10
        ) {
            return Install(*reinterpret_cast<UnkfuncPtr*>(&targetAddress), *reinterpret_cast<UnkfuncPtr*>(&hookFunction), allocationMethod, alignment);
        }

        void Uninstall() {}

        bool IsInstalled() const {
			return m_isInstalled;
		}
    private:
        bool GenerateOriginalFunction(UnkfuncPtr targetAddress, size_t size);
    private:
        CPURegRepresentation m_regs;
        void* m_location = nullptr; // The location we are hooking
		UnkownFunctionData m_originalFunction; // The original function data
        UnkownFunctionData m_customJITFunction; // The custom JIT function data
		bool m_isInstalled = false;
    };
}