// sysIntegrity_console.cpp
// Windows XP / MinGW / C++98 compatible
// Scans C:\Windows recursively, checks system integrity,
// shows detailed info, tracks changes from saved state,
// plays sounds, uses console colors,
// and shows non-worm missing files on 'P' key.

#include <windows.h>
#include <mmsystem.h> // For PlaySound
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <cstdio>
#include <conio.h>   // For _getch()
#include <direct.h>  // For _mkdir

#pragma comment(lib, "winmm.lib")

struct Config {
    bool playing_sound;
    Config() : playing_sound(true) {}
};

// Console color helpers
void SetConsoleTextRed() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
}
void SetConsoleTextGreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}
void SetConsoleTextYellow() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}
void ResetConsoleTextColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// Worm DLL map
std::map<std::string, std::string> GetWormDllMap() {
    std::map<std::string, std::string> wormDllMap;
    wormDllMap["rpcss.dll"] = "Blaster worm (MSBlast)";
    wormDllMap["msblast.exe"] = "Blaster worm (MSBlast)";
    wormDllMap["avserve2.exe"] = "Sasser worm";
    wormDllMap["avserve.exe"] = "Sasser worm";
    wormDllMap["avserve3.exe"] = "Sasser worm";
    wormDllMap["explorer.exe"] = "Nimda worm";
    wormDllMap["winlogon.exe"] = "Nimda worm";
    wormDllMap["mydoom.exe"] = "Mydoom worm";
    wormDllMap["mydoom.dll"] = "Mydoom worm";
    wormDllMap["msadc.dll"] = "Code Red worm";
    wormDllMap["msadc.exe"] = "Code Red worm";
    wormDllMap["sqlservr.exe"] = "Slammer worm";
    wormDllMap["wsock32.dll"] = "Various worms";
    wormDllMap["ws2_32.dll"] = "Various worms";
    return wormDllMap;
}

// Check if file exists
bool FileExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Play sound asynchronously
void PlaySoundFile(const std::string& exeDir, const std::string& soundFile) {
    std::string soundPath = exeDir + "sounds\\" + soundFile;
    DWORD attr = GetFileAttributesA(soundPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        std::cerr << "Warning: Sound file not found: " << soundPath << std::endl;
        return;
    }
    if (!PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC)) {
        std::cerr << "Warning: Failed to play sound: " << soundPath << std::endl;
    }
}

// Save file list to a text file (one file per line)
void SaveFileList(const std::string& filename, const std::vector<std::string>& files) {
    // Create folder if not exists
    size_t pos = filename.find_last_of("\\/");
    if (pos != std::string::npos) {
        std::string folder = filename.substr(0, pos);
        _mkdir(folder.c_str());  // Safe to call even if folder exists
    }

    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open()) return;
    for (size_t i = 0; i < files.size(); ++i) {
        ofs << files[i] << "\n";
    }
    ofs.close();
}

// Load file list from a text file
std::vector<std::string> LoadFileList(const std::string& filename) {
    std::vector<std::string> files;
    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open()) return files;
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) files.push_back(line);
    }
    ifs.close();
    return files;
}

// Recursive directory traversal to find all files under directory
void TraverseDirectory(const std::string& directory, std::vector<std::string>& files) {
    std::string searchPath = directory + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        const char* name = findData.cFileName;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        std::string fullPath = directory + "\\" + name;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            TraverseDirectory(fullPath, files);
        } else {
            files.push_back(fullPath);
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
}

// Get filename from full path (lowercase)
std::string GetFilenameLower(const std::string& path) {
    size_t pos = path.find_last_of("\\/");
    std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    for (size_t i = 0; i < filename.size(); ++i)
        filename[i] = (char)tolower(filename[i]);
    return filename;
}

// Show missing worm files
void ShowMissingFiles(const std::vector<std::string>& missingFiles) {
    if (missingFiles.empty()) {
        SetConsoleTextGreen();
        std::cout << "\nNo missing worm-related files detected.\n" << std::endl;
        ResetConsoleTextColor();
        return;
    }
    SetConsoleTextRed();
    std::cout << "\nMissing worm-related files:\n" << std::endl;
    for (size_t i = 0; i < missingFiles.size(); ++i) {
        std::cout << "  " << missingFiles[i] << " - Possible worm infection" << std::endl;
    }
    ResetConsoleTextColor();
    std::cout << std::endl;
}

// Show missing non-worm files
void ShowMissingNonWormFiles(const std::vector<std::string>& missingFiles, const std::map<std::string, std::string>& wormDllMap) {
    if (missingFiles.empty()) {
        SetConsoleTextGreen();
        std::cout << "\nNo missing non-worm related files detected.\n" << std::endl;
        ResetConsoleTextColor();
        return;
    }
    SetConsoleTextYellow();
    std::cout << "\nMissing non-worm related files:\n" << std::endl;
    bool anyNonWorm = false;
    for (size_t i = 0; i < missingFiles.size(); ++i) {
        std::string file = missingFiles[i];
        if (wormDllMap.find(file) == wormDllMap.end()) {
            std::cout << "  " << file << std::endl;
            anyNonWorm = true;
        }
    }
    if (!anyNonWorm) {
        std::cout << "  None" << std::endl;
    }
    ResetConsoleTextColor();
    std::cout << std::endl;
}

// Compare two file lists and return counts of added and removed files
void CompareFileLists(const std::vector<std::string>& oldList, const std::vector<std::string>& newList,
                      int& added, int& removed) {
    std::map<std::string, bool> oldMap, newMap;
    for (size_t i = 0; i < oldList.size(); ++i) oldMap[oldList[i]] = true;
    for (size_t i = 0; i < newList.size(); ++i) newMap[newList[i]] = true;
    added = 0; removed = 0;
    for (size_t i = 0; i < newList.size(); ++i) {
        if (oldMap.find(newList[i]) == oldMap.end()) ++added;
    }
    for (size_t i = 0; i < oldList.size(); ++i) {
        if (newMap.find(oldList[i]) == newMap.end()) ++removed;
    }
}

int main() {
    Config config;

    // Get executable directory
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos)
        exeDir = exeDir.substr(0, lastSlash + 1);
    else
        exeDir = "";

    std::vector<std::string> allFiles;
    std::map<std::string, std::string> wormDllMap = GetWormDllMap();
    std::vector<std::string> missingFiles;
    std::vector<std::string> foundFilesLower;

    std::cout << "Scanning C:\\Windows for files. Please wait..." << std::endl;
    TraverseDirectory("C:\\Windows", allFiles);
    std::cout << "Scan complete. Found " << allFiles.size() << " files." << std::endl;

    // Prepare lowercase filenames for worm detection
    foundFilesLower.reserve(allFiles.size());
    for (size_t i = 0; i < allFiles.size(); ++i) {
        foundFilesLower.push_back(GetFilenameLower(allFiles[i]));
    }

    // Load previous saved state if exists
    std::vector<std::string> savedState = LoadFileList("settings\\current_state.conf");
    bool usingSavedState = !savedState.empty();

    if (usingSavedState) {
        SetConsoleTextYellow();
        std::cout << "Using saved state from settings\\current_state.conf for change detection." << std::endl;
        ResetConsoleTextColor();
    }

    while (true) {
        std::cout << "\nOptions:\n"
                  << "  V - View all files\n"
                  << "  D - Diagnose missing worm-related files and show integrity\n"
                  << "  B - Show missing worm-related files\n"
                  << "  P - Show missing non-worm related files\n"
                  << "  C - Save current state\n"
                  << "  Q - Quit\n"
                  << "Enter option: ";

        int ch = _getch();
        std::cout << ch << "\n";

        if (ch == 'V' || ch == 'v') {
            std::cout << "\nListing all files found under C:\\Windows:\n" << std::endl;
            for (size_t i = 0; i < allFiles.size(); ++i) {
                std::cout << allFiles[i] << std::endl;
            }
            std::cout << "\nEnd of list.\n" << std::endl;
        }
        else if (ch == 'D' || ch == 'd') {
            // Detect missing worm files
            missingFiles.clear();
            for (std::map<std::string, std::string>::iterator it = wormDllMap.begin(); it != wormDllMap.end(); ++it) {
                std::string wormFile = it->first;
                bool found = false;
                for (size_t i = 0; i < foundFilesLower.size(); ++i) {
                    if (foundFilesLower[i] == wormFile) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    missingFiles.push_back(wormFile);
                }
            }

            // Save missing worm files for next run
            SaveFileList("logs\\state_missing_files.txt", missingFiles);

            // Integrity calculation
            int totalFiles = (int)allFiles.size();
            int integralFiles = totalFiles - (int)missingFiles.size(); // Approximate integral files = total - missing worms
            double integrityPercent = (totalFiles > 0) ? (100.0 * integralFiles / totalFiles) : 0.0;

            // Show integrity with color and play sound
            if (integrityPercent >= 95.0) {
                SetConsoleTextGreen();
                std::cout << "System Integrity: " << integralFiles << "/" << totalFiles
                          << " files OK (" << std::fixed << std::setprecision(2)
                          << integrityPercent << "%) [OK]" << std::endl;
                ResetConsoleTextColor();
                if (config.playing_sound) {
                    PlaySoundFile(exeDir, "success.wav");
                }
            }
            else {
                SetConsoleTextRed();
                std::cout << "System Integrity: " << integralFiles << "/" << totalFiles
                          << " files OK (" << std::fixed << std::setprecision(2)
                          << integrityPercent << "%) [WARNING]" << std::endl;
                ResetConsoleTextColor();
                if (config.playing_sound) {
                    PlaySoundFile(exeDir, "cord_sound.wav");
                }
            }

            // If using saved state, compare changes
            if (usingSavedState) {
                int added = 0, removed = 0;
                CompareFileLists(savedState, allFiles, added, removed);

                // Calculate percentage change
                double percentChange = 0.0;
                if ((int)savedState.size() > 0) {
                    percentChange = 100.0 * (double)(added - removed) / (double)savedState.size();
                }

                std::cout << std::fixed << std::setprecision(2);
                std::cout << "Change since last saved state: " << (percentChange >= 0 ? "+" : "") << percentChange << "% ";
                std::cout << "(" << (added - removed) << " files)" << std::endl;
            }
            else {
                SetConsoleTextYellow();
                std::cout << "No saved state loaded. Use 'C' to save current state." << std::endl;
                ResetConsoleTextColor();
            }
        }
        else if (ch == 'B' || ch == 'b') {
            // Show missing worm-related files from last diagnosis
            std::vector<std::string> lastMissing = LoadFileList("logs\\state_missing_files.txt");
            ShowMissingFiles(lastMissing);
        }
        else if (ch == 'P' || ch == 'p') {
            // Show missing non-worm related files from last diagnosis
            std::vector<std::string> lastMissing = LoadFileList("logs\\state_missing_files.txt");
            ShowMissingNonWormFiles(lastMissing, wormDllMap);
        }
        else if (ch == 'C' || ch == 'c') {
            // Save current full file list as current state
            SaveFileList("settings\\current_state.conf", allFiles);
            usingSavedState = true;
            savedState = allFiles;
            SetConsoleTextGreen();
            std::cout << "Current state saved to settings\\current_state.conf" << std::endl;
            ResetConsoleTextColor();
        }
        else if (ch == 'Q' || ch == 'q') {
            std::cout << "Exiting." << std::endl;
            break;
        }
        else {
            std::cout << "Invalid option. Please try again." << std::endl;
        }
    }

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
