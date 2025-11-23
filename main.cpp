#include "magicwound.h"
#ifdef _WIN32
#include <windows.h>
#endif

class UTF8Console {
public:
    UTF8Console() {
#ifdef _WIN32
        // Windows设置
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        
        // 修复Windows控制台字体问题
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleOutputCP(65001);
#else
        // Linux/macOS设置
        std::locale::global(std::locale("en_US.UTF-8"));
        std::cout.imbue(std::locale());
        std::wcout.imbue(std::locale());
#endif
    }
    
    ~UTF8Console() {
        // 清理代码（如果需要）
    }
};

// 全局UTF-8控制台设置
UTF8Console utf8_console;

int main() {
    GameManager game;
    game.run();
    return 0;
}
