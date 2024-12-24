#pragma once

#include <windows.h>
#include <detours.h>


class DetoursTransaction {
public:
    DetoursTransaction() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
    }
    ~DetoursTransaction() {
        DetourTransactionCommit();
    }
    DetoursTransaction(const DetoursTransaction&) = delete;
    DetoursTransaction(DetoursTransaction&&) = delete;
};
