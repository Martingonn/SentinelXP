#include <windows.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <direct.h>  // _mkdir
#include <string.h>  // strcmp

#define MAX_SHOW 10

struct FileEntry {
    std::string path;
    ULONGLONG size;
};

struct DirEntry {
    std::string path;
    ULONGLONG size;
};

// Case-insensitive string compare for Windows XP and C++98 (no strcasecmp)
int stricmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = *s1 >= 'A' && *s1 <= 'Z' ? *s1 + 32 : *s1;
        char c2 = *s2 >= 'A' && *s2 <= 'Z' ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        ++s1; ++s2;
    }
    return *s1 - *s2;
}

// Create directory if it doesn't exist (relative path)
void ensureDirExists(const char* dir) {
    DWORD attr = GetFileAttributesA(dir);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        if (_mkdir(dir) != 0) {
            // Could not create directory, ignore if it exists now
        }
    }
}

// Recursively scan directory and collect file and directory sizes
void scanDir(const std::string& root, std::vector<FileEntry>& files, std::vector<DirEntry>& dirs, ULONGLONG& totalSize) {
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    std::string pattern = root + "\\*";
    ULONGLONG dirSize = 0;
    hFind = FindFirstFileA(pattern.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        std::string fullPath = root + "\\" + ffd.cFileName;
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ULONGLONG subDirSize = 0;
            scanDir(fullPath, files, dirs, subDirSize);
            dirs.push_back(DirEntry());
            dirs.back().path = fullPath;
            dirs.back().size = subDirSize;
            dirSize += subDirSize;
        } else {
            ULONGLONG fsize = ((ULONGLONG)ffd.nFileSizeHigh << 32) + ffd.nFileSizeLow;
            files.push_back(FileEntry());
            files.back().path = fullPath;
            files.back().size = fsize;
            dirSize += fsize;
        }
    } while (FindNextFileA(hFind, &ffd) != 0);
    FindClose(hFind);
    totalSize += dirSize;
}

// Sorting comparison functions
bool fileEntryCmp(const FileEntry& a, const FileEntry& b) {
    return a.size > b.size;
}
bool dirEntryCmp(const DirEntry& a, const DirEntry& b) {
    return a.size > b.size;
}

// Print file list with sizes
void printFileList(const std::vector<FileEntry>& files, int start, int count) {
    int end = start + count;
    if (end > (int)files.size()) end = (int)files.size();
    for (int i = start; i < end; ++i) {
        printf("%3d. %-60s %10.2f MB\n", i + 1, files[i].path.c_str(), files[i].size / (1024.0 * 1024.0));
    }
}

// Print directory list with sizes
void printDirList(const std::vector<DirEntry>& dirs, int start, int count) {
    int end = start + count;
    if (end > (int)dirs.size()) end = (int)dirs.size();
    for (int i = start; i < end; ++i) {
        printf("%3d. %-60s %10.2f MB\n", i + 1, dirs[i].path.c_str(), dirs[i].size / (1024.0 * 1024.0));
    }
}

// Append session log to file
void saveLog(const std::vector<FileEntry>& files, const std::vector<DirEntry>& dirs, ULONGLONG totalSize) {
    ensureDirExists("logs");
    FILE* f = fopen("logs\\storage_log.txt", "a");
    if (!f) {
        printf("Failed to open logs\\storage_log.txt for appending.\n");
        return;
    }
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "Session on %04d-%02d-%02d %02d:%02d:%02d\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    fprintf(f, "Total size: %.2f MB\n", totalSize / (1024.0 * 1024.0));
    fprintf(f, "Largest files:\n");
    int n = (files.size() < 100) ? files.size() : 100;
    for (int i = 0; i < n; ++i)
        fprintf(f, "%3d. %s|%.2f\n", i + 1, files[i].path.c_str(), files[i].size / (1024.0 * 1024.0));
    fprintf(f, "Largest dirs:\n");
    n = (dirs.size() < 100) ? dirs.size() : 100;
    for (int i = 0; i < n; ++i)
        fprintf(f, "%3d. %s|%.2f\n", i + 1, dirs[i].path.c_str(), dirs[i].size / (1024.0 * 1024.0));
    fprintf(f, "ENDSESSION\n");
    fflush(f);
    fclose(f);
}

// Read previous total size from file
bool getPreviousSize(ULONGLONG& lastTotal) {
    FILE* f = fopen("logs\\previous_size.txt", "r");
    if (!f) return false;
    ULONGLONG size = 0;
    if (fscanf(f, "%llu", &size) == 1) {
        lastTotal = size;
        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

// Save current total size to file (overwrites or creates)
void savePreviousSize(ULONGLONG size) {
    ensureDirExists("logs");
    FILE* f = fopen("logs\\previous_size.txt", "wb");
    if (!f) {
        printf("Failed to open logs\\previous_size.txt for writing.\n");
        return;
    }
    if (fprintf(f, "%llu\n", size) < 0) {
        printf("Failed to write to logs\\previous_size.txt.\n");
    }
    fflush(f);
    fclose(f);
    printf("Saved previous size to logs\\previous_size.txt\n");
}

int main() {
    char rootDir[260] = {0};
    printf("Enter directory to scan (e.g. C:\\ or C:\\Folder). Enter 'nc' to skip overwriting previous_size.txt: ");
    if (!fgets(rootDir, sizeof(rootDir), stdin)) {
        printf("Failed to read input.\n");
        return 1;
    }

    // Trim trailing newline characters
    int len = (int)strlen(rootDir);
    while (len > 0 && (rootDir[len - 1] == '\n' || rootDir[len - 1] == '\r')) {
        rootDir[len - 1] = 0;
        --len;
    }
    if (len == 0) {
        printf("No directory entered. Exiting.\n");
        return 1;
    }

    // Check if user entered "nc" (case-insensitive)
    bool skipOverwrite = (len == 2 && (rootDir[0] == 'n' || rootDir[0] == 'N') && (rootDir[1] == 'c' || rootDir[1] == 'C'));

    if (skipOverwrite) {
        printf("Skipping overwriting previous_size.txt as requested.\n");
        printf("No directory scanned. Exiting.\n");
        return 0;
    }

    printf("Scanning directory: %s\n", rootDir);

    std::vector<FileEntry> files;
    std::vector<DirEntry> dirs;
    ULONGLONG totalSize = 0;

    scanDir(rootDir, files, dirs, totalSize);

    std::sort(files.begin(), files.end(), fileEntryCmp);
    std::sort(dirs.begin(), dirs.end(), dirEntryCmp);

    printf("\nTotal size: %.2f MB\n", totalSize / (1024.0 * 1024.0));

    ULONGLONG lastTotal = 0;
    if (getPreviousSize(lastTotal)) {
        double diffMB = (totalSize - lastTotal) / (1024.0 * 1024.0);
        double pct = lastTotal ? (100.0 * (totalSize - lastTotal) / lastTotal) : 0.0;
        printf("Since last check: %+.2f MB (%+.2f%%)\n", diffMB, pct);
    } else {
        printf("No previous size file found for comparison.\n");
    }

    printf("\nLargest files:\n");
    int fileShow = 0;
    char ch = 0;
    do {
        int showF = (fileShow + MAX_SHOW < (int)files.size()) ? MAX_SHOW : (int)files.size() - fileShow;
        printFileList(files, fileShow, showF);
        fileShow += showF;
        if (fileShow < (int)files.size()) {
            printf("Press E to show more files, C to cancel and exit, Enter to continue...\n");
            ch = getchar();
            while (ch == '\n' || ch == '\r') ch = getchar();
            if (ch == 'C' || ch == 'c') {
                printf("Exiting program as requested.\n");
                return 0;
            }
            if (ch != 'E' && ch != 'e') break;
        } else break;
    } while (fileShow < (int)files.size());

    printf("\nLargest directories:\n");
    int dirShow = 0;
    do {
        int showD = (dirShow + MAX_SHOW < (int)dirs.size()) ? MAX_SHOW : (int)dirs.size() - dirShow;
        printDirList(dirs, dirShow, showD);
        dirShow += showD;
        if (dirShow < (int)dirs.size()) {
            printf("Press E to show more directories, C to cancel and exit, Enter to finish...\n");
            ch = getchar();
            while (ch == '\n' || ch == '\r') ch = getchar();
            if (ch == 'C' || ch == 'c') {
                printf("Exiting program as requested.\n");
                return 0;
            }
            if (ch != 'E' && ch != 'e') break;
        } else break;
    } while (dirShow < (int)dirs.size());

    saveLog(files, dirs, totalSize);
    savePreviousSize(totalSize);

    printf("\nResults saved to logs\\storage_log.txt\n");
    printf("Previous size saved to logs\\previous_size.txt\n");

    printf("\nPress Enter to exit...");
    while ((ch = getchar()) != '\n' && ch != EOF) {} // consume until newline

    return 0;
}
