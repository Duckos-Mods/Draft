#include "Draft.hpp"
#include <intrin.h>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

Draft::InlineHook inlineHook;
Draft::CPURegRepresentation regData;
[[nodiscard]] void printHooked(Draft::CPURegRepresentation* registers)
{
	for (size_t i = (size_t)&regData; i < (size_t)&regData + sizeof(Draft::CPURegRepresentation); i++)
	{
		*(char*)i = *(char*)registers;
		registers = (Draft::CPURegRepresentation*)((size_t)registers + 1);
	}
}

[[nodiscard]] int printRandom(int i)
{
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	i += 20;
	std::cout << i << std::endl;
	i += 20;
	std::cout << i << std::endl;
	i += 20;
	std::cout << i << std::endl;
	i += 20;
	std::cout << i << std::endl;
	i += 20;
	std::cout << i << std::endl;
	i += 20;

	return i;
}

#ifdef _DEBUG
#define offset 0x4B
#endif
int main() {
	void* printRandomPtr = Draft::LocalFunctionPointerToRawAddress(printRandom);
	char* printRandomPtrChar = (char*)printRandomPtr;
	printRandomPtrChar += offset;
	printRandomPtr = (void*)printRandomPtrChar;
	Draft::CPURegRepresentation registers;
	printHooked(&registers);

	std::cout << "Pre hook printRandomPtr: " << printRandom(1) << std::endl;

	inlineHook.Install(printRandomPtr, printHooked);

	std::cout << "Post hook printRandomPtr: " << printRandom(1) << std::endl;

	std::cout << "RAX: " << registers.RAX << std::endl;
	std::cout << "RBX: " << registers.RBX << std::endl;
	std::cout << "RCX: " << registers.RCX << std::endl;
	std::cout << "RDX: " << registers.RDX << std::endl;
	std::cout << "R8: " << registers.R8 << std::endl;
	std::cout << "R9: " << registers.R9 << std::endl;
	std::cout << "R10: " << registers.R10 << std::endl;
	std::cout << "R11: " << registers.R11 << std::endl;
	std::cout << "R12: " << registers.R12 << std::endl;
	std::cout << "R13: " << registers.R13 << std::endl;
	std::cout << "R14: " << registers.R14 << std::endl;
	std::cout << "R15: " << registers.R15 << std::endl;
	std::cout << "RDI: " << registers.RDI << std::endl;
	std::cout << "RSI: " << registers.RSI << std::endl;
	std::cout << "RBP: " << registers.RBP << std::endl;
	std::cout << "RSP: " << registers.RSP << std::endl;

	system("pause");


	return 0;
} 