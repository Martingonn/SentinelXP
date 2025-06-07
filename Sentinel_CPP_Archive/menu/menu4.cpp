#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <limits>

using namespace std;

// Define a struct instead of tuple to hold settings
struct Settings {
    string version;
    string parameter2;
    string parameter3;
};

// Reads first three lines from settings.conf and returns them as Settings struct
Settings getSettings() {
    ifstream file("settings.conf");
    Settings s;

    if (file.is_open()) {
        if (!getline(file, s.version)) cerr << "Error: Unable to read version (line 1)." << endl;
        if (!getline(file, s.parameter2)) cerr << "Error: Unable to read parameter2 (line 2)." << endl;
        if (!getline(file, s.parameter3)) cerr << "Error: Unable to read parameter3 (line 3)." << endl;
        file.close();
    } else {
        cerr << "Error: Unable to open file settings.conf" << endl;
    }

    return s;
}

void welcomeMsg(const string& version) {
    cout << "SentinelXP version: " << version << endl;
}

string wstringToString(const wstring& wstr) {
    if (wstr.empty()) return string();

    int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

string getExeDir() {
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) return "";

    wstring fullPath(path, length);
    size_t pos = fullPath.find_last_of(L"\\/");
    if (pos == wstring::npos) return "";

    wstring dir = fullPath.substr(0, pos);
    return wstringToString(dir);
}

string combinePaths(const string& base, const string& relative) {
    if (base.empty()) return relative;
    // Replace string::back() with indexing
    if (base.size() > 0) {
        char lastChar = base[base.size() - 1];
        if (lastChar != '\\' && lastChar != '/') {
            return base + '\\' + relative;
        }
    }
    return base + relative;
}

int runExe(const string& exePath) {
    HINSTANCE result = ShellExecuteA(NULL, "open", exePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    INT_PTR code = reinterpret_cast<INT_PTR>(result);

    if (code <= 32) return static_cast<int>(code);

    cout << "Launched executable: " << exePath << endl;
    return 0;
}

void menuLoop(const string& exeDir) {
    // Note the space between > >
    vector< pair<string, string> > exeOptions;
    exeOptions.push_back(make_pair("subfolder\\ProgramA.exe", "Run Program A"));
    exeOptions.push_back(make_pair("subfolder\\ProgramB.exe", "Run Program B"));
    exeOptions.push_back(make_pair("subfolder\\ProgramC.exe", "Run Program C"));

    while (true) {
        cout << "\nSelect an option to run:\n";
        for (size_t i = 0; i < exeOptions.size(); ++i) {
            cout << (i + 1) << ". " << exeOptions[i].second << "\n";
        }
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        int choice = -1;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0) {
            cout << "Exiting...\n";
            break;
        }

        if (choice < 1 || choice > static_cast<int>(exeOptions.size())) {
            cout << "Invalid choice, please try again.\n";
            continue;
        }

        string fullExePath = combinePaths(exeDir, exeOptions[choice - 1].first);
        int errorCode = runExe(fullExePath);
        if (errorCode != 0) {
            cerr << "Failed to launch executable: " << fullExePath
                 << ". Error code: " << errorCode << endl;

            switch (errorCode) {
                case 2: cerr << "ERROR_FILE_NOT_FOUND\n"; break;
                case 3: cerr << "ERROR_PATH_NOT_FOUND\n"; break;
                case 5: cerr << "ERROR_ACCESS_DENIED\n"; break;
                case 8: cerr << "ERROR_NOT_ENOUGH_MEMORY\n"; break;
                case 26: cerr << "ERROR_DLL_NOT_FOUND\n"; break;
                case 27: cerr << "ERROR_DLL_INIT_FAILED\n"; break;
                default: cerr << "Unknown error\n"; break;
            }
        }
    }
}

int main() {
    Settings params = getSettings();
    string version = params.version;

    welcomeMsg(version);

    string exeDir = getExeDir();
    if (exeDir.empty()) {
        cerr << "Failed to get executable directory." << endl;
        return 1;
    }

    menuLoop(exeDir);

    return 0;
}
