#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

using namespace std;

// Reads first three lines from settings.conf and returns them as a tuple
tuple<string, string, string> getSettings() {
    ifstream file("settings.conf");
    string version, parameter2, parameter3;

    if (file.is_open()) {
        if (!getline(file, version)) {
            cerr << "Error: Unable to read version (line 1)." << endl;
        }
        if (!getline(file, parameter2)) {
            cerr << "Error: Unable to read parameter2 (line 2)." << endl;
        }
        if (!getline(file, parameter3)) {
            cerr << "Error: Unable to read parameter3 (line 3)." << endl;
        }
        file.close();
    } else {
        cerr << "Error: Unable to open file settings.conf" << endl;
    }

    return make_tuple(version, parameter2, parameter3);
}

// Prints welcome message using the version string
void welcomeMsg(const string& version) {
    wstring wversion(version.begin(), version.end());
    wcout << L"SentinelXP version: " << wversion << endl;
}

// Retrieves the directory of the running executable
wstring getExeDir() {
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return L"";
    }
    wstring fullPath(path, length);

    size_t pos = fullPath.find_last_of(L"\\/");
    if (pos == wstring::npos) {
        return L"";
    }
    return fullPath.substr(0, pos);
}

// Combines base and relative paths safely
wstring combinePaths(const wstring& base, const wstring& relative) {
    if (base.empty()) return relative;
    if (base.back() != L'\\' && base.back() != L'/') {
        return base + L'\\' + relative;
    }
    return base + relative;
}

// Launches the executable at the given path
// Returns 0 on success, or a Windows error code (>0) on failure
int runExe(const wstring& exePath) {
    HINSTANCE result = ShellExecuteW(NULL, L"open", exePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    INT_PTR code = reinterpret_cast<INT_PTR>(result);

    if (code <= 32) {
        return static_cast<int>(code);
    }

    wcout << L"Launched executable: " << exePath << endl;
    return 0;  // success
}

// Displays menu, handles user input, and runs selected executables
void menuLoop(const wstring& exeDir) {
    // List of relative executable paths and their display names
    vector<pair<wstring, wstring>> exeOptions = {
        {L"subfolder\\ProgramA.exe", L"Run Program A"},
        {L"subfolder\\ProgramB.exe", L"Run Program B"},
        {L"subfolder\\ProgramC.exe", L"Run Program C"}
    };

    while (true) {
        wcout << L"\nSelect an option to run:\n";
        for (size_t i = 0; i < exeOptions.size(); ++i) {
            wcout << (i + 1) << L". " << exeOptions[i].second << L"\n";
        }
        wcout << L"0. Exit\n";
        wcout << L"Enter choice: ";

        int choice = -1;
        wcin >> choice;

        if (wcin.fail()) {
            wcin.clear();
            wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
            wcout << L"Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0) {
            wcout << L"Exiting...\n";
            break;
        }

        if (choice < 1 || choice > static_cast<int>(exeOptions.size())) {
            wcout << L"Invalid choice, please try again.\n";
            continue;
        }

        wstring fullExePath = combinePaths(exeDir, exeOptions[choice - 1].first);
        int errorCode = runExe(fullExePath);
        if (errorCode != 0) {
            wcerr << L"Failed to launch executable: " << fullExePath 
                  << L". Error code: " << errorCode << endl;

            switch (errorCode) {
                case 2: wcerr << L"ERROR_FILE_NOT_FOUND\n"; break;
                case 3: wcerr << L"ERROR_PATH_NOT_FOUND\n"; break;
                case 5: wcerr << L"ERROR_ACCESS_DENIED\n"; break;
                case 8: wcerr << L"ERROR_NOT_ENOUGH_MEMORY\n"; break;
                case 26: wcerr << L"ERROR_DLL_NOT_FOUND\n"; break;
                case 27: wcerr << L"ERROR_DLL_INIT_FAILED\n"; break;
                default: wcerr << L"Unknown error\n"; break;
            }
        }
    }
}

int wmain() {
    // Read configuration parameters
    tuple<string, string, string> params = getSettings();
    string version = get<0>(params);

    // Display welcome message
    welcomeMsg(version);

    // Get executable directory
    wstring exeDir = getExeDir();
    if (exeDir.empty()) {
        wcerr << L"Failed to get executable directory." << endl;
        return 1;
    }

    // Start menu loop
    menuLoop(exeDir);

    return 0;
}
