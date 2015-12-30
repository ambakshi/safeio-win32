#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>
#include <stdint.h>
#include <assert.h>

#ifndef _WIN64
#error "Only amd64 is supported"
#endif

#define uint uint32_t

#define WIN_THREAD_LIMIT 16
static volatile int32_t s_winThreadLock;
static DWORD_PTR s_affinityMaskForProcess;
static DWORD_PTR s_affinityMaskForCpu[WIN_THREAD_LIMIT];
static uint s_cpuCount;

int main(int, const char **)
{
    DWORD_PTR processAffinityMask;
    DWORD_PTR systemAffinityMask;
    DWORD threadAffinityMask;
    DWORD affinityMaskBits[32];
    uint cpuCount;
    uint cpuIndex;

    printf("sizeof(PDWORD_PTR) = %lu\n", sizeof(DWORD_PTR));
    printf("sizeof(DWORD) = %lu\n", sizeof(DWORD));
    printf("sizeof(int) = %lu\n", sizeof(int));
    printf("sizeof(DWORD *) = %lu\n", sizeof(DWORD *));

    if (!GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask)) {
        DWORD err = GetLastError();
        fprintf(stderr, "GetProcessAffinityMask failed with code %d\n", err);
        return 2;
    }
    s_affinityMaskForProcess = processAffinityMask;

    printf("procAff=%llu\n", processAffinityMask);
    printf("sysAff =%llu\n", systemAffinityMask);

    cpuCount = 0;
    for (threadAffinityMask = 1; (~threadAffinityMask + 1) & processAffinityMask; threadAffinityMask <<= 1) {
        if (threadAffinityMask & processAffinityMask) {
            affinityMaskBits[cpuCount] = threadAffinityMask;
            cpuCount++;
            if (cpuCount == (sizeof(affinityMaskBits) / sizeof(affinityMaskBits[0])))
                break;
        }
    }

    printf("cpuCount = %d\n", cpuCount);

    uint i;
    for (i = 0; i < cpuCount; i++) {
        printf("affinityMaskBits[%d] = %lu\n", i, affinityMaskBits[i]);
    }
    if (cpuCount == 0 || cpuCount == 1) {
        printf("%s\n", "STEAM_CEG_TEST_SECRET();");
        s_cpuCount = 1;
        s_affinityMaskForCpu[0] = ~0u;
        return 0;
    }

    assert(WIN_THREAD_LIMIT == 16);
    s_cpuCount = cpuCount < WIN_THREAD_LIMIT ? cpuCount : WIN_THREAD_LIMIT;

    s_affinityMaskForCpu[0] = affinityMaskBits[0];
    s_affinityMaskForCpu[1] = affinityMaskBits[cpuCount - 1];

    for (cpuIndex = 2; cpuIndex < s_cpuCount; cpuIndex++)
        s_affinityMaskForCpu[cpuIndex] = affinityMaskBits[cpuIndex - 1];
    printf("\n");

    for (i = 0; i < s_cpuCount; i++) {
        printf("s_affinityMaskForCpu[%d] = %lu\n", i, s_affinityMaskForCpu[i]);
    }
}
