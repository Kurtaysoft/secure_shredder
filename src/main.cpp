#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>

namespace fs = std::filesystem;

// UI Color Codes
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

struct ShredResult
{
    int successCount = 0;
    std::vector<std::string> failedFiles;
};

void printBanner()
{
    system("cls");
    std::cout << CYAN << BOLD << R"(
  ____                                ____  _                        _     _           
 / ___|  ___  ___ _   _ _ __ ___  / ___|| |__  _ __ ___  __| | __| | ___ _ __ 
 \___ \ / _ \/ __| | | | '__/ _ \ \___ \| '_ \| '__/ _ \/ _` |/ _` |/ _ \ '__|
  ___) |  __/ (__| |_| | | |  __/  ___) | | | | | |  __/ (_| | (_| |  __/ |   
 |____/ \___|\___|\__,_|_|  \___| |____/|_| |_|_|  \___|\__,_|\__,_|\___|_|   
    )" << RESET
              << std::endl;
    std::cout << YELLOW << "      --- Forensic-Grade File Destruction Utility ---" << RESET << "\n\n";
}

void ghostMetadata(const fs::path &path)
{
    HANDLE hFile = CreateFileA(path.string().c_str(), FILE_WRITE_ATTRIBUTES,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FILETIME ft;
        SYSTEMTIME st;
        st.wYear = 1980;
        st.wMonth = 1;
        st.wDay = 1;
        st.wHour = 0;
        st.wMinute = 0;
        st.wSecond = 0;
        st.wMilliseconds = 0;
        SystemTimeToFileTime(&st, &ft);
        SetFileTime(hFile, &ft, &ft, &ft);
        CloseHandle(hFile);
    }
}

bool isSystemCritical(const fs::path &path)
{
    std::string p = path.string();
    return (p.find("Windows") != std::string::npos || p.find("System32") != std::string::npos || p.find("Program Files") != std::string::npos);
}

void printProgress(const std::string &filename, double percentage)
{
    int width = 30;
    std::string shortName = filename.length() > 15 ? filename.substr(0, 12) + "..." : filename;
    std::cout << "\r" << CYAN << "  Shredding: " << RESET << shortName << " [";
    int pos = (int)(width * percentage);
    for (int i = 0; i < width; ++i)
    {
        if (i < pos)
            std::cout << GREEN << "=";
        else if (i == pos)
            std::cout << GREEN << ">";
        else
            std::cout << " ";
    }
    std::cout << RESET << "] " << (int)(percentage * 100.0) << "%";
    std::cout.flush();
}

bool shredLogic(const fs::path &path, int passes)
{
    try
    {
        ghostMetadata(path);
        std::uintmax_t fileSize = fs::file_size(path);
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
            return false;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 255);

        for (int i = 0; i < passes; ++i)
        {
            file.seekp(0, std::ios::beg);
            for (std::uintmax_t j = 0; j < fileSize; ++j)
            {
                file.put(static_cast<char>(dist(gen)));
                if (j % 50000 == 0 || j == fileSize - 1)
                    printProgress(path.filename().string(), (double)j / fileSize);
            }
            file.flush();
        }
        file.close();
        fs::remove(path);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void handleIntegration()
{
    std::string keyPath = "Software\\Classes\\*\\shell\\SecureShredder";
    HKEY hKey;
    bool isInstalled = (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS);
    if (isInstalled)
        RegCloseKey(hKey);

    std::cout << BOLD << "  [ MANAGEMENT MODE ]" << RESET << "\n";
    std::cout << "  Status: " << (isInstalled ? GREEN "INSTALLED" : RED "NOT INSTALLED") << RESET << "\n\n";

    if (!isInstalled)
    {
        std::cout << "  1. Integrate into Right-Click Menu\n";
        std::cout << "  2. Exit\n  Choice: ";
        int c;
        std::cin >> c;
        if (c == 1)
        {
            char szPath[MAX_PATH];
            GetModuleFileNameA(NULL, szPath, MAX_PATH);
            std::string cmd = "\"" + std::string(szPath) + "\" \"%1\"";

            RegCreateKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
            RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)"Secure Shred File", 18);

            HKEY cKey;
            std::string cPath = keyPath + "\\command";
            RegCreateKeyExA(HKEY_CURRENT_USER, cPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &cKey, NULL);
            RegSetValueExA(cKey, NULL, 0, REG_SZ, (const BYTE *)cmd.c_str(), (DWORD)cmd.length() + 1);

            RegCloseKey(hKey);
            RegCloseKey(cKey);
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
            std::cout << GREEN << "\n  Success! Integration complete." << RESET << std::endl;
        }
    }
    else
    {
        std::cout << "  1. Update/Re-install (if you moved the file)\n";
        std::cout << "  2. Uninstall / Remove from Menu\n";
        std::cout << "  3. Exit\n  Choice: ";
        int c;
        std::cin >> c;
        if (c == 1)
        {
            char szPath[MAX_PATH];
            GetModuleFileNameA(NULL, szPath, MAX_PATH);
            std::string cmd = "\"" + std::string(szPath) + "\" \"%1\"";
            HKEY cKey;
            RegCreateKeyExA(HKEY_CURRENT_USER, (keyPath + "\\command").c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &cKey, NULL);
            RegSetValueExA(cKey, NULL, 0, REG_SZ, (const BYTE *)cmd.c_str(), (DWORD)cmd.length() + 1);
            RegCloseKey(cKey);
            std::cout << GREEN << "\n  Registry updated to current location." << RESET << std::endl;
        }
        else if (c == 2)
        {
            RegDeleteTreeA(HKEY_CURRENT_USER, keyPath.c_str());
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
            std::cout << YELLOW << "\n  Integration removed." << RESET << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    printBanner();

    if (argc < 2)
    {
        handleIntegration();
        std::cout << "\n  Press Enter to close...";
        std::cin.ignore();
        std::cin.get();
        return 0;
    }

    fs::path target(argv[1]);

    std::cout << BOLD << "  [ SESSION START ]" << RESET << "\n";
    std::cout << "  ------------------------------------" << "\n";
    std::cout << "  OBJECT    : " << target.filename().string() << "\n";
    if (fs::is_regular_file(target))
        std::cout << "  SIZE      : " << (fs::file_size(target) / 1024) << " KB\n";
    std::cout << "  ------------------------------------" << "\n\n";

    if (isSystemCritical(target))
    {
        std::cout << RED << BOLD << "  [!] CRITICAL SYSTEM PATH DETECTED!" << RESET << "\n";
        std::cout << "  Continue? (y/n): ";
        char c;
        std::cin >> c;
        if (c != 'y' && c != 'Y')
            return 0;
    }

    std::cout << RED << "  Destroy data? (y/n): " << RESET;
    char confirm;
    std::cin >> confirm;
    if (confirm != 'y' && confirm != 'Y')
        return 0;

    std::cout << "\n  Level: 1.Quick (SSD) | 2.Deep (Forensic)\n  Selection: ";
    int mode;
    std::cin >> mode;
    int passes = (mode == 2) ? 3 : 1;

    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);

    ShredResult results;
    if (fs::is_directory(target))
    {
        for (const auto &entry : fs::recursive_directory_iterator(target))
        {
            if (!fs::is_directory(entry))
            {
                if (shredLogic(entry.path(), passes))
                    results.successCount++;
                else
                    results.failedFiles.push_back(entry.path().string());
            }
        }
    }
    else
    {
        if (shredLogic(target, passes))
            results.successCount++;
        else
            results.failedFiles.push_back(target.string());
    }

    SetThreadExecutionState(ES_CONTINUOUS);
    MessageBeep(MB_OK);

    std::cout << GREEN << "\n\n  [ OPERATION COMPLETE ]" << RESET << std::endl;
    std::cout << "  Purged: " << results.successCount << " file(s)." << std::endl;

    if (!results.failedFiles.empty())
    {
        std::cout << RED << "  Failed: " << results.failedFiles.size() << RESET << std::endl;
        std::ofstream log("shred_log.txt");
        for (const auto &f : results.failedFiles)
            log << f << std::endl;
    }

    std::cout << "\n  Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
    return 0;
}