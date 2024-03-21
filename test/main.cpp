#include "Draft.hpp"
#include <intrin.h>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

Draft::Hook hook;
[[nodiscard]] int printHooked(int i)
{
	auto data = hook.fastCall<int, int>(i*10);
	return data;
}

[[nodiscard]] int printRandom(int i)
{
	return 1000 + i;
}


int main() {

	auto ptr = printRandom;
	std::cout << "Before hooking: " << ptr(10) << "\n";

	hook.Install(Draft::LocalFunctionPointerToRawAddress(ptr), Draft::LocalFunctionPointerToRawAddress(printHooked));
	std::cout << "After hooking: " << ptr(10) << "\n";

	hook.Uninstall();

	std::cout << "After unhooking: " << ptr(10) << "\n";

	return 0;
} 