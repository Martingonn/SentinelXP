#include <windows.h>
#include <mmsystem.h>  // For PlaySound and MessageBeep
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cctype>
#include <cstring>
#include <map>

#pragma comment(lib, "winmm.lib")

using namespace std;

bool g_playSounds = true; // Default to play sounds
string g_scanDisk;        // Disk or folder to scan

map<wstring, time_t> g_lastLogTime;
const int DEBOUNCE_SECONDS = 5;

void SetConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE)
        SetConsoleTextAttribute(hConsole, color);
}

void ResetConsoleColor() {
    SetConsoleColor(7);
}

string GetCurrentTimestamp() {
    time_t rawtime;
    time(&rawtime);
    struct tm timeinfo;
    struct tm* ptm = localtime(&rawtime);
    if (ptm != NULL) {
        timeinfo = *ptm;
    } else {
        memset(&timeinfo, 0, sizeof(timeinfo));
    }
    char buffer[20];
    sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d] ",
            timeinfo.tm_year + 1900,
            timeinfo.tm_mon + 1,
            timeinfo.tm_mday,
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec);
    return string(buffer);
}

wstring AnsiToWide(const string& str) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    if (size_needed == 0) return L"";
    wchar_t* buffer = new wchar_t[size_needed];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, size_needed);
    wstring wstr(buffer);
    delete[] buffer;
    return wstr;
}

string WideToAnsi(const wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size_needed == 0) return "";
    char* buffer = new char[size_needed];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buffer, size_needed, NULL, NULL);
    string str(buffer);
    delete[] buffer;
    return str;
}

bool StartsWithIgnoreCase(const wstring& str, const wstring& prefix) {
    if (str.size() < prefix.size()) return false;
    wstring strStart = str.substr(0, prefix.size());
    transform(strStart.begin(), strStart.end(), strStart.begin(), towlower);
    wstring prefixLower = prefix;
    transform(prefixLower.begin(), prefixLower.end(), prefixLower.begin(), towlower);
    return strStart == prefixLower;
}

bool EndsWithIgnoreCase(const string& str, const string& suffix) {
    if (str.length() < suffix.length()) return false;
    string strLower = str;
    string suffixLower = suffix;
    transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
    transform(suffixLower.begin(), suffixLower.end(), suffixLower.begin(), ::tolower);
    return strLower.compare(strLower.length() - suffixLower.length(), suffixLower.length(), suffixLower) == 0;
}

bool EqualsIgnoreCaseW(const wstring& a, const wstring& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (towlower(a[i]) != towlower(b[i])) return false;
    return true;
}

bool EqualsIgnoreCase(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower(*a) != tolower(*b)) return false;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

void LogAndPrint(const string& message, WORD color = 7, ofstream* logFile = NULL) {
    SetConsoleColor(color);
    cout << message << endl;
    ResetConsoleColor();
    if (logFile && logFile->is_open()) {
        *logFile << message << endl;
        logFile->flush();
    }
}

void PlaySoundIfEnabled(const TCHAR* soundFile) {
    if (g_playSounds) {
        PlaySound(soundFile, NULL, SND_FILENAME | SND_ASYNC);
    }
}

bool ShouldOmitFile(const wstring& fullPath, const string& filename) {
    // Lowercase full path for comparison
    wstring fullPathLower = fullPath;
    transform(fullPathLower.begin(), fullPathLower.end(), fullPathLower.begin(), towlower);

    // Omit exact paths
    static const wstring omitPaths[] = {
        L"c:\\documents and settings\\administrator\\ntuser.dat.log",
        L"c:\\documents and settings\\administrator\\desktop\\sentinel\\filemon\\filelog.txt",
        L"c:\\windows\\system32\\config\\software.log"
    };

    for (size_t i = 0; i < sizeof(omitPaths)/sizeof(omitPaths[0]); ++i) {
        if (fullPathLower == omitPaths[i]) {
            return true;
        }
    }

    // Omit Windows Prefetch files (.pf) inside C:\Windows\Prefetch
    if (StartsWithIgnoreCase(fullPathLower, L"c:\\windows\\prefetch") && EndsWithIgnoreCase(filename, ".pf")) {
        return true;
    }

    return false;
}

bool IsDebounced(const wstring& fullPath) {
    time_t now = time(NULL);
    map<wstring, time_t>::iterator it = g_lastLogTime.find(fullPath);
    if (it != g_lastLogTime.end()) {
        if (now - it->second < DEBOUNCE_SECONDS) {
            return true;
        }
    }
    g_lastLogTime[fullPath] = now;
    return false;
}

void PrintFileAction(DWORD action, const string& filename, const wstring& fullPath, ofstream& logFile) {
    if (ShouldOmitFile(fullPath, filename)) {
        return;
    }

    if (IsDebounced(fullPath)) {
        return;
    }

    bool inWindowsFolder = StartsWithIgnoreCase(fullPath, L"C:\\Windows");
    bool inProgramFilesFolder = StartsWithIgnoreCase(fullPath, L"C:\\Program Files");

    string timestamp = GetCurrentTimestamp();

    if ((action == FILE_ACTION_ADDED || action == FILE_ACTION_REMOVED || action == FILE_ACTION_MODIFIED) &&
        (inWindowsFolder || inProgramFilesFolder)) {

        string actionStr;
        switch(action) {
            case FILE_ACTION_ADDED: actionStr = "Added: "; break;
            case FILE_ACTION_REMOVED: actionStr = "Removed: "; break;
            case FILE_ACTION_MODIFIED: actionStr = "Modified: "; break;
            default: actionStr = "Action: "; break;
        }

        string msg = timestamp + actionStr + filename;
        LogAndPrint(msg, 12, &logFile); // Red
        PlaySoundIfEnabled(TEXT("sounds\\cord_sound.wav"));
        return;
    }

    // Other events print normally (yellow for add/remove, default for modify)
    string msg;
    WORD color = 7;
    switch(action) {
        case FILE_ACTION_ADDED:
            msg = timestamp + "Added: " + filename;
            color = 14; // Yellow
            break;
        case FILE_ACTION_REMOVED:
            msg = timestamp + "Removed: " + filename;
            color = 14; // Yellow
            break;
        case FILE_ACTION_MODIFIED:
            msg = timestamp + "Modified: " + filename;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            msg = timestamp + "Renamed From: " + filename;
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            msg = timestamp + "Renamed To: " + filename;
            break;
        default:
            msg = timestamp + "Unknown action on: " + filename;
            break;
    }
    LogAndPrint(msg, color, &logFile);
}

void LoadSettings(const string& configPath) {
    ifstream configFile(configPath.c_str());
    if (!configFile.is_open()) {
        cout << "Settings file not found: " << configPath << ". Using default settings." << endl;
        return;
    }

    string line;
    while (getline(configFile, line)) {
        size_t start = line.find_first_not_of(" \t");
        if (start == string::npos) continue;
        if (line[start] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos == string::npos) continue;

        string key = line.substr(0, eqPos);
        string value = line.substr(eqPos + 1);

        // Trim whitespace from key
        size_t keyStart = key.find_first_not_of(" \t");
        size_t keyEnd = key.find_last_not_of(" \t");
        if (keyStart == string::npos || keyEnd == string::npos) continue;
        key = key.substr(keyStart, keyEnd - keyStart + 1);

        // Trim whitespace from value
        size_t valStart = value.find_first_not_of(" \t");
        size_t valEnd = value.find_last_not_of(" \t");
        if (valStart == string::npos || valEnd == string::npos) continue;
        value = value.substr(valStart, valEnd - valStart + 1);

        if (key == "PlaySounds") {
            if (value == "false" || value == "0" || value == "no") {
                g_playSounds = false;
            } else {
                g_playSounds = true;
            }
        } else if (key == "ScanDisk") {
            g_scanDisk = value;
            if (!g_scanDisk.empty() && g_scanDisk[g_scanDisk.size() - 1] == '\\')
                g_scanDisk.erase(g_scanDisk.size() - 1);
        }
    }
    configFile.close();
}

int main() {
    LoadSettings("settings\\filemon_settings.conf");

    if (g_scanDisk.empty()) {
        cout << "Enter directory to monitor (e.g. C:\\Path\\To\\Folder or C:): ";
        getline(cin, g_scanDisk);
        if (g_scanDisk.empty()) {
            cerr << "No directory entered. Exiting." << endl;
            return 1;
        }
    }

    if (g_scanDisk.length() == 2 && g_scanDisk[1] == ':') {
        g_scanDisk += '\\';
    }

    wstring directoryToWatch = AnsiToWide(g_scanDisk);
    if (directoryToWatch.empty()) {
        cerr << "Failed to convert directory path to wide string." << endl;
        return 1;
    }

    // Create logs directory using Windows API
    if (!CreateDirectoryA("logs", NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            cerr << "Failed to create logs directory. Error: " << err << endl;
            return 1;
        }
    }

    ofstream logFile("logs\\filelog.txt", ios::out | ios::app);
    if (!logFile.is_open()) {
    HANDLE hDir = CreateFileW(
        directoryToWatch.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        cerr << "Failed to get directory handle. Error: " << err << endl;
        return 1;
    }

    const DWORD bufferSize = 1024 * 10;
    BYTE* buffer = new BYTE[bufferSize];
    if (!buffer) {
        cerr << "Failed to allocate buffer." << endl;
        CloseHandle(hDir);
        return 1;
    }

    cout << "Monitoring directory: " << WideToAnsi(directoryToWatch) << endl;

    DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                         FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

    DWORD bytesReturned = 0;

    while (true) {
        BOOL success = ReadDirectoryChangesW(
            hDir,
            buffer,
            bufferSize,
            TRUE,
            notifyFilter,
            &bytesReturned,
            NULL,
            NULL);

        if (!success) {
            DWORD err = GetLastError();
            if (err == ERROR_OPERATION_ABORTED) {
                cout << "Monitoring stopped." << endl;
                break;
            }
            cerr << "ReadDirectoryChangesW failed. Error: " << err << endl;
            break;
}
Use Control + Shift + m to toggle the tab key moving focus. Alternatively, use esc then tab to move to the next interactive element on the page.

        cerr << "Failed to open log file: logs\\filelog.txt" << endl;
        return 1;
    }

    HANDLE hDir = CreateFileW(
        directoryToWatch.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        cerr << "Failed to get directory handle. Error: " << err << endl;
        return 1;
    }

    const DWORD bufferSize = 1024 * 10;
    BYTE* buffer = new BYTE[bufferSize];
    if (!buffer) {
        cerr << "Failed to allocate buffer." << endl;
        CloseHandle(hDir);
        return 1;
    }

    cout << "Monitoring directory: " << WideToAnsi(directoryToWatch) << endl;

    DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                         FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

    DWORD bytesReturned = 0;

    while (true) {
        BOOL success = ReadDirectoryChangesW(
            hDir,
            buffer,
            bufferSize,
            TRUE,
            notifyFilter,
            &bytesReturned,
            NULL,
            NULL);

        if (!success) {
            DWORD err = GetLastError();
            if (err == ERROR_OPERATION_ABORTED) {
                cout << "Monitoring stopped." << endl;
                break;
            }
            cerr << "ReadDirectoryChangesW failed. Error: " << err << endl;
            break;
        }

        if (bytesReturned == 0)
            continue;

        DWORD offset = 0;
        FILE_NOTIFY_INFORMATION* pNotify = NULL;

        while (true) {
            if (offset >= bytesReturned) {
                cerr << "Invalid offset detected: " << offset << ", stopping processing." << endl;
                break;
            }

            pNotify = (FILE_NOTIFY_INFORMATION*)(buffer + offset);

            int filenameLength = pNotify->FileNameLength / sizeof(WCHAR);
            if (filenameLength <= 0 || filenameLength >= 512) {
                if (pNotify->NextEntryOffset == 0)
                    break;
                offset += pNotify->NextEntryOffset;
                continue;
            }

            WCHAR wFilename[512];
            wcsncpy(wFilename, pNotify->FileName, filenameLength);
            wFilename[filenameLength] = 0;

            wstring fullPath = directoryToWatch;
            if (!fullPath.empty() && fullPath[fullPath.size() - 1] != L'\\')
                fullPath += L'\\';
            fullPath += wFilename;

            char filename[512] = {0};
            int convResult = WideCharToMultiByte(CP_ACP, 0, wFilename, -1, filename, sizeof(filename), NULL, NULL);
            if (convResult == 0) {
                DWORD convErr = GetLastError();
                cerr << "Failed to convert filename to ANSI. Error: " << convErr << endl;
            } else {
                PrintFileAction(pNotify->Action, filename, fullPath, logFile);
            }

            if (pNotify->NextEntryOffset == 0)
                break;

            offset += pNotify->NextEntryOffset;
        }
    }

    delete[] buffer;
    CloseHandle(hDir);
    logFile.close();
    return 0;
}
