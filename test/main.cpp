#include "Draft.hpp"
#include <intrin.h>

Draft::Hook hook;
[[nodiscard]] int printHooked(int i)
{
	std::cout << "Hooked" << std::endl;
	std::cout << "Hooked" << std::endl;
	std::cout << "Hooked" << std::endl;
	std::cout << "Hooked" << std::endl;

	auto data = hook.fastCall<int, int>(i*10);
	std::cout << "Return value: " << data << std::endl;
	return data * 1000;
}

[[nodiscard]] int printRandom(int i)
{
	std::cout << "Random" << std::endl;
	std::cout << "Random" << std::endl;
	std::cout << "Random" << std::endl;
	std::cout << "Random" << std::endl;
	return 1000 + i;
}

void* funcRefToReallLocation(void* pFunction) {
	char* ptr = reinterpret_cast<char*>(pFunction);
	ptr++;
	int32_t offset = *reinterpret_cast<int32_t*>(ptr);
	ptr--;
	uint64_t target = ((uint64_t)ptr + offset);
	while (target % 16 != 0) {
		target++;
	}
	return reinterpret_cast<void*>(target);
}

template<typename T>
void* functionPointerToVoidPointer(T pFunction) {
	return *reinterpret_cast<void**>(&pFunction);
}

int main() {
	void* pHooked = funcRefToReallLocation(printHooked);
	void* pRandom = funcRefToReallLocation(printRandom);
	std::cout << "Before hook\n"<< printRandom(100) << std::endl;
	
	hook.Install(pRandom, printHooked);
	std::cout << "After hook\n"<< printRandom(100) << std::endl;
	hook.Uninstall();
	std::cout << "After uninstall\n"<< printRandom(100) << std::endl;
	return 0;
} 