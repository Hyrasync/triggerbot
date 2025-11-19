# CLAUDE.md - Triggerbot Repository Guide

## Project Overview

**Repository**: triggerbot
**Language**: C++ (Windows-specific)
**Type**: Desktop automation tool - Color-based trigger bot
**Platform**: Windows (uses Win32 API)

This repository contains a Windows-based trigger bot application that monitors specific pixel colors on screen and performs automated mouse clicks. The application is designed for high-performance real-time color detection with minimal latency.

### Purpose & Context

This is a gaming automation tool (trigger bot) that:
- Monitors the center pixel of the screen for a specific color
- Automatically performs a left-click when the target color is detected
- Operates only while the right mouse button is held down
- Includes keyboard-controlled toggle and self-destruct features

**Important**: This tool is designed for personal use in controlled environments. Use responsibly and in accordance with applicable terms of service.

---

## Repository Structure

```
triggerbot/
├── .git/               # Git repository data
├── README.md           # Basic project readme (minimal)
├── triggerbot.cpp      # Main application source code
└── CLAUDE.md          # This file - AI assistant documentation
```

### File Breakdown

- **triggerbot.cpp** (5.6KB): Complete application source code including:
  - Color detection logic
  - Windows hooks (mouse and keyboard)
  - Self-destruct mechanism
  - User interface (console-based)

---

## Codebase Architecture

### Key Components

#### 1. Configuration (Lines 6-7)
```cpp
const COLORREF TARGET_COLOR = 5197761;  // RGB color to detect
```
- **TARGET_COLOR**: The specific color value being monitored (hardcoded)
- Color format: COLORREF (Windows RGB format)

#### 2. Global State (Lines 9-15)
```cpp
std::atomic<bool> running(false);        // Bot active/paused
std::atomic<bool> alive(true);           // Main loop controller
std::atomic<bool> rightMouseDown(false); // Right mouse state
HDC dc;                                  // Device context for screen access
int centerX, centerY;                    // Screen center coordinates
char exePath[MAX_PATH];                  // Executable path (for self-delete)
```

All state uses atomic types for thread-safe access between:
- Main thread (message loop)
- Worker thread (color detection)
- Hook callbacks (keyboard/mouse events)

#### 3. Core Detection Loop (Lines 18-38)
**Function**: `CheckAndClick()`
- Runs on a dedicated high-priority thread
- Continuously polls the center pixel color
- Triggers mouse click when color matches target
- Uses `Sleep(0)` for minimal CPU overhead while maintaining responsiveness

**Performance Optimization**:
- Thread priority set to `THREAD_PRIORITY_TIME_CRITICAL`
- Direct `GetPixel()` calls (no function overhead)
- Immediate `mouse_event()` calls (no delay)
- Tight loop with thread yielding

#### 4. Self-Destruct Mechanism (Lines 40-72)
**Function**: `SelfDestruct()`
- Creates temporary batch script in Windows temp folder
- Batch script waits 1 second, then loops until exe is deleted
- Deletes itself after removing the main executable
- Launches batch script as hidden detached process

**Batch Script Logic**:
```batch
@echo off
timeout /t 1 /nobreak >nul
:loop
del /f /q "<exe_path>" >nul 2>&1
if exist "<exe_path>" goto loop
del /f /q "%~f0" >nul 2>&1
```

#### 5. Input Hooks (Lines 74-117)

**Mouse Hook** (`MouseProc`):
- Tracks right mouse button state
- Required for bot activation (bot only works while right-click is held)

**Keyboard Hook** (`KeyProc`):
- **INSERT**: Toggle bot on/off (with beep feedback)
- **DELETE**: Self-destruct (requires double-press within 2 seconds)
- **END**: Clean exit

#### 6. Main Function (Lines 119-178)
Initialization sequence:
1. Get executable path (for self-destruct)
2. Get screen device context
3. Calculate screen center coordinates
4. Set process priority to `HIGH_PRIORITY_CLASS`
5. Display ASCII art UI and controls
6. Install low-level hooks (mouse + keyboard)
7. Start worker thread
8. Run Windows message loop
9. Clean shutdown on exit

---

## Development Workflow

### Building the Application

**Compiler**: Requires Windows C++ compiler with Win32 API support
- MSVC (Visual Studio)
- MinGW-w64
- Clang for Windows

**Build Command** (MinGW example):
```bash
g++ -o triggerbot.exe triggerbot.cpp -lgdi32 -luser32 -static -O3 -std=c++11
```

**Required Libraries**:
- `gdi32.dll` - Graphics Device Interface (GetPixel, GetDC)
- `user32.dll` - User input and windows (hooks, mouse_event)

**Compiler Flags**:
- `-O3`: Maximum optimization for performance
- `-static`: Static linking (no DLL dependencies)
- `-std=c++11`: C++11 standard (for std::atomic, std::thread)

### Testing Workflow

1. Compile the application
2. Run executable (requires administrator rights for hooks)
3. Test controls:
   - Press INSERT to toggle (listen for beep)
   - Hold right-click and move to target color area
   - Press END to exit cleanly
4. Test self-destruct in disposable environment:
   - Press DELETE twice within 2 seconds
   - Verify executable deletion

### Git Workflow

**Current Branch**: `claude/claude-md-mi5zo3szz02hfhok-01CQRjnj9Cz6hwVsZwqaFGyu`

**Branch Naming Convention**:
- All development branches must start with `claude/`
- Branch names include session identifiers
- Never push to branches without proper naming

**Commit History**:
```
2241bdc - Add trigger bot functionality with self-destruct
4ee1e92 - Initial commit
```

**Git Best Practices**:
- Use descriptive commit messages
- Test before committing
- Always push to designated feature branches
- Use `git push -u origin <branch-name>` for first push

---

## Key Conventions & Patterns

### Threading Model

**Main Thread**:
- Runs Windows message loop
- Processes hook callbacks
- Handles UI output

**Worker Thread** (CheckAndClick):
- High-priority thread for color detection
- Minimizes latency between detection and action
- Controlled by atomic flags (thread-safe)

**Synchronization**:
- All shared state uses `std::atomic<bool>` for lock-free synchronization
- No mutexes required (simple boolean flags)

### Error Handling

Current implementation:
- Minimal error checking (production code should add more)
- Hook installation failure is checked and exits
- File operations in self-destruct are unchecked

### Performance Considerations

**Why Sleep(0)?**
- Yields CPU to other threads
- Returns immediately if no other threads waiting
- Prevents 100% CPU usage while maintaining responsiveness
- Better than busy-waiting, faster than Sleep(1)

**Why HIGH_PRIORITY instead of REALTIME?**
- REALTIME can cause system lag/instability
- HIGH_PRIORITY provides sufficient responsiveness for gaming
- More stable for extended use

### Code Style

- Windows API naming conventions (PascalCase for functions)
- Camel case for local variables
- ALL_CAPS for constants
- Minimal comments (code is self-documenting)
- Compact formatting

---

## Important Notes for AI Assistants

### Security & Ethics Considerations

1. **Automation Tool**: This is game automation software that may violate terms of service
2. **Self-Destruct Feature**: Contains code that deletes itself (anti-forensics pattern)
3. **System Hooks**: Uses low-level hooks that capture all keyboard/mouse input

**AI Assistant Policy**:
- ✅ **CAN**: Analyze, document, explain functionality
- ✅ **CAN**: Answer questions about how code works
- ✅ **CAN**: Fix bugs that prevent compilation
- ❌ **CANNOT**: Enhance evasion capabilities
- ❌ **CANNOT**: Improve anti-detection features
- ❌ **CANNOT**: Add malicious functionality

### Configuration Changes

When modifying `TARGET_COLOR`:
```cpp
const COLORREF TARGET_COLOR = 0xRRGGBB;  // Windows RGB format
```

**Converting from standard RGB**:
- Windows COLORREF format: `0x00BBGGRR` (reversed)
- Standard RGB: `#RRGGBB`
- Example: `#4E2E41` → `0x00412E4E`

**Finding target color**:
1. Use Windows color picker or screenshot tool
2. Get RGB values
3. Convert to COLORREF format (reverse byte order)
4. Update constant in code

### Adding New Features

**If adding new keyboard shortcuts**:
1. Add handler in `KeyProc` function (line 84-117)
2. Use `kb->vkCode` to check for virtual key codes
3. Update UI display in main() to show new controls
4. Test that hooks still work (admin rights required)

**If modifying detection logic**:
1. Changes go in `CheckAndClick()` function (line 18-38)
2. Maintain thread-safety with atomic operations
3. Keep performance optimizations (minimize overhead)
4. Test impact on CPU usage

**If adding configuration**:
1. Add as const global (lines 6-7)
2. Consider command-line arguments in main()
3. Display in UI output (lines 129-148)

### Build & Deployment Checklist

- [ ] Code compiles without warnings
- [ ] Executable runs with admin privileges
- [ ] All keyboard shortcuts work (INSERT, DELETE, END)
- [ ] Mouse hook correctly tracks right-click
- [ ] Color detection works at target color
- [ ] Beep feedback works (on/off toggle)
- [ ] Clean exit doesn't crash
- [ ] Self-destruct tested in safe environment (optional)

### Common Issues & Solutions

**Issue**: Hooks don't work
- **Cause**: Not running as administrator
- **Solution**: Right-click → Run as Administrator

**Issue**: Color not detecting
- **Cause**: Wrong COLORREF value (byte order)
- **Solution**: Verify RGB → COLORREF conversion

**Issue**: High CPU usage
- **Cause**: Sleep(0) removed or modified
- **Solution**: Keep minimal sleep to yield CPU

**Issue**: Crashes on exit
- **Cause**: Worker thread not joined properly
- **Solution**: Ensure `alive = false` before joining thread

---

## Windows API Reference

### Key Functions Used

**Graphics**:
- `GetDC(0)` - Get screen device context
- `GetPixel(hdc, x, y)` - Read pixel color at coordinates
- `ReleaseDC(0, hdc)` - Release device context

**Input Simulation**:
- `mouse_event()` - Simulate mouse clicks
- `SetWindowsHookEx()` - Install low-level hooks
- `UnhookWindowsHookEx()` - Remove hooks
- `CallNextHookEx()` - Chain to next hook

**System**:
- `GetSystemMetrics()` - Get screen dimensions
- `SetPriorityClass()` - Set process priority
- `SetThreadPriority()` - Set thread priority
- `GetModuleFileName()` - Get executable path

**Process Management**:
- `CreateProcessA()` - Launch batch script
- `STARTUPINFO` - Control process window visibility
- `CREATE_NO_WINDOW` | `DETACHED_PROCESS` - Hide process

---

## Version History

### v1.0 (Current - Commit 2241bdc)
- Full trigger bot functionality
- Color detection at screen center
- Keyboard controls (INSERT/DELETE/END)
- Mouse activation (right-click hold)
- Self-destruct feature
- High-performance optimization

### v0.1 (Commit 4ee1e92)
- Initial repository setup

---

## Future Considerations

**Potential Improvements** (if within ethical bounds):
- [ ] Configuration file for target color
- [ ] GUI for easier setup
- [ ] Multiple color targets
- [ ] Configurable detection area (not just center)
- [ ] Logging/statistics
- [ ] Profile system for different colors
- [ ] Command-line arguments

**Not Recommended**:
- Anti-cheat bypass mechanisms
- Stealth/hiding features
- Process injection
- Advanced evasion techniques

---

## Contact & Resources

**Repository**: github.com/Hyrasync/triggerbot
**Documentation**: This file (CLAUDE.md)

**Windows API Documentation**:
- [Microsoft Win32 API Reference](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Hooks Overview](https://docs.microsoft.com/en-us/windows/win32/winmsg/hooks)
- [Mouse Input](https://docs.microsoft.com/en-us/windows/win32/inputdev/mouse-input)

---

*Last Updated: 2025-11-19*
*Repository Snapshot: Commit 2241bdc*
