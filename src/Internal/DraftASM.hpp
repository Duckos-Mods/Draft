#pragma once
#include "DraftUtils.hpp"
#include <array>
#include <span>

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
        uint32_t offset;

        jmp32(int32_t _offset) {
            offset = _offset;
		}
    };



    struct jmp64 {
        uint8_t  pushOpcode = OpToByte(Opcodes::PUSH);
        uint32_t pushLowAddressSlice = 0;
        uint8_t  movOpcode = OpToByte(Opcodes::MOV);
        uint8_t  movModRm = OpToByte(Opcodes::JMP64_MOV_MODRM);
        uint8_t  movSib = OpToByte(Opcodes::JMP64_MOV_SIB);
        uint8_t  movOffset = OpToByte(Opcodes::JMP64_MOV_OFFSET);
        uint32_t movHighAddressSlice = 0;
        uint8_t  retOpcode = OpToByte(Opcodes::RET);

        jmp64(uintptr_t address) {
            pushLowAddressSlice = static_cast<uint32_t>(address & 0xFFFFFFFF);
            movHighAddressSlice = static_cast<uint32_t>((address >> 32) & 0xFFFFFFFF);
        }

    };

#pragma pack(pop)

    class ASMHandler
    {
    public:
        ASMHandler(uint8_t* code, size_t size) : m_code(code), m_size(size) {}

        void Write(uint8_t* code, size_t size)
        {
			if (m_offset + size > m_size)
				DRAFT_THROW("Buffer overflow");

			memcpy(m_code + m_offset, code, size);
			m_offset += size;
		}

        template<typename T>
        void Write(T& value)
        {
			Write(reinterpret_cast<uint8_t*>(&value), sizeof(T));
        }

        size_t disassemble(std::span<uint8_t> code, size_t minSize);


         size_t tellg() const { return m_offset; }
         void seekg(size_t offset) { m_offset = offset; }


         void WriteToBuffer(uint8_t* buffer, size_t size) const
         {
			 if (size > m_size)
				 DRAFT_THROW("Buffer overflow");

			 memcpy(buffer, m_code, size);
		 }

    private:
        uint8_t* m_code = nullptr;
        size_t m_size = 0;
        size_t m_offset = 0;
    };
}