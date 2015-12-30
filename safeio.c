//
// Safe/atomic file i/o routines for environments where
// multiple threads/processes can write to the same
// output file. This can happen in build systems where the
// dependencies aren't known (or it is too expensive to find
// out), and rather than building things once it's easier/faster
// to take the hit of doing some work multiple times.
//
// Amit Bakshi
// Jan 2015
//
//
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>

DWORD safeio_printerr(DWORD err, const char *desc, const char *path)
{
    char msgBuf[1024] = "";

    if (err == ERROR_SUCCESS)
        return ERROR_SUCCESS;

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msgBuf, sizeof(msgBuf), NULL);

    fprintf(stderr, "ERROR[0x%08X] [%d] %s: %s. %s\n", err, GetCurrentThreadId(), desc,
            path ? path : "<none>", msgBuf);
    return err;
}

DWORD sfrename(FILE *fp, const char *path)
{
    DWORD err = ERROR_SUCCESS;
    FILE_RENAME_INFO *finfo;
    HANDLE fh;
    const size_t plen = strlen(path);
    const DWORD nbytes = (DWORD)(sizeof(FILE_RENAME_INFO) + sizeof(WCHAR) * plen);

    int fd = _fileno(fp);
    if (fd == -1) {
        return ERROR_INVALID_HANDLE;
    }
    fh = (HANDLE)_get_osfhandle(fd);
    if (fh == INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    finfo = (FILE_RENAME_INFO *)malloc(nbytes);
    assert(finfo);
    swprintf(finfo->FileName, sizeof(WCHAR) * (plen + 1), L"%hs", path);
    finfo->FileNameLength = (ULONG)(sizeof(WCHAR) * plen);
    finfo->ReplaceIfExists = TRUE;
    finfo->RootDirectory = NULL;

    if (!SetFileInformationByHandle(fh, FileRenameInfo, finfo, nbytes)) {
        err = safeio_printerr(GetLastError(), "sfcommit", path);
    }
    free(finfo);
    return err;
}

FILE *sfopen(const char *path, const char *mode)
{
    char tmpfile[MAX_PATH];
    assert(mode && mode[0] == 'w');

    int omode = mode[1] == 'b' ? _O_BINARY : _O_TEXT;

    _snprintf(tmpfile, MAX_PATH, "%s%04x", path, GetCurrentThreadId() & 0xffff);

    HANDLE fh = CreateFileA(tmpfile, GENERIC_ALL, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    int fd = _open_osfhandle((intptr_t)fh, omode);
    if (fd == -1) {
        CloseHandle(fh);
        return NULL;
    }

    FILE *fp = _fdopen(fd, mode);
    if (!fp) {
        _close(fd); // This will close the underlying handle
        return NULL;
    }
    return fp;
}
