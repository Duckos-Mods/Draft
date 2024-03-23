#pragma once
#include <cstdint>
#include <array>
#include "DraftUtils.hpp"
#include <array>


#define NewRegisterDef(registerName, regLowName, regHighName) \
	union \
	{ \
		uint64_t registerName; \
		struct \
		{ \
			uint32_t regLowName; \
			uint32_t regHighName; \
		}; \
	}
namespace Draft
{
	// Float shenanigans
	using uint128_t = uint64_t[2];

    // All the registers that are used in x86-64
    enum class Registers : uint16_t
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
		uint64_t value = 0;
        struct 
        {
            uint32_t low;
			uint32_t high;

        };
    };

    class CPURegRepresentation
    {
	public:
		union {
			uint64_t RAX = 1; struct {
				uint32_t EAX; uint32_t EDX;
			};
		};
		union {
			uint64_t RBX = 2; struct {
				uint32_t EBX;
			};
		};
		union {
			uint64_t RCX = 3; struct {
				uint32_t ECX;
			};
		};
		union {
			uint64_t RDX = 4; struct {
				uint32_t EDX;
			};
		};
		union {
			uint64_t RSI = 5; struct {
				uint32_t ESI;
			};
		};
		union {
			uint64_t RDI = 6; struct {
				uint32_t EDI;
			};
		};
		union {
			uint64_t RBP = 7; struct {
				uint32_t EBP;
			};
		};
		union {
			uint64_t RSP = 8; struct {
				uint32_t ESP;
			};
		};
		union {
			uint64_t R8 = 9; struct {
				uint32_t R8D;
			};
		};
		union {
			uint64_t R9 = 10; struct {
				uint32_t R9D;
			};
		};
		union {
			uint64_t R10 = 11; struct {
				uint32_t R10D;
			};
		};
		union {
			uint64_t R11 = 12; struct {
				uint32_t R11D;
			};
		};
		union {
			uint64_t R12 = 13; struct {
				uint32_t R12D;
			};
		};
		union {
			uint64_t R13 = 14; struct {
				uint32_t R13D;
			};
		};
		union {
			uint64_t R14 = 15; struct {
				uint32_t R14D;
			};
		};
		union {
			uint64_t R15 = 16; struct {
				uint32_t R15D;
			};
		};
		// Floating point registers
		union 
		{
			uint128_t XMM0;
			std::array<float, 4> XMM0F;
			std::array<double, 2> XMM0D;
		};
		union
		{
			uint128_t XMM1;
			std::array<float, 4> XMM1F;
			std::array<double, 2> XMM1D;
		};
		union
		{
			uint128_t XMM2;
			std::array<float, 4> XMM2F;
			std::array<double, 2> XMM2D;
		};
		union
		{
			uint128_t XMM3;
			std::array<float, 4> XMM3F;
			std::array<double, 2> XMM3D;
		};
		union
		{
			uint128_t XMM4;
			std::array<float, 4> XMM4F;
			std::array<double, 2> XMM4D;
		};
		union
		{
			uint128_t XMM5;
			std::array<float, 4> XMM5F;
			std::array<double, 2> XMM5D;
		};
		union
		{
			uint128_t XMM6;
			std::array<float, 4> XMM6F;
			std::array<double, 2> XMM6D;
		};
		union
		{
			uint128_t XMM7;
			std::array<float, 4> XMM7F;
			std::array<double, 2> XMM7D;
		};
		union
		{
			uint128_t XMM8;
			std::array<float, 4> XMM8F;
			std::array<double, 2> XMM8D;
		};
		union
		{
			uint128_t XMM9;
			std::array<float, 4> XMM9F;
			std::array<double, 2> XMM9D;
		};
		union
		{
			uint128_t XMM10;
			std::array<float, 4> XMM10F;
			std::array<double, 2> XMM10D;
		};
		union
		{
			uint128_t XMM11;
			std::array<float, 4> XMM11F;
			std::array<double, 2> XMM11D;
		};
		union
		{
			uint128_t XMM12;
			std::array<float, 4> XMM12F;
			std::array<double, 2> XMM12D;
		};
		union
		{
			uint128_t XMM13;
			std::array<float, 4> XMM13F;
			std::array<double, 2> XMM13D;
		};
		union
		{
			uint128_t XMM14;
			std::array<float, 4> XMM14F;
			std::array<double, 2> XMM14D;
		};
		union
		{
			uint128_t XMM15;
			std::array<float, 4> XMM15F;
			std::array<double, 2> XMM15D;
		};

		// EFLAGS and RFAGS
		uint32_t EFLAGS = 0;
		uint64_t RFLAGS = 0;


	public:
		CPURegRepresentation() = default;
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