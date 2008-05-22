#pragma once

extern HINSTANCE g_hInstance;

typedef std::list<CString> StrList;

enum FILE_STATUS {
    FS_UNCHECKED,
    FS_CHECKED,
    FS_MISSING
};

typedef struct _FILE_ENTRY {
    CString strFileName;
    DWORD dwExpectedCRC;
    DWORD dwFileCRC;
    bool bCheck;
    bool bError;
    FILE_STATUS fs;
} FILE_ENTRY;

typedef std::vector<FILE_ENTRY *> FileList;
