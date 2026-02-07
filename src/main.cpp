#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>

namespace fs = std::filesystem;

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
    std::cout << YELLOW << "          --- A KurtaySoft Forensic Utility ---" << RESET << "\n\n";
}

void showWarnings()
{
    system("cls");
    std::cout << YELLOW << "===========================================================================" << "\n";
    std::cout << "        SECURE SHREDDER PRO - OFFICIAL WARNINGS & DOCUMENTATION            " << "\n";
    std::cout << "                      A KURTAYSOFT UTILITY                                 " << "\n";
    std::cout << "===========================================================================" << RESET << "\n\n";

    std::cout << BOLD << "1. PERMANENT DATA LOSS" << RESET << "\n";
    std::cout << "This utility is designed for IRREVERSIBLE data destruction. Once a file \n"
                 "has been shredded using either the Standard or Forensic pass, it cannot \n"
                 "be recovered by the Windows Recycle Bin, professional undelete software, \n"
                 "or hardware-based forensic lab recovery.\n\n";

    std::cout << BOLD << "2. SSD AND NVMe WEAR (IMPORTANT)" << RESET << "\n";
    std::cout << "Modern Solid State Drives (SSDs) and NVMe drives have a finite number of \n"
                 "write cycles (TBW - Total Bytes Written).\n"
                 "- Level 1 (Standard Purge) is the recommended setting for flash storage.\n"
                 "- Level 2 (Forensic Shred/3-Pass) performs triple the write operations.\n"
                 "Frequent use of 3-Pass shredding on SSDs will accelerate drive wear\n"
                 "and is generally unnecessary for flash-based media due to how controller-level Wear Leveling works.\n\n";

    std::cout << BOLD << "3. DOD 5220.22-M COMPLIANCE" << RESET << "\n";
    std::cout << "The 3-Pass \"Forensic\" mode follows the logic of the Department of Defense \n"
                 "5220.22-M standard. It is designed to mitigate \"Magnetic Force Microscopy\"\n"
                 "recovery on older Hard Disk Drives (HDDs).\n"
                 "If you are using a traditional mechanical spinning drive, this is your safest option for total privacy.\n\n";

    std::cout << BOLD << "4. SYSTEM CRITICAL FILES" << RESET << "\n";
    std::cout << "Secure Shredder Pro includes a safety filter for 'C:\\Windows' and \n"
                 "'System32' directories. However, users should never attempt to shred \n"
                 "active system files. Doing so may result in a non-bootable OS (BSoD).\n\n";

    std::cout << BOLD << "5. LEGAL DISCLAIMER" << RESET << "\n";
    std::cout << "KurtaySoft provides this tool \"as is\" without warranty of any kind. \n"
                 "By using this software, you assume all risk for accidental data loss.\n\n";

    std::cout << YELLOW << "===========================================================================" << RESET << "\n";
    std::cout << "Copyright (c) 2026 | Produced by KurtaySoft\n";
    std::cout << "Status: V0.101-BETA | Support: github.com/Kurtaysoft\n";
    std::cout << "===========================================================================" << "\n\n";
    std::cout << "Press Enter to return to menu...";
    std::cin.ignore();
    std::cin.get();
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
    std::cout << "  1. Integrate into Right-Click Menu\n";
    std::cout << "  2. Read Security Warnings & Documentation\n";
    std::cout << "  3. Uninstall / Remove from Menu\n";
    std::cout << "  4. Exit\n  Choice: ";
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
        std::cout << GREEN << "\n  Success! Integrated." << RESET << std::endl;
    }
    else if (c == 2)
    {
        showWarnings();
        printBanner();
        handleIntegration();
    }
    else if (c == 3)
    {
        RegDeleteTreeA(HKEY_CURRENT_USER, keyPath.c_str());
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        std::cout << YELLOW << "\n  Removed." << RESET << std::endl;
    }
}

int main(int argc, char *argv[])
{
    printBanner();
    if (argc < 2)
    {
        handleIntegration();
        return 0;
    }
    std::vector<fs::path> targets;
    for (int i = 1; i < argc; ++i)
        targets.push_back(argv[i]);
    std::cout << BOLD << "  [ SESSION START ]" << RESET << "\n";
    std::cout << "  OBJECTS TO PURGE : " << targets.size() << "\n\n";
    for (const auto &t : targets)
        if (isSystemCritical(t))
        {
            std::cout << RED << BOLD << "  [!] SYSTEM PATH DETECTED: " << t.filename() << RESET << "\n  Continue? (y/n): ";
            char c;
            std::cin >> c;
            if (c != 'y' && c != 'Y')
                return 0;
        }
    std::cout << RED << "  Destroy data permanently? (y/n): " << RESET;
    char confirm;
    std::cin >> confirm;
    if (confirm != 'y' && confirm != 'Y')
        return 0;
    std::cout << "\n  [ SECURITY LEVEL ]\n";
    std::cout << "  1. Standard Purge (1-Pass) - Recommended for SSD/NVMe longevity.\n";
    std::cout << "  2. Forensic Shred (3-Pass) - DoD 5220.22-M Standard. (Higher Drive Wear)\n";
    std::cout << "  Selection: ";
    int mode;
    std::cin >> mode;
    int passes = (mode == 2) ? 3 : 1;
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
    ShredResult results;
    for (const auto &target : targets)
    {
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
    }
    SetThreadExecutionState(ES_CONTINUOUS);
    MessageBeep(MB_OK);
    std::cout << GREEN << "\n\n  [ OPERATION COMPLETE ]" << RESET << "\n  Purged: " << results.successCount << " file(s)." << std::endl;
    if (!results.failedFiles.empty())
    {
        std::ofstream log("shred_log.txt");
        for (const auto &f : results.failedFiles)
            log << f << std::endl;
        std::cout << RED << "  Check shred_log.txt for errors." << RESET << std::endl;
    }
    std::cout << "\n  Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
    return 0;
}