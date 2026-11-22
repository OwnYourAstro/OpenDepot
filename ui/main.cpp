// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include <QApplication>
#include <QFile>

#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("OpenDepot");
    app.setOrganizationName("OpenDepot");

    QFile theme(":/theme.qss");
    if (theme.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(theme.readAll()));

    MainWindow window;
    window.show();
    return app.exec();
}
