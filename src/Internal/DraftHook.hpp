#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include "DraftMemory.hpp"

namespace Draft
{
    using UnkfuncPtr = void*;

    enum class HookInstallMethod
    {
        // Automatically chooses the best method for the target function this can be slower than choosing the method yourself due to it all 2 times and comparing addresses
        JumpAutomaticInstall, 

        // Installs a jump to the target function with 20 bytes
        Jump20byteInstall,

        // Installs a jump to the target function with 5 bytes
        Jump5byteInstall,

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


        /**
        * @brief Uninstalls the hook
        */
        void Uninstall() {
            if (m_location != nullptr)
            {
                memcpy(m_location, m_originalFunction.m_function, m_originalFunction.m_size);
                OsFree(m_originalFunction.m_function, m_originalFunction.m_size); // Free the copied function
			}
        }

        bool IsInstalled() const {
			return m_isInstalled;
		}
    private:
        bool FindInstallMethod(UnkfuncPtr targetLocation, HookInstallMethod& installMethod, UnkfuncPtr hookFunction);
        bool GenerateTrampoline(UnkfuncPtr targetFunction, AllocationMethod allocationMethod, size_t alignment, size_t bytesToAllocate);
        bool IsBigEnough(UnkfuncPtr targetFunction, size_t size);
    private:
        void* m_location = nullptr; // The location of the function we are hooking
        void* m_trampoline = nullptr; // Used to call the original function head that we overwrote then jump back to the original function
        UnkownFunctionData m_originalFunction; // The original function data
        bool m_isInstalled = false;




    };
}