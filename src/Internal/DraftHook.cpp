#include "DraftHook.hpp"
#include "DraftASM.hpp"
#undef min
#undef max
#include <zasm/zasm.hpp>
#include <zasm/x86/x86.hpp>
#include <zasm/x86/assembler.hpp>
using namespace zasm;

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

		/**
		* We arnt using ZASM here because we arent doing anything fancy with the instructions
		*/

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
		m_isInstalled = true;
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
	
	// Tests

	static void* allocatePage(std::size_t codeSize)
	{
#ifdef _WIN32
		return VirtualAlloc(nullptr, codeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else
		// TODO: mmap for Linux.
		return nullptr;
#endif
	}

	// I actually have no idea if thats enough space cuz we do a lot of ASM stuff
#define DRAFT_ALLOC_SIZE 0x400 
	
	bool InlineHook::Install(UnkfuncPtr targetAddress, InlineHookFunction hookFunction, AllocationMethod allocationMethod, size_t alignment)
	{
		DRAFT_HANDLE_ERR(IsInstalled(), return false);

		size_t size = EnumToUnderlying<HookInstallMethod, size_t>(HookInstallMethod::Jump14byteInstall); // CBA to deal with the other sizes
		ASMHandler temp(nullptr, 0);
		size = temp.GetMinInstrSizeForMinsize(targetAddress, size);
		SetToRWE(targetAddress, size);
		GenerateOriginalFunction(targetAddress, size);
		void* customJITFunction = nullptr;
		if (allocationMethod == AllocationMethod::FarAlloc)
			customJITFunction = allocatePage(DRAFT_ALLOC_SIZE);
		else if (allocationMethod == AllocationMethod::NearAlloc)
			SetToRWE<char>(RawAlineAlloc<char>(DRAFT_ALLOC_SIZE, alignment), DRAFT_ALLOC_SIZE);
		else
			return false;
		memset(customJITFunction, 0xCC, DRAFT_ALLOC_SIZE);
		this->m_customJITFunction.m_function = customJITFunction;
		this->m_customJITFunction.m_size = DRAFT_ALLOC_SIZE;


		zasm::Program program(zasm::MachineMode::AMD64);
		zasm::x86::Assembler a(program);
		zasm::Label label = a.createLabel();
		// push all registers to a variable
		a.bind(label);
		uint64_t* CPUInfo = (uint64_t*)(void*)&this->m_regs; 
		// A lot of ASM stuff
		
		// Saves the contents of the registers
		{
			a.push(x86::rsp); // push the trampoline RSP
			a.push(x86::rsp); // push the original RSP
			a.push(x86::rax); // Push RAX
			a.mov(x86::rax, uint64_t(++CPUInfo)); // Move the address of the CPUInfo to RAX
			for (size_t regID = ZYDIS_REGISTER_RBX; regID <= ZYDIS_REGISTER_R15; regID++)
			{
				x86::Gp64 reg(static_cast<x86::Reg::Id>(regID));
				a.mov(x86::qword_ptr(x86::rax), reg);
				a.add(x86::rax, 8);
			}
		}
		
		// handle EFLAGS
		/*{
			uint64_t EFLAGSPosition = (uint64_t)&this->m_regs.EFLAGS;
			a.mov(x86::rax, EFLAGSPosition);
			a.pushfq();
			a.pop(x86::rcx);
			a.mov(x86::qword_ptr(x86::rax), x86::rcx);
		}*/

		// Save the float registers
		{
			uint64_t FloatPosition = (uint64_t)&this->m_regs.XMM0;
			a.mov(x86::rax, FloatPosition);
			for (size_t regID = ZYDIS_REGISTER_XMM0; regID <= ZYDIS_REGISTER_XMM15; regID++)
			{
				x86::Xmm reg(static_cast<x86::Reg::Id>(regID));
				a.movq(x86::qword_ptr(x86::rax), reg);
				a.add(x86::rax, 16);
			}
		}

		// Save RAX
		{
			a.pop(x86::rbx); // Pop the original RAX
			a.mov(x86::rax, uint64_t(--CPUInfo)); // Move the address of the CPUInfo to RAX
			a.mov(x86::qword_ptr(x86::rax), x86::rbx); // Save RAX
		}

		//Fix RSP
		{
			a.pop(x86::rax); // Pop the original RSP
			a.add(x86::rax, 16); // Add 16 to the RSP
			a.push(x86::rax); // Push the new RSP
		}

		// align stack, save original on the stack
		{
			a.push(x86::rsp);
			a.sub(x86::rsp, 48);
			a.and_(x86::rsp, -16);
		}

		// Call the hook function
		{
			uint64_t CPUInfoPosition = (uint64_t)&this->m_regs;
			a.mov(x86::rdi, CPUInfoPosition);
			a.mov(x86::rax, (uint64_t)hookFunction);
			a.call(x86::rax);
		}
		
		
		
		Serializer serializer{};
		auto res = serializer.serialize(program, (uint64_t)this->m_customJITFunction.m_function);
		DRAFT_HANDLE_ERR(res != Error::None, 
			return false);

		const auto* sect = serializer.getSectionInfo(0); // Only one section
		const auto* data = serializer.getCode() + sect->offset;
		DRAFT_HANDLE_ERR(sect->physicalSize > DRAFT_ALLOC_SIZE, return false);
		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(this->m_customJITFunction.m_function), DRAFT_ALLOC_SIZE);
		asmHandler.Write(data, sect->physicalSize);
		asmHandler.Write((const uint8_t*)this->m_originalFunction.m_function, this->m_originalFunction.m_size);
		jmp64 jmp(reinterpret_cast<uintptr_t>(targetAddress) + this->m_originalFunction.m_size);
		asmHandler.Write(jmp);
		SetProtection(this->m_customJITFunction.m_function, DRAFT_ALLOC_SIZE, MemoryProtection::Execute);

		ASMHandler targetHandler(reinterpret_cast<uint8_t*>(targetAddress), DRAFT_ALLOC_SIZE);
		jmp64 jmpToCustom(reinterpret_cast<uintptr_t>(this->m_customJITFunction.m_function));
		targetHandler.Write(jmpToCustom);
		SetProtection(targetAddress, DRAFT_ALLOC_SIZE, MemoryProtection::Execute);

		m_isInstalled = true;
		return true;







	}
	bool InlineHook::GenerateOriginalFunction(UnkfuncPtr targetAddress, size_t size)
	{
		ASMHandler asmHandler(reinterpret_cast<uint8_t*>(targetAddress), size * 30);
		size_t sliceSize = size;
		this->m_originalFunction.m_function = RawAlloc<char>(sliceSize);
		this->m_originalFunction.m_size = sliceSize;
		memcpy(this->m_originalFunction.m_function, targetAddress, sliceSize);
		return true;
	}
}