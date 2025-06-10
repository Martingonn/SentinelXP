#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <direct.h> // For _mkdir

#pragma comment(lib, "psapi.lib")

bool IsWindowsProcess(const char* exePath) {
    char winDir[MAX_PATH];
    if (GetWindowsDirectoryA(winDir, MAX_PATH) == 0)
        return false;
    size_t len = strlen(winDir);
    return _strnicmp(exePath, winDir, len) == 0;
}

void ClearScreen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count, cellCount;
    COORD homeCoords = { 0, 0 };

    if (hStdOut == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hStdOut, (TCHAR) ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hStdOut, homeCoords);
}

void PrintUsage(SIZE_T& winMem, SIZE_T& otherMem) {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        return;
    cProcesses = cbNeeded / sizeof(DWORD);

    winMem = 0;
    otherMem = 0;

    for (unsigned int i = 0; i < cProcesses; ++i) {
        DWORD pid = aProcesses[i];
        if (pid == 0)
            continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess)
            continue;

        char exePath[MAX_PATH] = {0};
        if (GetModuleFileNameExA(hProcess, NULL, exePath, MAX_PATH)) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                if (IsWindowsProcess(exePath))
                    winMem += pmc.WorkingSetSize;
                else
                    otherMem += pmc.WorkingSetSize;
            }
        }
        CloseHandle(hProcess);
    }

    printf("Total RAM used by Windows processes: %.2f MB\n", winMem / (1024.0 * 1024.0));
    printf("Total RAM used by other processes:   %.2f MB\n", otherMem / (1024.0 * 1024.0));
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

volatile bool g_bRunning = true;

int main() {
    // Set up Ctrl+C handler
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    // For averaging
    double sumWinMB = 0.0, sumOtherMB = 0.0;
    int sampleCount = 0;

    while (g_bRunning) {
        ClearScreen();
        SIZE_T winMem = 0, otherMem = 0;
        PrintUsage(winMem, otherMem);

        double winMB = winMem / (1024.0 * 1024.0);
        double otherMB = otherMem / (1024.0 * 1024.0);

        sumWinMB += winMB;
        sumOtherMB += otherMB;
        sampleCount++;

        printf("\nPress Ctrl+C to exit.\n");
        Sleep(1000);
    }

    // Compute averages
    double avgWinMB = sampleCount ? (sumWinMB / sampleCount) : 0.0;
    double avgOtherMB = sampleCount ? (sumOtherMB / sampleCount) : 0.0;

    // Ensure logs directory exists
    _mkdir("logs");

    // Write to log file
    FILE* f = fopen("logs/ram_usage.txt", "a");
    if (f) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(f, "Session on %04d-%02d-%02d %02d:%02d:%02d\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        fprintf(f, "Average RAM used by Windows processes: %.2f MB\n", avgWinMB);
        fprintf(f, "Average RAM used by other processes:   %.2f MB\n", avgOtherMB);
        fprintf(f, "Samples: %d\n\n", sampleCount);
        fclose(f);
    } else {
        printf("Failed to write log file.\n");
    }

    printf("Averages saved to logs/ram_usage.txt\n");
    return 0;
}

// Ctrl+C handler
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT ||
        fdwCtrlType == CTRL_CLOSE_EVENT ||
        fdwCtrlType == CTRL_BREAK_EVENT ||
        fdwCtrlType == CTRL_SHUTDOWN_EVENT) {
        g_bRunning = false;
        Sleep(100); // Give main loop time to finish
        return TRUE;
    }
    return FALSE;
}
