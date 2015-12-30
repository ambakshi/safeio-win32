// GetDriveGeometry taken from Microsoft sample:
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363147(v=vs.85).aspx
//
// GetFsStats - Show filesystem stats using DeviceIoControl
// Run the program with the path to a file on the volume you wish to have stats for
//
// Amit Bakshi
// Dec 2015


#define UNICODE 1
#define _UNICODE 1

/* The code of interest is in the subroutine GetDriveGeometry. The
code in main shows how to interpret the results of the call. */

#include <sdkddkver.h>
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

#define wszDrive L"\\\\.\\PhysicalDrive0"


typedef struct {
    FILESYSTEM_STATISTICS fss;
    NTFS_STATISTICS ntfss;
    char pad0[320 - 272]; // pad to multiple of 64
} FILESYSTEM_NTFS;

DWORD GetFsStats(LPWSTR wszPath, FILESYSTEM_NTFS *st)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE; // handle to the drive to be examined
    BOOL bResult = FALSE;                  // results flag
    DWORD junk = 0;                        // discard results
    DWORD err = 0;

    hDevice = CreateFileW(wszPath,          // drive to open
                          0,                // no access to the drive
                          FILE_SHARE_READ | // share mode
                          FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          NULL,                       // default security attributes
                          OPEN_EXISTING,              // disposition
                          FILE_FLAG_BACKUP_SEMANTICS, // file attributes
                          NULL);                      // do not copy file attributes

    if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
    {
        return GetLastError();
    }

    bResult = DeviceIoControl(hDevice,                         // device to be queried
                              FSCTL_FILESYSTEM_GET_STATISTICS, // operation to perform
                              NULL, 0,                         // no input buffer
                              st, sizeof(*st),                 // output buffer
                              &junk,                           // # bytes returned
                              (LPOVERLAPPED)NULL);             // synchronous I/O

    if (!bResult) {
        err = GetLastError();
    }

    CloseHandle(hDevice);

    return (err);
}


DWORD GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY *pdg)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE; // handle to the drive to be examined
    BOOL bResult = FALSE;                  // results flag
    DWORD junk = 0;                        // discard results
    DWORD err = 0;

    hDevice = CreateFileW(wszPath,          // drive to open
                          0,                // no access to the drive
                          FILE_SHARE_READ | // share mode
                          FILE_SHARE_WRITE,
                          NULL,                       // default security attributes
                          OPEN_EXISTING,              // disposition
                          FILE_FLAG_BACKUP_SEMANTICS, // file attributes
                          NULL);                      // do not copy file attributes

    if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
    {
        return GetLastError();
    }

    bResult = DeviceIoControl(hDevice,                       // device to be queried
                              IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
                              NULL, 0,                       // no input buffer
                              pdg, sizeof(*pdg),             // output buffer
                              &junk,                         // # bytes returned
                              (LPOVERLAPPED)NULL);           // synchronous I/O

    if (!bResult) {
        err = GetLastError();
    }
    CloseHandle(hDevice);

    return (err);
}

int wmain(int argc, wchar_t *argv[])
{
    DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure
    DWORD bResult;             // GetLastError from function call
    ULONGLONG DiskSize = 0;    // size of the drive, in bytes

    wchar_t *drive_arg = argc > 1 ? argv[1] : wszDrive;

    bResult = GetDriveGeometry(drive_arg, &pdg);

    if (bResult == 0) {
        wprintf(L"Drive path      = %ws\n", wszDrive);
        wprintf(L"Cylinders       = %I64d\n", pdg.Cylinders);
        wprintf(L"Tracks/cylinder = %ld\n", (ULONG)pdg.TracksPerCylinder);
        wprintf(L"Sectors/track   = %ld\n", (ULONG)pdg.SectorsPerTrack);
        wprintf(L"Bytes/sector    = %ld\n", (ULONG)pdg.BytesPerSector);

        DiskSize = (ULONGLONG)pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
                   (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
        wprintf(L"Disk size       = %I64d (Bytes)\n"
                L"                = %.2f (Gb)\n",
                DiskSize, (double)DiskSize / (1024 * 1024 * 1024));
    } else {
        fwprintf(stderr, L"GetDriveGeometry failed. Error %ld.\n", bResult);
    }


    FILESYSTEM_NTFS fss = { 0 };
    bResult = GetFsStats(argc > 1 ? argv[1] : L"C:\\Windows\\system.ini", &fss);
    if (bResult == 0) {
        wprintf(L"Filesystem         = %ws\n", argc > 1 ? argv[1] : L"C:\\Windows\\system.ini");
        wprintf(L"FilesystemType     = %ld\n", (ULONG)fss.fss.FileSystemType);
        wprintf(L"UserFileReads      = %ld\n", (ULONG)fss.fss.UserFileReads);
        wprintf(L"UserFileReadBytes  = %.2f (Gb)\n", (double)fss.fss.UserFileReadBytes / (1024 * 1024 * 1024));
        wprintf(L"UserFileWrites     = %ld\n", (ULONG)fss.fss.UserFileWrites);
        wprintf(L"UserFileWriteBytes = %.2f (Gb)\n", (double)fss.fss.UserFileWriteBytes / (1024 * 1024 * 1024));
        wprintf(L"UserDiskReads      = %ld\n", (ULONG)fss.fss.UserDiskReads);
        wprintf(L"UserDiskWrites     = %ld\n", (ULONG)fss.fss.UserDiskWrites);
        wprintf(L"MetaDataDiskReads  = %ld\n", (ULONG)fss.fss.MetaDataDiskReads);
        wprintf(L"MetaDataDiskWrites = %ld\n", (ULONG)fss.fss.MetaDataDiskWrites);

    } else {
        fwprintf(stderr, L"GetFsStatsss failed. Error %ld.\n", bResult);
    }


    return (int)bResult;
}
