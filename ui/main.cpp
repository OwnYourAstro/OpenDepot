// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QTimer>

#include "MainWindow.h"

namespace {

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* info) {
    char path[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, path);
    if (n == 0 || n >= MAX_PATH) return EXCEPTION_EXECUTE_HANDLER;
    strncat_s(path, MAX_PATH, "opendepot_crash.txt", _TRUNCATE);
    FILE* f = nullptr;
    if (fopen_s(&f, path, "a") != 0 || !f) return EXCEPTION_EXECUTE_HANDLER;
    const HMODULE base = GetModuleHandleW(nullptr);
    const auto baseAddr = reinterpret_cast<unsigned long long>(base);
    std::fprintf(f, "CRASH code=%08lX address=%p rva=%llX base=%llX\n",
                 info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress,
                 static_cast<unsigned long long>(
                     reinterpret_cast<uintptr_t>(info->ExceptionRecord->ExceptionAddress) - baseAddr),
                 baseAddr);
    void* frames[48];
    const USHORT count = CaptureStackBackTrace(0, 48, frames, nullptr);
    for (USHORT i = 0; i < count; ++i) {
        const auto addr = reinterpret_cast<unsigned long long>(frames[i]);
        std::fprintf(f, "  frame %02u rva=%llX\n", i, addr - baseAddr);
    }
    std::fclose(f);
    return EXCEPTION_EXECUTE_HANDLER;
}

}  // namespace

int main(int argc, char** argv) {
    SetUnhandledExceptionFilter(CrashHandler);

    QApplication app(argc, argv);
    app.setApplicationName("OpenDepot");
    app.setOrganizationName("OpenDepot");
    app.setWindowIcon(QIcon(":/icon.ico"));

    QFile theme(":/theme.qss");
    if (theme.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(theme.readAll()));

    MainWindow window;
    window.show();

    QString file;
    QString section;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--section" && i + 1 < argc) {
            section = QString::fromLocal8Bit(argv[++i]);
        } else if (file.isEmpty()) {
            file = arg;
        }
    }
    if (!file.isEmpty()) window.openFile(file);
    if (!section.isEmpty()) window.selectSectionByName(section);

    bool cycle = false;
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--cycle") == 0) cycle = true;
    if (cycle) {
        static const char* kNames[] = {
            "Radio Information", "Radio Wide", "Test Channel List", "Narrow Bandwidth",
            "Radio Wide VRS-EP", "Controls", "Display & Menu", "Phone", "Secure",
            "Trunking Configuration", "Trunking System", "Trunking Personality", "Talkgroup",
            "Conventional Configuration", "MPL", "Conventional Personality", "Message Alias List",
            "Status Alias List", "MDC", "Quik-Call II", "DTMF", "Singletone", "ASTRO Systems",
            "Scan Configuration", "Scan List", "Zone Channel Assignment", "Codeplug Directory",
            "Passcode", "Radio Wide", "Phone", "Secure", "Display & Menu", "Talkgroup"};
        auto* timer = new QTimer(&app);
        auto* index = new int(0);
        const int total = int(sizeof(kNames) / sizeof(kNames[0]));
        QObject::connect(timer, &QTimer::timeout, [&window, timer, index, total] {
            if (*index >= total) {
                timer->stop();
                QCoreApplication::quit();
                return;
            }
            window.selectSectionByName(QString::fromLatin1(kNames[(*index)++]));
        });
        timer->start(250);
    }
    return app.exec();
}
