#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    std::cout << "X2Modern - Twitter Bot Application" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    std::cout << "Platform: Windows" << std::endl;
    std::cout << "Build: Cross-compiled on Linux" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Note: This is a console version." << std::endl;
    std::cout << "Full GUI version requires Qt5 runtime on Windows." << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
