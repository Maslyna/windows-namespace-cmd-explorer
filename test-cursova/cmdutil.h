#pragma once

#include <tchar.h>
#include <Windows.h>

class CommandLineParser {
public:
    int GetArgParam(int argc, _TCHAR** argv, PCWSTR elem, PCWSTR& result) {
        for (int i = 1; i < argc; i++) {
            if (_tcscmp(argv[i], elem) == 0 && i + 1 < argc) {
                result = argv[i + 1];
                return 0;
            }
        }
        return 1;
    }

    bool ArgParamExists(int argc, _TCHAR** argv, PCWSTR elem) {
        for (int i = 1; i < argc; i++) {
            if (_tcscmp(argv[i], elem) == 0) {
                return true;
            }
        }
        return false;
    }
};