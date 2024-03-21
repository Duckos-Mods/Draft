#include "DraftASM.hpp"
#undef min
#undef max
#include <zasm/decoder.h>

namespace Draft {

	

	size_t ASMHandler::GetMinInstrSizeForMinsize(void* address, size_t minSize)
	{
        size_t size = 0;
		char* addr = static_cast<char*>(address);
		zasm::Decoder decoder(zasm::MachineMode::AMD64);
		while (size < minSize)
		{
			zasm::Instruction instr;
			const auto res = decoder.decode(addr + size, 0xFF, (uint64_t)addr);
			if (!res)
				DRAFT_THROW("Failed to decode instruction");
			const auto& instrInfo = *res;
			size += instrInfo.getLength();
		}
		return size;
	}


}
