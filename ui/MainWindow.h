// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#pragma once

#include <QDateTime>
#include <QHash>
#include <QMainWindow>
#include <QString>

#include <atomic>
#include <memory>
#include <thread>

#include "opendepot/core/adk.h"
#include "opendepot/core/model.h"

class QAction;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressDialog;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTableWidgetItem;
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
    void saveCodeplug();
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
    void buildModelTree();
    void navigateTreeItem(QTreeWidgetItem* item);
    void showSection(int index);
    void refreshSectionList();
    void refreshSectionRecord(int row);
    void applySectionPayload();
    void applySectionNames();
    void applyFieldEdit(QTableWidgetItem* item);
    void exportPlainText();
    void applyChannelEdit();
    void refreshPorts();
    void refreshAfterEdit();
    void refreshZones();
    void selectChannelRow(int zone, int channel);
    const opendepot::RecordNode* findNode(uint32_t id) const;
    QWidget* buildRadioInfoPage();
    QWidget* buildZonesPage();
    QWidget* buildRadioWidePage();
    QWidget* buildFieldCatalogPage();
    void fillFieldCatalog(int tableIndex);
    QWidget* buildCpsTablesPage();
    void fillCpsTable(int tableIndex);
    QWidget* buildSectionPage();
    void refreshRadioWidePage();
    void setMaxChannels(int value);
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
    std::unique_ptr<opendepot::CodeplugModel> model_;
    bool have_ = false;
    bool dirty_ = false;
    QString path_;
    QDateTime openedAt_;

    QTreeWidget* tree_ = nullptr;
    QTreeWidgetItem* treeRoot_ = nullptr;
    QTreeWidgetItem* zonesItem_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QScrollArea* infoScroll_ = nullptr;
    QLabel* subtitle_ = nullptr;
    QLabel* serial_ = nullptr;
    QLabel* modelLabel_ = nullptr;
    QLabel* flashcode_ = nullptr;
    QLabel* headerVersion_ = nullptr;
    QLabel* recordCount_ = nullptr;
    QLabel* passcode_ = nullptr;
    QLabel* codeplugFile_ = nullptr;
    QLabel* modelStatus_ = nullptr;
    QWidget* chips_ = nullptr;
    QGridLayout* chipsLayout_ = nullptr;
    QTableWidget* zones_ = nullptr;
    QTableWidget* catalogTables_ = nullptr;
    QTableWidget* catalogColumns_ = nullptr;
    QTableWidget* cpsTableList_ = nullptr;
    QTableWidget* cpsGrid_ = nullptr;
    QLabel* cpsTableInfo_ = nullptr;
    QSpinBox* rwMaxChannels_ = nullptr;
    QHash<QString, QWidget*> rwBound_;
    QTableWidget* rwNamedFields_ = nullptr;
    QLineEdit* zoneNameEdit_ = nullptr;
    QLineEdit* channelNameEdit_ = nullptr;
    QLineEdit* rxEdit_ = nullptr;
    QLineEdit* txEdit_ = nullptr;
    QLineEdit* rxSqEdit_ = nullptr;
    QLineEdit* txSqEdit_ = nullptr;
    int editZone_ = -1;
    int editChannel_ = -1;

    int sectionIndex_ = -1;
    QLabel* sectionTitle_ = nullptr;
    QLabel* sectionSubtitle_ = nullptr;
    QGroupBox* trunkGroup_ = nullptr;
    QLineEdit* trunkWacn_ = nullptr;
    QLineEdit* trunkHomeId_ = nullptr;
    QTableWidget* sectionRecords_ = nullptr;
    QPlainTextEdit* sectionDetail_ = nullptr;
    QPlainTextEdit* sectionHex_ = nullptr;
    QTableWidget* sectionFields_ = nullptr;
    QTableWidget* sectionNamed_ = nullptr;
    QTableWidget* sectionNames_ = nullptr;
    QLineEdit* sectionNameEdit_ = nullptr;
    QPushButton* sectionApplyNames_ = nullptr;

    QPlainTextEdit* report_ = nullptr;
    QPlainTextEdit* doctor_ = nullptr;
    QPlainTextEdit* diff_ = nullptr;
    QComboBox* port_ = nullptr;
    QPlainTextEdit* radioLog_ = nullptr;
    QProgressDialog* busy_ = nullptr;
    QTimer* poll_ = nullptr;
    QAction* saveAct_ = nullptr;
    std::unique_ptr<RadioJob> job_;
};
