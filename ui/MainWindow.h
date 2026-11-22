// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#pragma once

#include <QDateTime>
#include <QMainWindow>
#include <QString>

#include <atomic>
#include <memory>
#include <thread>

#include "opendepot/core/adk.h"

class QAction;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressDialog;
class QScrollArea;
class QStackedWidget;
class QTableWidget;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

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

    void openFile(const QString& file);

private slots:
    void openCodeplug();
    void saveCodeplugAs();
    void generateReport();
    void runDoctor();
    void compareCodeplug();
    void showRecords();
    void showZones();
    void readRadio();
    void writeRadio();
    void pollRadio();
    void about();

private:
    void setupMenu();
    void setupUi();
    void buildTree();
    QWidget* buildRadioInfoPage();
    QWidget* buildTextPage(const QString& title, QPlainTextEdit** out, const QString& button,
                           void (MainWindow::*handler)());
    QWidget* buildRadioPage();
    QWidget* makeCard(const QString& caption, QLabel** value, const QString& initial = "-");
    void setCodeplug(opendepot::Codeplug cp, const QString& path);
    void updateDashboard();
    void fillZones(QTableWidget* table, bool groupRows);
    void setModelStatus(const QString& text);
    void showBusy(const QString& text);
    void finishBusy();
    void appendRadioLog(const QString& line);

    opendepot::Codeplug cp_;
    bool have_ = false;
    QString path_;
    QDateTime openedAt_;

    QTreeWidget* tree_ = nullptr;
    QTreeWidgetItem* treeRoot_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QScrollArea* infoScroll_ = nullptr;
    QLabel* subtitle_ = nullptr;
    QLabel* serial_ = nullptr;
    QLabel* model_ = nullptr;
    QLabel* flashcode_ = nullptr;
    QLabel* headerVersion_ = nullptr;
    QLabel* recordCount_ = nullptr;
    QLabel* passcode_ = nullptr;
    QLabel* codeplugFile_ = nullptr;
    QLabel* modelStatus_ = nullptr;
    QWidget* chips_ = nullptr;
    QGridLayout* chipsLayout_ = nullptr;
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
