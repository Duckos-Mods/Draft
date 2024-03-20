#include "DraftHook.hpp"
#include "DraftASM.hpp"


namespace Draft
{
	bool Hook::Install(UnkfuncPtr targetFunction, UnkfuncPtr hookFunction, HookInstallMethod installMethod, AllocationMethod allocationMethod, size_t alignment)
	{
		DRAFT_HANDLE_ERR(IsInstalled(), return false);

		if (installMethod == HookInstallMethod::JumpAutomaticInstall)
			DRAFT_HANDLE_ERR(!FindInstallMethod(targetFunction, installMethod, hookFunction), return false);
		size_t sizes = (installMethod == HookInstallMethod::Jump5byteInstall) ? 5 : 20;
		SetToRWE(targetFunction, sizes);
		// Test if the function is large enough to install the hook
		GenerateTrampoline(targetFunction, allocationMethod, alignment, sizes);

		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(targetFunction), sizes);
		if (installMethod == HookInstallMethod::Jump5byteInstall)
		{
			int32_t jmpOffset = reinterpret_cast<uintptr_t>(hookFunction) - reinterpret_cast<uintptr_t>(targetFunction) - 5;
			jmp32 jmp(jmpOffset);
			asmHandler.Write(jmp);
		}
		else
		{
			jmp64 jmp(reinterpret_cast<uintptr_t>(hookFunction));
			asmHandler.Write(jmp);
		}
		SetProtection(hookFunction, sizes, MemoryProtection::Execute);
		return true;
	}
	bool Hook::FindInstallMethod(UnkfuncPtr targetLocation, HookInstallMethod& installMethod, UnkfuncPtr hookFunction)
	{
		HookInstallMethod bestMethod = HookInstallMethod::Jump20byteInstall;

		// Check if the target function is within 4GB of the current function
		uint64_t distance = ((uint64_t) targetLocation > (uint64_t) hookFunction) 
			? (uint64_t) targetLocation - (uint64_t) hookFunction 
			: (uint64_t) hookFunction - (uint64_t) targetLocation;

		if (distance < 0x7FFFFFFF)
			bestMethod = HookInstallMethod::Jump5byteInstall;

		installMethod = bestMethod;
		return true;
	}
	bool Hook::GenerateTrampoline(UnkfuncPtr targetFunction, AllocationMethod allocationMethod, size_t alignment, size_t bytesToAllocate)
	{
		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(targetFunction), bytesToAllocate);
		return true;
	}
	bool Hook::IsBigEnough(UnkfuncPtr targetFunction, size_t size)
	{
		size_t func = reinterpret_cast<uintptr_t>(targetFunction);
		size_t offset = 0;
		bool hasHitCC = false;
		for (size_t i = 0; i < size; i++)
		{
			if (reinterpret_cast<char*>(targetFunction)[i] == 0xCC)
			{
				hasHitCC = true;
#ifdef DRAFT_NO_PAD_WRITE
				return false;
#endif
			}
		}

		for (size_t i = offset; i < size; i++)
		{
			if (reinterpret_cast<char*>(targetFunction)[i] != 0xCC)
				return false;
		}
		return true;
	}
}