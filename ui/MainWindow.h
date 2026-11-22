// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#pragma once

#include <QMainWindow>
#include <QString>

#include <atomic>
#include <memory>
#include <thread>

#include "opendepot/core/adk.h"

class QAction;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QProgressDialog;
class QStackedWidget;
class QTableWidget;
class QTimer;

struct RadioJob {
    std::atomic<bool> done{false};
    bool ok = false;
    bool isWrite = false;
    QString error;
    opendepot::Codeplug cp;
    std::thread thread;

    ~RadioJob() {
        if (thread.joinable()) thread.join();
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openCodeplug();
    void saveCodeplugAs();
    void refreshViews();
    void generateReport();
    void runDoctor();
    void compareCodeplug();
    void readRadio();
    void writeRadio();
    void pollRadio();
    void about();

private:
    QWidget* buildDashboard();
    QWidget* buildTextPage(QPlainTextEdit** out, const QString& button,
                           void (MainWindow::*handler)());
    QWidget* buildRadioPage();
    void setupMenu();
    void setCodeplug(opendepot::Codeplug cp, const QString& path);
    void showBusy(const QString& text);
    void finishBusy();
    void appendRadioLog(const QString& line);

    opendepot::Codeplug cp_;
    bool have_ = false;
    QString path_;

    QListWidget* nav_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QLabel* identity_ = nullptr;
    QLabel* flashcode_ = nullptr;
    QLabel* counts_ = nullptr;
    QTableWidget* zones_ = nullptr;
    QPlainTextEdit* report_ = nullptr;
    QPlainTextEdit* doctor_ = nullptr;
    QPlainTextEdit* diff_ = nullptr;
    QLineEdit* port_ = nullptr;
    QPlainTextEdit* radioLog_ = nullptr;
    QProgressDialog* busy_ = nullptr;
    QTimer* poll_ = nullptr;
    QAction* saveAct_ = nullptr;
    std::unique_ptr<RadioJob> job_;
};
