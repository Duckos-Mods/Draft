#pragma once
#include <cstdint>


#define NewRegisterDef(registerName, regLowName, regHighName) \
	union registerName \
	{ \
		uint64_t value; \
		struct \
		{ \
			uint32_t regLowName; \
			uint32_t regHighName; \
		}; \
	}
namespace Draft
{
    // All the registers that are used in x86-64
    enum class Registers
    {
        RAX,
        RBX,
        RCX,
        RDX,
        RSI,
        RDI,
        RBP,
        RSP,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        REGISTERS_COUNT
    };

    union Register64Bit
    {
        uint64_t value;
        struct 
        {
            uint32_t low;
			uint32_t high;

        };
    };

    class CPURegRepresentation
    {
    public:
        NewRegisterDef(RAX, EAX, EDX);
        NewRegisterDef(RBX, EBX, EBX);
		NewRegisterDef(RCX, ECX, ECX);
		NewRegisterDef(RDX, EDX, EDX);
		NewRegisterDef(RSI, ESI, ESI);
		NewRegisterDef(RDI, EDI, EDI);
		NewRegisterDef(RBP, EBP, EBP);
		NewRegisterDef(RSP, ESP, ESP);
    
		NewRegisterDef(R8, R8D, R8D);
		NewRegisterDef(R9, R9D, R9D);
		NewRegisterDef(R10, R10D, R10D);
		NewRegisterDef(R11, R11D, R11D);
		NewRegisterDef(R12, R12D, R12D);
		NewRegisterDef(R13, R13D, R13D);
		NewRegisterDef(R14, R14D, R14D);
		NewRegisterDef(R15, R15D, R15D);

		static const char* GetRegisterName(Registers reg)
		{
			switch (reg)
			{
			case Registers::RAX:
				return "RAX";
			case Registers::RBX:
				return "RBX";
			case Registers::RCX:
				return "RCX";
			case Registers::RDX:
				return "RDX";
			case Registers::RSI:
				return "RSI";
			case Registers::RDI:
				return "RDI";
			case Registers::RBP:
				return "RBP";
			case Registers::RSP:
				return "RSP";
			case Registers::R8:
				return "R8";
			case Registers::R9:
				return "R9";
			case Registers::R10:
				return "R10";
			case Registers::R11:
				return "R11";
			case Registers::R12:
				return "R12";
			case Registers::R13:
				return "R13";
			case Registers::R14:
				return "R14";
			case Registers::R15:
				return "R15";
			default:
				return "Unknown";
			}
		}

    };


}