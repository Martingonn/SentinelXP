#include <windows.h>
#include <tlhelp32.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>

#pragma comment(lib, "winmm.lib")

// List of critical system DLLs and kernel modules to check (ci.dll removed)
const char* criticalModules[] = {
    "ntoskrnl.exe",  // Kernel
    "kernel32.dll",
    "user32.dll",
    "gdi32.dll",
    "advapi32.dll",
    "shell32.dll"
};

const int numCriticalModules = sizeof(criticalModules) / sizeof(criticalModules[0]);

// Trim whitespace from start and end of string
static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) ++start;
    size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) --end;
    return s.substr(start, end - start);
}

// Reads "playing_sound" setting from config file at given path
// Returns true if playing_sound is set to "true" (case insensitive), false otherwise
bool ReadPlayingSoundSetting(const char* configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Could not open config file: " << configPath << std::endl;
        return false; // default false if no file
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove comments (anything after # or ;)
        size_t commentPos = line.find_first_of("#;");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        line = trim(line);
        if (line.empty()) continue;

        // Look for key=value pattern
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, equalPos));
        std::string value = trim(line.substr(equalPos + 1));

        // Case-insensitive compare key to "playing_sound"
        if (key.size() == 13 &&
            (key[0] == 'p' || key[0] == 'P') &&
            (key[1] == 'l' || key[1] == 'L') &&
            (key[2] == 'a' || key[2] == 'A') &&
            (key[3] == 'y' || key[3] == 'Y') &&
            (key[4] == 'i' || key[4] == 'I') &&
            (key[5] == 'n' || key[5] == 'N') &&
            (key[6] == 'g' || key[6] == 'G') &&
            (key[7] == '_' || key[7] == '_') &&
            (key[8] == 's' || key[8] == 'S') &&
            (key[9] == 'o' || key[9] == 'O') &&
            (key[10] == 'u' || key[10] == 'U') &&
            (key[11] == 'n' || key[11] == 'N') &&
            (key[12] == 'd' || key[12] == 'D')) {
            // Interpret value as boolean
            // Accept "true", "1", "yes" as true
            if (value.size() >= 1) {
                char c = value[0];
                if (c == 't' || c == 'T' || c == '1' || c == 'y' || c == 'Y') {
                    return true;
                }
            }
            return false;
        }
    }

    return false; // default false if setting not found
}

// Helper: Check if file exists
bool FileExists(const char* filename) {
    DWORD attrib = GetFileAttributesA(filename);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Helper: Compute a simple checksum of a file (not cryptographically secure)
unsigned long SimpleFileChecksum(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return 0;

    unsigned long checksum = 0;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        for (std::streamsize i = 0; i < file.gcount(); ++i) {
            checksum += (unsigned char)buffer[i];
        }
    }
    return checksum;
}

// Get full system directory path + filename
std::string GetSystemFilePath(const char* filename) {
    char systemDir[MAX_PATH];
    UINT len = GetSystemDirectoryA(systemDir, MAX_PATH);
    if (len == 0 || len > MAX_PATH) return "";
    std::string path(systemDir);
    path += "\\";
    path += filename;
    return path;
}

// Set console text color to red
void SetConsoleTextRed() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        // 12 = FOREGROUND_RED | FOREGROUND_INTENSITY
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
}

// Reset console text color to default (gray)
void ResetConsoleTextColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        // 7 = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE (default gray)
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}

// Check critical system modules for existence and integrity (basic checksum)
// Prints issues in red
bool CheckSystemModules(std::map<std::string, unsigned long>& knownGoodChecksums) {
    bool allOk = true;
    for (int i = 0; i < numCriticalModules; ++i) {
        std::string fullPath = GetSystemFilePath(criticalModules[i]);
        if (!FileExists(fullPath.c_str())) {
            SetConsoleTextRed();
            std::cout << "Missing critical system file: " << fullPath << std::endl;
            ResetConsoleTextColor();
            allOk = false;
            continue;
        }
        unsigned long checksum = SimpleFileChecksum(fullPath.c_str());
        if (knownGoodChecksums.find(criticalModules[i]) != knownGoodChecksums.end()) {
            if (checksum != knownGoodChecksums[criticalModules[i]]) {
                SetConsoleTextRed();
                std::cout << "Modified system file detected: " << fullPath << std::endl;
                ResetConsoleTextColor();
                allOk = false;
            }
        } else {
            // Store checksum for first run (in real scenario, use known good checksums)
            knownGoodChecksums[criticalModules[i]] = checksum;
        }
    }
    return allOk;
}

// Check running processes for suspicious or missing system processes
// Prints issues in red
bool CheckRunningProcesses() {
    bool allOk = true;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot." << std::endl;
        return false;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        std::cerr << "Failed to get first process." << std::endl;
        return false;
    }

    // List of expected core Windows processes on XP (simplified)
    const char* expectedProcesses[] = {
        "csrss.exe",
        "winlogon.exe",
        "services.exe",
        "lsass.exe",
        "svchost.exe",
        "explorer.exe"
    };
    const int numExpected = sizeof(expectedProcesses) / sizeof(expectedProcesses[0]);

    std::vector<std::string> runningProcesses;
    do {
        runningProcesses.push_back(pe.szExeFile);
    } while (Process32Next(hSnapshot, &pe));

    CloseHandle(hSnapshot);

    for (int i = 0; i < numExpected; ++i) {
        bool found = false;
        for (size_t j = 0; j < runningProcesses.size(); ++j) {
            if (_stricmp(runningProcesses[j].c_str(), expectedProcesses[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            SetConsoleTextRed();
            std::cout << "Critical system process missing: " << expectedProcesses[i] << std::endl;
            ResetConsoleTextColor();
            allOk = false;
        }
    }

    return allOk;
}

int main() {
    std::cout << "Starting Windows Integrity Check..." << std::endl;

    bool playSoundSetting = ReadPlayingSoundSetting("settings/integrity_settings.conf");
    std::cout << "Playing sound setting: " << (playSoundSetting ? "true" : "false") << std::endl;

    std::map<std::string, unsigned long> knownGoodChecksums;

    bool modulesOk = CheckSystemModules(knownGoodChecksums);
    bool processesOk = CheckRunningProcesses();

    if (!modulesOk || !processesOk) {
        std::cout << "Integrity issues detected!" << std::endl;
        if (playSoundSetting) {
            if (!PlaySoundA("sounds\\cord_sound.wav", NULL, SND_FILENAME | SND_ASYNC)) {
                std::cerr << "Failed to play alert sound." << std::endl;
            }
        }
    } else {
        std::cout << "All system files and processes appear intact." << std::endl;
        if (playSoundSetting) {
            if (!PlaySoundA("sounds\\success.wav", NULL, SND_FILENAME | SND_ASYNC)) {
                std::cerr << "Failed to play success sound." << std::endl;
            }
        }
    }

    // Pause so console stays open
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
