#include "Draft.hpp"

[[nodiscard]] short getFive() {
	return 5;
}

[[nodiscard]] short get10() {
	return 10;
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
	Draft::Hook hook;
	void* func = funcRefToReallLocation(functionPointerToVoidPointer(&getFive));
	void* redirect = funcRefToReallLocation(functionPointerToVoidPointer(&get10));
	short(*ptr)() = &getFive;
	std::cout << "Original: " << ptr() << std::endl;
	if (!hook.Install(func, redirect, Draft::HookInstallMethod::JumpAutomaticInstall, Draft::AllocationMethod::NearAlloc, 0))
	{
		std::cout << "Failed to install hook" << std::endl;
		return 1;
	}
	std::cout << "Hooked: " << ptr() << std::endl;
	hook.Uninstall();
	return 0;
} 