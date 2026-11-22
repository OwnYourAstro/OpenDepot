// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include <QApplication>
#include <QFile>
#include <QIcon>

#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("OpenDepot");
    app.setOrganizationName("OpenDepot");
    app.setWindowIcon(QIcon(":/icon.ico"));

    QFile theme(":/theme.qss");
    if (theme.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(theme.readAll()));

    MainWindow window;
    window.show();
    if (argc > 1) window.openFile(QString::fromLocal8Bit(argv[1]));
    return app.exec();
}
