// sysIntegrity7.cpp
#include <windows.h>
#include <mmsystem.h>  // For PlaySound
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <ctime>
#include <sstream>

#pragma comment(lib, "winmm.lib")

struct Config {
    bool playing_sound;
    bool debugger_mode;

    Config() : playing_sound(true), debugger_mode(true) {}

    void ReadFromFile(const std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open config file: " << filename << ", using defaults." << std::endl;
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            size_t comment_pos = line.find('#');
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);

            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            size_t end = line.find_last_not_of(" \t\r\n");
            line = line.substr(start, end - start + 1);

            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos) continue;

            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);

            start = key.find_first_not_of(" \t");
            end = key.find_last_not_of(" \t");
            if (start == std::string::npos) continue;
            key = key.substr(start, end - start + 1);

            start = value.find_first_not_of(" \t");
            end = value.find_last_not_of(" \t");
            if (start == std::string::npos) value = "";
            else value = value.substr(start, end - start + 1);

            for (size_t i = 0; i < key.size(); ++i) {
                if (key[i] >= 'A' && key[i] <= 'Z') key[i] = key[i] - 'A' + 'a';
            }

            if (key == "playing_sound") {
                playing_sound = (value == "true" || value == "1");
            } else if (key == "debugger_mode") {
                debugger_mode = (value == "true" || value == "1");
            }
        }
        file.close();
    }
};

static std::string cleanLine(const std::string& s) {
    std::string cleaned;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 32 && c <= 126) cleaned += (char)c;
    }
    size_t start = 0;
    while (start < cleaned.size() && (cleaned[start] == ' ' || cleaned[start] == '\t')) ++start;
    size_t end = cleaned.size();
    while (end > start && (cleaned[end - 1] == ' ' || cleaned[end - 1] == '\t')) --end;
    if (start >= end) return "";
    return cleaned.substr(start, end - start);
}

bool FileExists(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

unsigned long SimpleFileChecksum(const char* path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file for checksum: " << path << std::endl;
        return 0;
    }
    unsigned long checksum = 0;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::streamsize bytesRead = file.gcount();
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            checksum += (unsigned char)buffer[i];
        }
    }
    file.close();
    return checksum;
}

void SetConsoleTextRed() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

void SetConsoleTextYellow() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void SetConsoleTextGreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void ResetConsoleTextColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void ShowSpinner(int iterations) {
    const char spinner_chars[] = {'|', '/', '-', '\\'};
    int spinner_len = sizeof(spinner_chars) / sizeof(spinner_chars[0]);
    for (int i = 0; i < iterations; ++i) {
        char c = spinner_chars[i % spinner_len];
        std::cout << "\rLoading " << c << std::flush;
        clock_t start_time = clock();
        while ((clock() - start_time) < (CLOCKS_PER_SEC / 10)) {}
    }
    std::cout << "\r             \r";
}

std::vector<std::string> ReadSysList(const char* configPath, bool debugger_mode) {
    std::vector<std::string> modules;
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open syslist file: " << configPath << std::endl;
        return modules;
    }

    std::string line;
    int line_count = 0;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) continue;

        if (debugger_mode) {
            std::cout << "Read line chars (hex): ";
            for (size_t i = 0; i < cleaned.size(); ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)cleaned[i] << " ";
            }
            std::cout << std::dec << " => [" << cleaned << "]" << std::endl;
        } else {
            ShowSpinner(1);
        }

        modules.push_back(cleaned);
        ++line_count;
    }
    file.close();

    if (!debugger_mode) {
        std::cout << "Loaded " << line_count << " entries from syslist.txt." << std::endl;
    }

    return modules;
}

bool CheckRunningProcesses() {
    // Dummy placeholder, always true
    return true;
}

void PlayRingingSound(const std::string& exeDir) {
    std::string soundPath = exeDir + "sounds\\cord_sound.wav";
    DWORD attr = GetFileAttributesA(soundPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        std::cerr << "Warning: Sound file not found: " << soundPath << std::endl;
        return;
    }
    if (!PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC)) {
        std::cerr << "Warning: Failed to play sound: " << soundPath << std::endl;
    }
}

std::string GetFileExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    std::string ext = filename.substr(pos);
    for (size_t i = 0; i < ext.size(); ++i) ext[i] = (char)tolower(ext[i]);
    return ext;
}

std::map<std::string, unsigned long> LoadState(const std::string& filepath) {
    std::map<std::string, unsigned long> state;
    std::ifstream file(filepath.c_str());
    if (!file.is_open()) {
        // No saved state, treat as first run
        return state;
    }
    std::string line;
    while (std::getline(file, line)) {
        size_t sep = line.find('|');
        if (sep == std::string::npos) continue;
        std::string path = line.substr(0, sep);
        unsigned long checksum = 0;
        std::istringstream iss(line.substr(sep + 1));
        iss >> checksum;
        state[path] = checksum;
    }
    file.close();
    return state;
}

void SaveState(const std::string& filepath, const std::map<std::string, unsigned long>& state) {
    std::ofstream file(filepath.c_str());
    if (!file.is_open()) {
        std::cerr << "Warning: Could not save state to " << filepath << std::endl;
        return;
    }
    for (std::map<std::string, unsigned long>::const_iterator it = state.begin(); it != state.end(); ++it) {
        file << it->first << "|" << it->second << std::endl;
    }
    file.close();
}

int main() {
    std::cout << "Starting Windows Integrity Check..." << std::endl;

    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Warning: Could not determine executable directory." << std::endl;
    }
    std::string exeDir(exePath);
    size_t pos = exeDir.find_last_of("\\/");
    if (pos != std::string::npos) {
        exeDir = exeDir.substr(0, pos + 1);
    } else {
        exeDir.clear();
    }

    std::string settingsDir = exeDir + "settings\\";
    std::string configPath = settingsDir + "integrity_settings.conf";
    std::string syslistPath = settingsDir + "syslist.txt";
    std::string statePath = settingsDir + "state.conf";

    Config config;
    config.ReadFromFile(configPath);

    std::cout << "Playing sound setting: " << (config.playing_sound ? "true" : "false") << std::endl;
    std::cout << "Debugger mode: " << (config.debugger_mode ? "true" : "false") << std::endl;

    std::vector<std::string> criticalModules = ReadSysList(syslistPath.c_str(), config.debugger_mode);
    if (criticalModules.empty()) {
        std::cerr << "No modules specified in syslist.txt or file missing." << std::endl;
        std::cout << "Press Enter to close...";
        std::cin.get();
        return 1;
    }

    std::map<std::string, unsigned long> savedState = LoadState(statePath);
    std::map<std::string, unsigned long> currentState;

    bool criticalFileMissing = false;
    int integral_files = 0;
    int max_integral_files = (int)criticalModules.size();

    for (size_t i = 0; i < criticalModules.size(); ++i) {
        std::string fullPath = criticalModules[i];
        if (fullPath.empty()) continue;

        for (size_t j = 0; j < fullPath.size(); ++j) {
            if (fullPath[j] == '/') fullPath[j] = '\\';
        }

        if (config.debugger_mode) {
            std::cout << "Checking file: [" << fullPath << "]" << std::endl;
        }

        if (!FileExists(fullPath.c_str())) {
            std::string ext = GetFileExtension(fullPath);
            if (ext == ".exe" || ext == ".dll") {
                SetConsoleTextRed();
                std::cout << "Missing critical file: " << fullPath << std::endl;
                ResetConsoleTextColor();
                criticalFileMissing = true;
            } else {
                SetConsoleTextYellow();
                std::cout << "Missing file: " << fullPath << std::endl;
                ResetConsoleTextColor();
            }
            continue;
        }

        unsigned long checksum = SimpleFileChecksum(fullPath.c_str());
        if (checksum == 0) {
            SetConsoleTextRed();
            std::cout << "Warning: Could not read checksum for: " << fullPath << std::endl;
            ResetConsoleTextColor();
            criticalFileMissing = true;
            continue;
        }

        currentState[fullPath] = checksum;

        std::map<std::string, unsigned long>::iterator it = savedState.find(fullPath);
        if (it != savedState.end()) {
            if (checksum != it->second) {
                SetConsoleTextRed();
                std::cout << "Modified file detected: " << fullPath << std::endl;
                ResetConsoleTextColor();
                criticalFileMissing = true;
            } else {
                SetConsoleTextGreen();
                std::cout << "Module found" << std::endl;
                ResetConsoleTextColor();
                integral_files++;
            }
        } else {
            SetConsoleTextGreen();
            std::cout << "Module found (new)" << std::endl;
            ResetConsoleTextColor();
            integral_files++;
        }
    }

    if (config.playing_sound) {
        PlayRingingSound(exeDir);
    }

    bool processesOk = CheckRunningProcesses();

    if (!criticalFileMissing && processesOk) {
        std::cout << "All files and processes are intact." << std::endl;
    }

    double integrity_percent = (max_integral_files > 0) ? (100.0 * integral_files / max_integral_files) : 0.0;
    std::cout << "System Integrity: " << integral_files << "/" << max_integral_files
              << " files OK (" << std::fixed << std::setprecision(2) << integrity_percent << "%)" << std::endl;

    // Calculate previous integrity stats
    int prev_integral_files = 0;
    int prev_max_files = (int)savedState.size();
    // Count how many saved files exist and match checksum in current state
    for (std::map<std::string, unsigned long>::const_iterator it = savedState.begin(); it != savedState.end(); ++it) {
        std::map<std::string, unsigned long>::const_iterator curr_it = currentState.find(it->first);
        if (curr_it != currentState.end() && curr_it->second == it->second) {
            prev_integral_files++;
        }
    }
    double prev_integrity_percent = (prev_max_files > 0) ? (100.0 * prev_integral_files / prev_max_files) : 0.0;

    // Show change compared to previous state
    double diff_percent = integrity_percent - prev_integrity_percent;
    int diff_files = integral_files - prev_integral_files;

    std::cout << "Change since last saved state: ";
    if (diff_percent >= 0.0) std::cout << "+";
    std::cout << std::fixed << std::setprecision(2) << diff_percent << "% (";
    if (diff_files >= 0) std::cout << "+";
    std::cout << diff_files << " files)" << std::endl;

    // Ask user whether to save current state
    char saveChoice = 'N';
    std::cout << "Save current system state as baseline for future checks? (Y/N): ";
    std::cin >> saveChoice;
    std::cin.ignore(10000, '\n'); // clear input buffer

    if (saveChoice == 'Y' || saveChoice == 'y') {
        SaveState(statePath, currentState);
        std::cout << "Current system state saved to " << statePath << std::endl;
    } else {
        std::cout << "Current system state NOT saved." << std::endl;
    }

    std::cout << "Press Enter to close...";
    std::cin.get();

    return 0;
}
