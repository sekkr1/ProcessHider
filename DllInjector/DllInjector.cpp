// DllInjector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <psapi.h>
#include <TlHelp32.h>

#include "inject.h"

int main()
{
    /*DWORD processes[1024];
    DWORD cbNeeded;

    if (0 == EnumProcesses(processes, sizeof(processes), &cbNeeded))
        return 1;
        */
    std::cout << "Searching Taskmgr.exe process..." << std::endl;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot)
        return 1;

    PROCESSENTRY32 process;
    process.dwSize = sizeof(PROCESSENTRY32);
    BOOL lastProcessRetval = Process32First(snapshot, &process);
    while (TRUE == lastProcessRetval) {
        if (0 == strcmp(process.szExeFile, "Taskmgr.exe"))
            break;
        lastProcessRetval = Process32Next(snapshot, &process);
    }

    if (FALSE == lastProcessRetval)
        return 1;

    std::cout << "Injecting DLL..." << std::endl;
    injectDll(process.th32ProcessID, "C:\\Users\\dekel\\source\\repos\\DllInjector\\x64\\Debug\\InjectedDll.dll");
    return 0;
}
