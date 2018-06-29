#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>

/*
 * Author : x0r19x91
 * Revealing the mines - Minesweeper
 */

#pragma comment(lib, "user32")

typedef LONG (WINAPI* NT_ROUTINE) (HANDLE);

int main(int argc, char** argv)
{
    int n_mines = 0, width, height, i;
    char* base;
    char row[32];
    HWND hWnd, hProcSnap, hProcess = (HANDLE)-1;
    char* exe_name;
    PROCESSENTRY32 pInfo;
    HANDLE hNt;
    NT_ROUTINE NtSuspendProcess, NtResumeProcess;
    
    exe_name = "winmine.exe";
    if (argc == 2)
    {
        exe_name = strlwr(argv[1]);
    }
    
    // already loaded, just return module base
    hNt = LoadLibrary("ntdll.dll");
    NtSuspendProcess = (NT_ROUTINE) GetProcAddress(hNt, "NtSuspendProcess");
    NtResumeProcess = (NT_ROUTINE) GetProcAddress(hNt, "NtResumeProcess");
    
    hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    Process32First(hProcSnap, &pInfo);
    do
    {
        if (strstr(strlwr(pInfo.szExeFile), exe_name))
        {
            hProcess = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_VM_READ, FALSE, pInfo.th32ProcessID);
            printf("[*] Found Minesweeper (%s) ... PiD = %d\n", pInfo.szExeFile, pInfo.th32ProcessID);
            break;
        }
    } while (Process32Next(hProcSnap, &pInfo));
    
    CloseHandle(hProcSnap);
    if (! hProcess)
    {
        printf("[-] Failed to open process !!\n");
        exit(2);
    }
    else if (hProcess == (HANDLE)-1)
    {
        printf("[ Usage ]\n    %s [minesweeper_binary_name]\n", *argv);
        printf("\n[ Note ]\n    Execute this program only after starting minesweeper\n");
        printf("    If no argument is provided, the program searches for WinMine.exe\n");
        printf("\n[~ x0r19x91 ~]\n");
        exit(0);
    }
    
    printf("[*] Waiting for Minesweeper window to appear ...\n");
    while (!(hWnd = FindWindow("Minesweeper", "Minesweeper")));
    printf("[*] HWnd : %p\n", (long*) hWnd);
    
    while (!n_mines)
        ReadProcessMemory(hProcess, (char*) 0x1005330, &n_mines, 4, NULL);
    
    NtSuspendProcess(hProcess);
    ReadProcessMemory(hProcess, (char*) 0x1005334, &width, 4, NULL);
    ReadProcessMemory(hProcess, (char*) 0x1005338, &height, 4, NULL);
    
    printf("[*] # mines : %d\n", n_mines);
    printf("[*] Dimensions : %d x %d\n", height, width);
    printf("[ Note ] X represents Mine\n");
    printf("[*] Mine Layout ...\n");
    base = (char*) 0x1005360;
    
    while (height--) {
        ReadProcessMemory(hProcess, base, row, 32, NULL);
        base += 0x20;
        i = 1;
        do {
            if ((row[i] & 0xffu) == 0x8f)
                row[i] = 'X';
            else
                row[i] = '.';
        } while (row[++i] != 0x10);
        printf("%.*s\n", width, row+1);
    }
    
    NtResumeProcess(hProcess);
    WaitForSingleObject(hProcess, -1);
    CloseHandle(hProcess);
}