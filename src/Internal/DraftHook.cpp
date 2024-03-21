#include "DraftHook.hpp"
#include "DraftASM.hpp"


namespace Draft
{
	bool Hook::Install(UnkfuncPtr targetFunction, UnkfuncPtr hookFunction, HookInstallMethod installMethod, AllocationMethod allocationMethod, size_t alignment)
	{
		DRAFT_HANDLE_ERR(IsInstalled(), return false);

		if (installMethod == HookInstallMethod::JumpAutomaticInstall)
			DRAFT_HANDLE_ERR(!FindInstallMethod(targetFunction, installMethod, hookFunction), return false);
		size_t sizes = EnumToUnderlying<HookInstallMethod, size_t>(installMethod);
		SetToRWE(targetFunction, sizes);
		// Test if the function is large enough to install the hook
		GenerateTrampoline(targetFunction, allocationMethod, alignment, installMethod);

		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(targetFunction), sizes);
		switch (installMethod)
		{
		case Draft::HookInstallMethod::JumpAutomaticInstall:
			break;
		case Draft::HookInstallMethod::Ret20byteInstall:
			{
				ret64 jmp(reinterpret_cast<uintptr_t>(hookFunction));
				asmHandler.Write(jmp);
				break;
			}
		case Draft::HookInstallMethod::Jump5byteInstall:
			{
				int32_t jmpOffset = reinterpret_cast<uintptr_t>(hookFunction) - reinterpret_cast<uintptr_t>(targetFunction) - 5;
				jmp32 jmp(jmpOffset);
				asmHandler.Write(jmp);
				break;
			}
		case Draft::HookInstallMethod::Jump14byteInstall:
			{
			    jmp64 jmp(reinterpret_cast<uintptr_t>(hookFunction));
				asmHandler.Write(jmp);
				break;
			}
		default:
			Uninstall();
			return false;
		}

		SetProtection(hookFunction, sizes, MemoryProtection::Execute);
		return true;
	}
	void Hook::Uninstall()
	{
		if (m_location == nullptr)
			return;
		SetToRWE(m_location, m_originalFunction.m_size);
		memcpy(m_location, m_originalFunction.m_function, m_originalFunction.m_size);
		SetProtection(m_location, m_originalFunction.m_size, MemoryProtection::Execute);
		free(m_originalFunction.m_function);
		if (m_trampoline == nullptr)
			return;
		_aligned_free(m_trampoline);
		m_trampoline = nullptr;
		m_location = nullptr;

	}
	bool Hook::FindInstallMethod(UnkfuncPtr targetLocation, HookInstallMethod& installMethod, UnkfuncPtr hookFunction)
	{
		HookInstallMethod bestMethod = HookInstallMethod::Jump14byteInstall;

		uint64_t distance = ((uint64_t) targetLocation > (uint64_t) hookFunction) 
			? (uint64_t) targetLocation - (uint64_t) hookFunction 
			: (uint64_t) hookFunction - (uint64_t) targetLocation;

		if (distance < 2147483647)
			bestMethod = HookInstallMethod::Jump5byteInstall;

		installMethod = bestMethod;
		return true;
	}
	bool Hook::GenerateTrampoline(UnkfuncPtr targetFunction, AllocationMethod allocationMethod, size_t alignment, HookInstallMethod installMethod)
	{
		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(targetFunction), 0x1000); // Just a BS amount of space
		size_t minimumSliceSize = EnumToUnderlying<HookInstallMethod, size_t>(installMethod);
		size_t sliceSize = asmHandler.GetMinInstrSizeForMinsize(targetFunction, minimumSliceSize);
		size_t trampolineSize = sliceSize + EnumToUnderlying<HookInstallMethod, size_t>(HookInstallMethod::Jump14byteInstall);
		this->m_location = targetFunction;

		this->m_originalFunction.m_function = RawAlloc<char>(sliceSize);
		this->m_originalFunction.m_size = sliceSize;
		memcpy(this->m_originalFunction.m_function, targetFunction, sliceSize);

		if (allocationMethod == AllocationMethod::FarAlloc)
			m_trampoline = RawRWEAlloc<uint8_t>(trampolineSize, alignment);
		else if (allocationMethod == AllocationMethod::NearAlloc)
			SetToRWE<uint8_t>(RawAlineAlloc<uint8_t>(trampolineSize, alignment), trampolineSize);

		ASMHandler trampolineHandler((uint8_t*)m_trampoline, trampolineSize);
		memcpy(m_trampoline, targetFunction, sliceSize);
		trampolineHandler.seekg(sliceSize);
		jmp64 jmp(reinterpret_cast<uintptr_t>(targetFunction) + sliceSize);
		trampolineHandler.Write(jmp);
		return true;
	}
	bool Hook::IsBigEnough(UnkfuncPtr targetFunction, size_t size)
	{
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