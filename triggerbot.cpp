#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>

// Config
const COLORREF TARGET_COLOR = 5197761;

// Globals
std::atomic<bool> running(false);
std::atomic<bool> alive(true);
std::atomic<bool> rightMouseDown(false);
HDC dc;
int centerX, centerY;
char exePath[MAX_PATH];

// ULTRA FAST pixel check - Direct inline, minimal overhead
void CheckAndClick() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    
    const COLORREF target = TARGET_COLOR;
    COLORREF color;
    
    while (alive) {
        if (running && rightMouseDown) {
            // Direct GetPixel call - no function overhead
            color = GetPixel(dc, centerX, centerY);
            
            if (color == target) {
                // Instant click - no delay
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
        }
        // Minimal sleep to prevent 100% CPU lag
        Sleep(0);  // Yields thread but returns immediately
    }
}

// Self-delete function
void SelfDestruct() {
    std::cout << "\n[SELF-DELETE] Erasing...\n";
    Sleep(300);
    
    char batchPath[MAX_PATH];
    GetTempPathA(MAX_PATH, batchPath);
    strcat(batchPath, "~del.bat");
    
    FILE* batch = fopen(batchPath, "w");
    if (batch) {
        fprintf(batch, "@echo off\n");
        fprintf(batch, "timeout /t 1 /nobreak >nul\n");
        fprintf(batch, ":loop\n");
        fprintf(batch, "del /f /q \"%s\" >nul 2>&1\n", exePath);
        fprintf(batch, "if exist \"%s\" goto loop\n", exePath);
        fprintf(batch, "del /f /q \"%%~f0\" >nul 2>&1\n");
        fclose(batch);
        
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        CreateProcessA(NULL, batchPath, NULL, NULL, FALSE, 
                      CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    alive = false;
}

// Mouse hook
LRESULT CALLBACK MouseProc(int code, WPARAM wp, LPARAM lp) {
    if (code >= 0) {
        if (wp == WM_RBUTTONDOWN) rightMouseDown = true;
        if (wp == WM_RBUTTONUP) rightMouseDown = false;
    }
    return CallNextHookEx(0, code, wp, lp);
}

// Keyboard hook
LRESULT CALLBACK KeyProc(int code, WPARAM wp, LPARAM lp) {
    if (code >= 0 && wp == WM_KEYDOWN) {
        auto* kb = (KBDLLHOOKSTRUCT*)lp;
        
        // INSERT = Toggle
        if (kb->vkCode == VK_INSERT) {
            running = !running;
            Beep(running ? 1000 : 500, 50);
            std::cout << (running ? "\n[ACTIVE]" : "\n[PAUSED]") << std::flush;
        }
        
        // DELETE = Self-Delete
        if (kb->vkCode == VK_DELETE) {
            static bool confirmDelete = false;
            if (!confirmDelete) {
                std::cout << "\n[DELETE] Press DELETE again to confirm!\n";
                confirmDelete = true;
                std::thread([&]() {
                    Sleep(2000);
                    confirmDelete = false;
                }).detach();
            } else {
                SelfDestruct();
            }
        }
        
        // END = Exit
        if (kb->vkCode == VK_END) {
            alive = false;
            Beep(800, 100);
        }
    }
    return CallNextHookEx(0, code, wp, lp);
}

int main() {
    // Get exe path
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    // Setup - HIGH_PRIORITY instead of REALTIME (prevents lags)
    dc = GetDC(0);
    centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    
    std::cout << "\n";
    std::cout << " ##     ## ##    ## ########   ########\n";
    std::cout << " ##     ##  ##  ##  ##     ## ##     ##\n";
    std::cout << " ##     ##   ####   ##     ## ##     ##\n";
    std::cout << " #########    ##    ########  ########\n";
    std::cout << " ##     ##    ##    ##   ##   ##     ##\n";
    std::cout << " ##     ##    ##    ##    ##  ##     ##\n";
    std::cout << " ##     ##    ##    ##     ## ########\n";
    std::cout << "\n";
    std::cout << "============================================\n";
    std::cout << "         UPLOAD LABS - MAX SPEED\n";
    std::cout << "============================================\n";
    std::cout << "  INSERT    = Toggle On/Off\n";
    std::cout << "  DELETE    = Self-Delete (2x confirm)\n";
    std::cout << "  END       = Exit\n";
    std::cout << "============================================\n";
    std::cout << "  Color: #" << std::hex << TARGET_COLOR << std::dec << "\n";
    std::cout << "  Speed: Sleep(0) - Instant Detection\n";
    std::cout << "============================================\n\n";
    std::cout << "[PAUSED] Press INSERT to activate...\n";
    
    // Hooks
    auto mHook = SetWindowsHookExA(WH_MOUSE_LL, MouseProc, 0, 0);
    auto kHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyProc, 0, 0);
    
    if (!mHook || !kHook) {
        std::cerr << "Hook installation failed!\n";
        return 1;
    }
    
    // Start detection thread
    std::thread worker(CheckAndClick);
    
    // Message loop
    MSG msg;
    while (alive && GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    alive = false;
    if (worker.joinable()) worker.join();
    UnhookWindowsHookEx(mHook);
    UnhookWindowsHookEx(kHook);
    ReleaseDC(0, dc);
    
    std::cout << "\n[EXIT] Goodbye!\n";
    return 0;
}
