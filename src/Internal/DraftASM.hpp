#pragma once
#include "DraftUtils.hpp"
#include "DraftASMTypes.hpp"
#include <array>
#include <span>

namespace Draft
{
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

        size_t GetMinInstrSizeForMinsize(void* address, size_t minSize);
        size_t GetInstrSize(void* address);


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