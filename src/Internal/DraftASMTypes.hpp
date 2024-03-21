#pragma once
#include <cstdint>

namespace Draft
{
    enum class Opcodes : uint8_t
    {
        RET = 0xC3,
        MOV = 0xC7,
        PUSH = 0x68,
        JMP = 0xE9,


        JMP64_MOV_MODRM = 0x44,
        JMP64_MOV_SIB = 0x24,
        JMP64_MOV_OFFSET = 0x4,
    };

    uint8_t constexpr OpToByte(Opcodes op)
    {
		return static_cast<uint8_t>(op);
	}

#pragma pack(push, 1)
    struct jmp32 {
        uint8_t opcode = OpToByte(Opcodes::JMP);
        int32_t offset;

        jmp32(int32_t _offset) {
            offset = _offset;
		}
    };

    struct jmp64 {
        uint8_t opcodeFirst = 0xFF;
        uint8_t opcodeSecond = 0x25;
        uint32_t offset = 0;
        uint64_t address;

        jmp64(uint64_t _address) {
			address = _address;
		}

    };

    struct ret64 {
        uint8_t  pushOpcode = OpToByte(Opcodes::PUSH);
        uint32_t pushLowAddressSlice = 0;
        uint8_t  movOpcode = OpToByte(Opcodes::MOV);
        uint8_t  movModRm = OpToByte(Opcodes::JMP64_MOV_MODRM);
        uint8_t  movSib = OpToByte(Opcodes::JMP64_MOV_SIB);
        uint8_t  movOffset = OpToByte(Opcodes::JMP64_MOV_OFFSET);
        uint32_t movHighAddressSlice = 0;
        uint8_t  retOpcode = OpToByte(Opcodes::RET);

        ret64(uintptr_t address) {
            pushLowAddressSlice = static_cast<uint32_t>(address & 0xFFFFFFFF);
            movHighAddressSlice = static_cast<uint32_t>((address >> 32) & 0xFFFFFFFF);
        }

    };

#pragma pack(pop)

} // namespace Draft
