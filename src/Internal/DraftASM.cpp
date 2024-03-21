#include "DraftASM.hpp"


namespace Draft {

	

	size_t ASMHandler::GetMinInstrSizeForMinsize(void* address, size_t minSize)
	{
        size_t size = 0;
        while (size < minSize)
		{
			size += GetInstrSize(address);
			address = (void*)((size_t)address + size);
		}
		return size;
	}

    // All the code below is taken almost as is from https://github.com/SamuelTulach/LightHook because it's a good code and I don't want to reinvent the wheel
    
    #define HOOK_R (*b >> 4)
    #define HOOK_C (*b & 0xF)

	static const unsigned char PREFIXES[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	static const unsigned char OP1_MODRM[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	static const unsigned char OP1_IMM8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	static const unsigned char OP1_IMM32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	static const unsigned char OP2_MODRM[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

    static int FindByte(const unsigned char* buffer, const unsigned long long maxLength, const unsigned char value)
    {
        for (unsigned long long i = 0; i < maxLength; i++)
        {
            if (buffer[i] == value)
                return 1;
        }

        return 0;
    }
    static void ParseModRM(unsigned char** buffer, const int addressPrefix)
    {
        const unsigned char modRm = *++ * buffer;

        if (!addressPrefix || (addressPrefix && **buffer >= 0x40))
        {
            int hasSib = 0;
            if (**buffer < 0xC0 && (**buffer & 0b111) == 0b100 && !addressPrefix)
                hasSib = 1, (*buffer)++;

            if (modRm >= 0x40 && modRm <= 0x7F)
                (*buffer)++;
            else if ((modRm <= 0x3F && (modRm & 0b111) == 0b101) || (modRm >= 0x80 && modRm <= 0xBF))
                *buffer += (addressPrefix) ? 2 : 4;
            else if (hasSib && (**buffer & 0b111) == 0b101)
                *buffer += (modRm & 0b01000000) ? 1 : 4;
        }
        else if (addressPrefix && modRm == 0x26)
            *buffer += 2;
    }
    static int GetInstructionSize(const void* address)
    {
        /*
         * Based on length-disassembler by @Nomade040
         * https://github.com/Nomade040/length-disassembler
         */

        unsigned long long offset = 0;
        int operandPrefix = 0, addressPrefix = 0, rexW = 0;
        unsigned char* b = (unsigned char*)address;

        for (int i = 0; i < 14 && FindByte(PREFIXES, sizeof(PREFIXES), *b) || HOOK_R == 4; i++, b++)
        {
            if (*b == 0x66)
                operandPrefix = 1;
            else if (*b == 0x67)
                addressPrefix = 1;
            else if (HOOK_R == 4 && HOOK_C >= 8)
                rexW = 1;
        }

        if (*b == 0x0F)
        {
            b++;
            if (*b == 0x38 || *b == 0x3A)
            {
                if (*b++ == 0x3A)
                    offset++;

                ParseModRM(&b, addressPrefix);
            }
            else
            {
                if (HOOK_R == 8)
                    offset += 4;
                else if ((HOOK_R == 7 && HOOK_C < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC)
                    offset++;

                if (FindByte(OP2_MODRM, sizeof(OP2_MODRM), *b) || (HOOK_R != 3 && HOOK_R > 0 && HOOK_R < 7) || *b >= 0xD0 || (HOOK_R == 7 && HOOK_C != 7) || HOOK_R == 9 || HOOK_R == 0xB || (HOOK_R == 0xC && HOOK_C < 8) || (HOOK_R == 0 && HOOK_C < 4))
                    ParseModRM(&b, addressPrefix);
            }
        }
        else
        {
            if ((HOOK_R == 0xE && HOOK_C < 8) || (HOOK_R == 0xB && HOOK_C < 8) || HOOK_R == 7 || (HOOK_R < 4 && (HOOK_C == 4 || HOOK_C == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || FindByte(OP1_IMM8, sizeof(OP1_IMM8), *b))
                offset++;
            else if (*b == 0xC2 || *b == 0xCA)
                offset += 2;
            else if (*b == 0xC8)
                offset += 3;
            else if ((HOOK_R < 4 && (HOOK_C == 5 || HOOK_C == 0xD)) || (HOOK_R == 0xB && HOOK_C >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || FindByte(OP1_IMM32, sizeof(OP1_IMM32), *b))
                offset += (rexW) ? 8 : (operandPrefix ? 2 : 4);
            else if (HOOK_R == 0xA && HOOK_C < 4)
                offset += (rexW) ? 8 : (addressPrefix ? 2 : 4);
            else if (*b == 0xEA || *b == 0x9A)
                offset += operandPrefix ? 4 : 6;

            if (FindByte(OP1_MODRM, sizeof(OP1_MODRM), *b) || (HOOK_R < 4 && (HOOK_C < 4 || (HOOK_C >= 8 && HOOK_C < 0xC))) || HOOK_R == 8 || (HOOK_R == 0xD && HOOK_C >= 8))
                ParseModRM(&b, addressPrefix);
        }

        return (int)(++b + offset - (unsigned char*)address);
    }

	size_t ASMHandler::GetInstrSize(void* address)
	{
		return (size_t)GetInstructionSize(address);
	}

}
