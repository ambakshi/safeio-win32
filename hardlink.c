//
//
// Safely create hardlinks that can overwrite their target
// Amit Bakshi
// Dec 2015
//
//
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


unsigned hardlink(const char *dst, const char *src)
{
    char tmpnam[MAX_PATH];
    unsigned err = 0;

    _snprintf_s(tmpnam, MAX_PATH, _TRUNCATE, "%s%x", dst, GetCurrentThreadId());

    if (!CreateHardLinkA(tmpnam, src, NULL)) {
        err = GetLastError();
        goto hardlink_error;
    }
    if (MoveFileEx(tmpnam, dst, MOVEFILE_REPLACE_EXISTING)) {
        return 0;
    }
    err = GetLastError();
    if (err == ERROR_ACCESS_DENIED) {
        DWORD attr = GetFileAttributes(dst);
        bool is_readonly = (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_READONLY);
        if (is_readonly) {
            if (!SetFileAttributes(dst, attr & (~FILE_ATTRIBUTE_READONLY))) {
                err = GetLastError();
                goto hardlink_error;
            }
        }
        if (!MoveFileEx(tmpnam, dst, MOVEFILE_REPLACE_EXISTING)) {
            err = GetLastError();
            if (is_readonly) {
                SetFileAttributes(dst, attr);
            }
            goto hardlink_error;
        }
        return 0;
    }

hardlink_error:
    DeleteFile(tmpnam);
    fprintf(stderr, "CreateHardLinkA: Couldn't link '%s' to '%s' (0x%08x)\n", dst, src, err);
    return err;
}


unsigned main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s dstfile srcfile\n\nexample:\n\t%s target.bin source.bin\n\n",
                argv[0], argv[0]);
        return 2;
    }
    return hardlink(argv[1], argv[2]);
}
