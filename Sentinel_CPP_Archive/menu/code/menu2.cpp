#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

std::vector<std::string> readModuleList(const std::string& filename) {
    std::vector<std::string> modules;
    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return modules;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove any trailing whitespace or newline characters
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (!line.empty()) {
            modules.push_back(line);
        }
    }

    file.close();
    return modules;
}

void displayMenu(const std::vector<std::string>& modules) {
    for (size_t i = 0; i < modules.size(); ++i) {
        std::cout << i + 1 << ". " << modules[i] << std::endl;
    }
}

bool runModule(const std::string& moduleName, const std::string& modulesDir) {
    std::string commandLine = "\"" + modulesDir + "\\" + moduleName + "\"";
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process in a separate window
    if (!CreateProcess(NULL,
                       const_cast<char*>(commandLine.c_str()),
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NEW_CONSOLE,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
        std::cerr << "Error running module: " << moduleName << std::endl;
        return false;
    }

    // We're not waiting for the process to finish
    // This allows the module to run in a new window

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

int main() {
    const std::string modulesDir = ".\\modules"; // Current directory
    std::string configFile = "settings\\module_list.conf";
    std::vector<std::string> modules = readModuleList(configFile);

    if (modules.empty()) {
        std::cerr << "No modules found in configuration file." << std::endl;
        return 1;
    }

    int choice;
    while (true) {
        displayMenu(modules);
        std::cout << "Enter the number of the module to run (0 to exit): ";
        std::cin >> choice;

        if (std::cin.fail() || choice < 0 || choice > modules.size()) {
            std::cerr << "Invalid selection. Please try again." << std::endl;
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
        } else if (choice == 0) {
            break;
        } else {
            runModule(modules[choice - 1], modulesDir);
        }
    }

    return 0;
}
