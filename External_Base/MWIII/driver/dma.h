#pragma once
#include "vmmdll.h"
#include "leechcore.h"
#pragma comment(lib,"leechcore.lib")
#pragma comment(lib,"vmm.lib")

#define REMOTE_PROCESS_SHORT "cod.exe"
#define REMOTE_PROCESS_LONG L"cod.exe"

namespace DMA
{
    extern VMM_HANDLE hVMM;
    extern bool Connected;
    extern uint32_t AttachedProcessId;
    extern uint64_t BaseAddress;
    extern uint64_t PebAddress;

    bool Connect();
    void Disconnect();
    bool AttachToProcessId(LPSTR szProcessName = const_cast<LPSTR>(REMOTE_PROCESS_SHORT));
    bool GetPEBAddress(uint32_t dwProcessId);

    template<typename Var, typename U>
    Var Read(U address, size_t uiSize, bool bFullReadRequired = true)
    {
        Var output{};
        if (!AttachedProcessId || !Connected || !address) {
            return output;
        }

        uint32_t bytesRead = 0;
        uint32_t flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO;

        BOOL bRetn = (VMMDLL_MemReadEx(hVMM, AttachedProcessId, (uint64_t)address, (uint8_t*)&output, uiSize,
            reinterpret_cast<PDWORD>(&bytesRead), flags) && bytesRead != 0);

        if (!bRetn || (bFullReadRequired && bytesRead != uiSize)) {
            return output;
        }

        return output;
    }

    // ScatterMemory class definition
    class ScatterMemory
    {
    public:
        VMMDLL_SCATTER_HANDLE Initialize()
        {
            return VMMDLL_Scatter_Initialize(hVMM, AttachedProcessId, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING_IO);
        }

        bool Clear(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            return VMMDLL_Scatter_Clear(hSCATTER, AttachedProcessId, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING_IO);
        }

        void Close(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            VMMDLL_Scatter_CloseHandle(hSCATTER);
        }

        bool ExecuteRead(VMMDLL_SCATTER_HANDLE hSCATTER)
        {
            return VMMDLL_Scatter_ExecuteRead(hSCATTER);
        }

        template<typename U, typename P>
        bool PrepareEX(VMMDLL_SCATTER_HANDLE hSCATTER, U vAddress, P pOutput, size_t uiSize)
        {
            if (!hSCATTER || !vAddress || !uiSize) {
                return false;
            }

            return VMMDLL_Scatter_PrepareEx(hSCATTER, vAddress, uiSize, (uint8_t*)pOutput, NULL);
        }
    };
}