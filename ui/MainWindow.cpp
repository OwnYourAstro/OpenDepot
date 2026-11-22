// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include "MainWindow.h"

#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QDateTime>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <filesystem>
#include <vector>

#include "opendepot/core/radio.h"

namespace {
constexpr int kInfoPage = 0;
constexpr int kReportPage = 1;
constexpr int kDoctorPage = 2;
constexpr int kDiffPage = 3;
constexpr int kRadioPage = 4;
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("OpenDepot");
    resize(1320, 860);
    setupMenu();
    setupUi();
    setModelStatus("Model: -");
    statusBar()->showMessage("Ready");

    poll_ = new QTimer(this);
    poll_->setInterval(200);
    connect(poll_, &QTimer::timeout, this, &MainWindow::pollRadio);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenu() {
    auto* file = menuBar()->addMenu("&File");
    auto* open = file->addAction("&Open Codeplug...");
    open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this, &MainWindow::openCodeplug);

    saveAct_ = file->addAction("Save &As...");
    saveAct_->setShortcut(QKeySequence::SaveAs);
    saveAct_->setEnabled(false);
    connect(saveAct_, &QAction::triggered, this, &MainWindow::saveCodeplugAs);

    file->addSeparator();
    auto* quit = file->addAction("E&xit");
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, this, &QWidget::close);

    auto* view = menuBar()->addMenu("&View");
    connect(view->addAction("&Expand All"), &QAction::triggered, this,
            [this] { tree_->expandAll(); });
    connect(view->addAction("&Collapse All"), &QAction::triggered, this,
            [this] { tree_->collapseAll(); });

    auto* tools = menuBar()->addMenu("&Tools");
    connect(tools->addAction("&Records"), &QAction::triggered, this, &MainWindow::showRecords);
    connect(tools->addAction("Service &Report"), &QAction::triggered, this,
            &MainWindow::generateReport);
    connect(tools->addAction("Codeplug &Doctor"), &QAction::triggered, this,
            &MainWindow::runDoctor);
    connect(tools->addAction("Codeplug &Diff..."), &QAction::triggered, this,
            &MainWindow::compareCodeplug);

    auto* radio = menuBar()->addMenu("&Radio");
    connect(radio->addAction("&Read Radio"), &QAction::triggered, this, &MainWindow::readRadio);
    connect(radio->addAction("&Write Radio..."), &QAction::triggered, this,
            &MainWindow::writeRadio);

    auto* help = menuBar()->addMenu("&Help");
    connect(help->addAction("&About"), &QAction::triggered, this, &MainWindow::about);

    auto* bar = addToolBar("Main");
    bar->setMovable(false);
    auto add = [&](QStyle::StandardPixmap icon, const QString& text, void (MainWindow::*slot)()) {
        auto* a = bar->addAction(style()->standardIcon(icon), text);
        connect(a, &QAction::triggered, this, slot);
    };
    add(QStyle::SP_DialogOpenButton, "Open", &MainWindow::openCodeplug);
    auto* saveBtn = bar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save");
    connect(saveBtn, &QAction::triggered, this, &MainWindow::saveCodeplugAs);
    bar->addSeparator();
    add(QStyle::SP_ArrowDown, "Read Radio", &MainWindow::readRadio);
    add(QStyle::SP_ArrowUp, "Write Radio", &MainWindow::writeRadio);
}

void MainWindow::setupUi() {
    auto* central = new QWidget;
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* treePanel = new QWidget;
    treePanel->setObjectName("treePanel");
    treePanel->setFixedWidth(292);
    auto* tl = new QVBoxLayout(treePanel);
    tl->setContentsMargins(10, 12, 10, 10);
    tl->setSpacing(8);
    auto* heading = new QLabel("Tree View");
    heading->setObjectName("treeHeading");
    tl->addWidget(heading);

    buildTree();
    tl->addWidget(tree_, 1);

    auto* buttons = new QHBoxLayout;
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setObjectName("small");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    auto* helpBtn = new QPushButton("Help");
    helpBtn->setObjectName("small");
    connect(helpBtn, &QPushButton::clicked, this, &MainWindow::about);
    auto* expandBtn = new QPushButton("Expand All");
    expandBtn->setObjectName("small");
    connect(expandBtn, &QPushButton::clicked, tree_, &QTreeWidget::expandAll);
    buttons->addWidget(closeBtn);
    buttons->addWidget(helpBtn);
    buttons->addWidget(expandBtn);
    buttons->addStretch(1);
    tl->addLayout(buttons);

    root->addWidget(treePanel);

    pages_ = new QStackedWidget;
    pages_->addWidget(buildRadioInfoPage());
    pages_->addWidget(buildTextPage("Service Report", &report_, "Generate report",
                                    &MainWindow::generateReport));
    pages_->addWidget(buildTextPage("Codeplug Doctor", &doctor_, "Run doctor",
                                    &MainWindow::runDoctor));
    pages_->addWidget(buildTextPage("Codeplug Diff", &diff_, "Compare with codeplug...",
                                    &MainWindow::compareCodeplug));
    pages_->addWidget(buildRadioPage());
    pages_->setCurrentIndex(kInfoPage);
    root->addWidget(pages_, 1);

    setCentralWidget(central);
}

void MainWindow::buildTree() {
    tree_ = new QTreeWidget;
    tree_->setObjectName("tree");
    tree_->setHeaderHidden(true);
    tree_->setColumnCount(1);
    tree_->setIndentation(16);
    tree_->setIconSize(QSize(16, 16));

    auto icon = [this](QStyle::StandardPixmap p) { return style()->standardIcon(p); };

    treeRoot_ = new QTreeWidgetItem(tree_, QStringList{"No codeplug"});
    treeRoot_->setIcon(0, icon(QStyle::SP_ComputerIcon));
    treeRoot_->setData(0, Qt::UserRole, kInfoPage);

    auto* info = new QTreeWidgetItem(treeRoot_, QStringList{"Radio Information"});
    info->setIcon(0, icon(QStyle::SP_MessageBoxInformation));
    info->setData(0, Qt::UserRole, kInfoPage);

    auto* config = new QTreeWidgetItem(treeRoot_, QStringList{"Radio Configuration"});
    config->setIcon(0, icon(QStyle::SP_DirIcon));
    config->setData(0, Qt::UserRole, kInfoPage);
    for (const char* n : {"Radio Wide", "Controls", "Keypad", "Display & Menu"}) {
        auto* it = new QTreeWidgetItem(config, QStringList{QString::fromLatin1(n)});
        it->setIcon(0, icon(QStyle::SP_FileIcon));
        it->setData(0, Qt::UserRole, kInfoPage);
    }

    auto* conv = new QTreeWidgetItem(treeRoot_, QStringList{"Conventional"});
    conv->setIcon(0, icon(QStyle::SP_DirIcon));
    conv->setData(0, Qt::UserRole, kInfoPage);
    auto* zonesIt = new QTreeWidgetItem(conv, QStringList{"Zones & Channels"});
    zonesIt->setIcon(0, icon(QStyle::SP_FileIcon));
    zonesIt->setData(0, Qt::UserRole, kInfoPage);
    zonesIt->setData(0, Qt::UserRole + 1, "zones");

    for (const char* n : {"ASTRO Systems", "Trunking", "Scan"}) {
        auto* it = new QTreeWidgetItem(treeRoot_, QStringList{QString::fromLatin1(n)});
        it->setIcon(0, icon(QStyle::SP_DirIcon));
        it->setData(0, Qt::UserRole, kInfoPage);
    }

    auto* tools = new QTreeWidgetItem(treeRoot_, QStringList{"Tools"});
    tools->setIcon(0, icon(QStyle::SP_DirIcon));
    tools->setData(0, Qt::UserRole, kInfoPage);
    struct ToolItem {
        const char* name;
        int page;
    };
    const ToolItem toolItems[] = {{"Service Report", kReportPage},
                                  {"Codeplug Doctor", kDoctorPage},
                                  {"Diff", kDiffPage},
                                  {"Radio", kRadioPage}};
    for (const auto& t : toolItems) {
        auto* it = new QTreeWidgetItem(tools, QStringList{QString::fromLatin1(t.name)});
        it->setIcon(0, icon(QStyle::SP_FileIcon));
        it->setData(0, Qt::UserRole, t.page);
    }

    tree_->expandAll();

    connect(tree_, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* cur, QTreeWidgetItem*) {
                if (!cur) return;
                if (cur->data(0, Qt::UserRole + 1).toString() == "zones") {
                    showZones();
                    return;
                }
                pages_->setCurrentIndex(cur->data(0, Qt::UserRole).toInt());
            });
}

QWidget* MainWindow::makeCard(const QString& caption, QLabel** value, const QString& initial) {
    auto* frame = new QFrame;
    frame->setObjectName("card");
    auto* lay = new QVBoxLayout(frame);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(6);
    auto* cap = new QLabel(caption.toUpper());
    cap->setObjectName("cardCaption");
    *value = new QLabel(initial);
    (*value)->setObjectName("cardValue");
    (*value)->setWordWrap(true);
    (*value)->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lay->addWidget(cap);
    lay->addWidget(*value);
    lay->addStretch(1);
    return frame;
}

QWidget* MainWindow::buildRadioInfoPage() {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    infoScroll_ = scroll;

    auto* content = new QWidget;
    content->setObjectName("contentRoot");
    auto* lay = new QVBoxLayout(content);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(18);

    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    titles->setSpacing(2);
    auto* title = new QLabel("Radio Information");
    title->setObjectName("pageTitle");
    subtitle_ = new QLabel("No codeplug loaded");
    subtitle_->setObjectName("muted");
    titles->addWidget(title);
    titles->addWidget(subtitle_);
    header->addLayout(titles);
    header->addStretch(1);
    auto* readBtn = new QPushButton("Read Radio");
    readBtn->setObjectName("accent");
    connect(readBtn, &QPushButton::clicked, this, &MainWindow::readRadio);
    auto* recBtn = new QPushButton("Records");
    connect(recBtn, &QPushButton::clicked, this, &MainWindow::showRecords);
    auto* zonesBtn = new QPushButton("Zones");
    connect(zonesBtn, &QPushButton::clicked, this, &MainWindow::showZones);
    header->addWidget(readBtn);
    header->addWidget(recBtn);
    header->addWidget(zonesBtn);
    lay->addLayout(header);

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);
    grid->addWidget(makeCard("Serial", &serial_), 0, 0);
    grid->addWidget(makeCard("Model", &model_), 0, 1);
    grid->addWidget(makeCard("FLASHcode", &flashcode_), 0, 2);
    grid->addWidget(makeCard("Header version", &headerVersion_), 1, 0);
    grid->addWidget(makeCard("Records", &recordCount_), 1, 1);
    grid->addWidget(makeCard("Passcode", &passcode_), 1, 2);
    grid->addWidget(makeCard("Codeplug file", &codeplugFile_, "No codeplug loaded"), 2, 0, 1, 2);

    auto* explore = new QFrame;
    explore->setObjectName("card");
    auto* el = new QVBoxLayout(explore);
    el->setContentsMargins(16, 14, 16, 14);
    el->setSpacing(6);
    auto* exCap = new QLabel("EXPLORE");
    exCap->setObjectName("cardCaption");
    auto* browse = new QPushButton("Browse this codeplug's records");
    browse->setObjectName("link");
    connect(browse, &QPushButton::clicked, this, &MainWindow::showRecords);
    auto* zoneLink = new QPushButton("Zone and channel details");
    zoneLink->setObjectName("link");
    connect(zoneLink, &QPushButton::clicked, this, &MainWindow::showZones);
    el->addWidget(exCap);
    el->addWidget(browse);
    el->addWidget(zoneLink);
    el->addStretch(1);
    grid->addWidget(explore, 2, 2);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 1);
    lay->addLayout(grid);

    auto* feat = new QLabel("FLASHcode Features");
    feat->setObjectName("sectionTitle");
    lay->addWidget(feat);
    chips_ = new QWidget;
    chipsLayout_ = new QGridLayout(chips_);
    chipsLayout_->setContentsMargins(0, 0, 0, 0);
    chipsLayout_->setHorizontalSpacing(8);
    chipsLayout_->setVerticalSpacing(8);
    lay->addWidget(chips_);

    auto* zoneTitle = new QLabel("Zones and Channels");
    zoneTitle->setObjectName("sectionTitle");
    lay->addWidget(zoneTitle);
    zones_ = new QTableWidget(0, 5);
    zones_->setHorizontalHeaderLabels({"#", "Channel", "Mode", "RX MHz", "TX MHz"});
    zones_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zones_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    zones_->verticalHeader()->setVisible(false);
    zones_->verticalHeader()->setDefaultSectionSize(34);
    zones_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    zones_->setSelectionBehavior(QAbstractItemView::SelectRows);
    zones_->setShowGrid(false);
    lay->addWidget(zones_);
    lay->addStretch(1);

    scroll->setWidget(content);
    return scroll;
}

QWidget* MainWindow::buildTextPage(const QString& titleText, QPlainTextEdit** out,
                                   const QString& button, void (MainWindow::*handler)()) {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(12);
    auto* title = new QLabel(titleText);
    title->setObjectName("pageTitle");
    lay->addWidget(title);
    auto* btn = new QPushButton(button);
    connect(btn, &QPushButton::clicked, this, handler);
    lay->addWidget(btn, 0, Qt::AlignLeft);
    *out = new QPlainTextEdit;
    (*out)->setReadOnly(true);
    (*out)->setPlaceholderText("Nothing to show yet.");
    lay->addWidget(*out, 1);
    return page;
}

QWidget* MainWindow::buildRadioPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(14);
    auto* title = new QLabel("Radio");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    auto* card = new QFrame;
    card->setObjectName("card");
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(18, 16, 18, 16);
    cl->setSpacing(10);
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Port"));
    port_ = new QLineEdit("COM20");
    port_->setMaximumWidth(150);
    row->addWidget(port_);
    auto* read = new QPushButton("Read Radio");
    read->setObjectName("accent");
    auto* write = new QPushButton("Write Radio...");
    row->addWidget(read);
    row->addWidget(write);
    row->addStretch(1);
    cl->addLayout(row);
    auto* note = new QLabel(
        "Reads and writes use OpenDepot's own serial/PPP/RCMP transport. "
        "A write can brick a radio; keep a verified backup.");
    note->setObjectName("muted");
    note->setWordWrap(true);
    cl->addWidget(note);
    lay->addWidget(card);

    radioLog_ = new QPlainTextEdit;
    radioLog_->setReadOnly(true);
    lay->addWidget(radioLog_, 1);

    connect(read, &QPushButton::clicked, this, &MainWindow::readRadio);
    connect(write, &QPushButton::clicked, this, &MainWindow::writeRadio);
    return page;
}

void MainWindow::setModelStatus(const QString& text) {
    if (!modelStatus_) {
        modelStatus_ = new QLabel(text);
        modelStatus_->setObjectName("muted");
        statusBar()->addPermanentWidget(modelStatus_);
    } else {
        modelStatus_->setText(text);
    }
}

void MainWindow::openCodeplug() {
    const QString start = path_.isEmpty() ? QString() : path_;
    const QString file = QFileDialog::getOpenFileName(this, "Open codeplug", start,
                                                      "Codeplug (*.cpg);;All files (*)");
    if (file.isEmpty()) return;
    openFile(file);
}

void MainWindow::openFile(const QString& file) {
    opendepot::Codeplug cp;
    if (!opendepot::LoadCodeplug(std::filesystem::path(file.toStdWString()), cp)) {
        QMessageBox::critical(this, "Open codeplug",
                              QString::fromStdString("Cannot load: " + cp.error));
        return;
    }
    setCodeplug(std::move(cp), file);
}

void MainWindow::saveCodeplugAs() {
    if (!have_) return;
    const QString file = QFileDialog::getSaveFileName(this, "Save codeplug", path_,
                                                      "Codeplug (*.cpg)");
    if (file.isEmpty()) return;
    std::string err;
    if (!opendepot::SaveCodeplug(std::filesystem::path(file.toStdWString()), cp_, err)) {
        QMessageBox::critical(this, "Save codeplug",
                              QString::fromStdString("Cannot save: " + err));
        return;
    }
    path_ = file;
    updateDashboard();
    statusBar()->showMessage("Saved " + file, 8000);
}

void MainWindow::setCodeplug(opendepot::Codeplug cp, const QString& path) {
    cp_ = std::move(cp);
    have_ = true;
    path_ = path;
    openedAt_ = QDateTime::currentDateTime();
    saveAct_->setEnabled(true);
    if (treeRoot_)
        treeRoot_->setText(0, QString::fromStdString(cp_.header.serial) + "  " +
                                 QString::fromStdString(cp_.header.model));
    updateDashboard();
    pages_->setCurrentIndex(kInfoPage);
    tree_->setCurrentItem(treeRoot_);
    statusBar()->showMessage("Loaded " + path, 8000);
}

void MainWindow::updateDashboard() {
    if (!have_) {
        serial_->setText("-");
        model_->setText("-");
        flashcode_->setText("-");
        headerVersion_->setText("-");
        recordCount_->setText("-");
        passcode_->setText("-");
        codeplugFile_->setText("No codeplug loaded");
        subtitle_->setText("No codeplug loaded");
        setModelStatus("Model: -");
        return;
    }
    serial_->setText(QString::fromStdString(cp_.header.serial));
    model_->setText(QString::fromStdString(cp_.header.model));
    flashcode_->setText(QString::fromStdString(opendepot::GetFlashcode(cp_)));
    headerVersion_->setText(QString::fromStdString(cp_.header.version));
    size_t good = 0;
    for (const auto& r : cp_.records)
        if (r.verified) ++good;
    recordCount_->setText(QString("%1 / %2 CRC-verified")
                              .arg(static_cast<qulonglong>(cp_.records.size()))
                              .arg(static_cast<qulonglong>(good)));

    opendepot::CodeplugPassword pw;
    if (opendepot::GetCodeplugPassword(cp_, pw) && pw.present)
        passcode_->setText(pw.enabled ? "Set (enabled)" : "Set (disabled)");
    else
        passcode_->setText("Not set");

    const QFileInfo info(path_);
    codeplugFile_->setText(info.fileName().isEmpty() ? "No codeplug loaded" : info.fileName());
    codeplugFile_->setToolTip(path_);
    subtitle_->setText(QString("Codeplug opened from %1 on %2")
                           .arg(info.fileName(), openedAt_.toString("yyyy-MM-dd HH:mm:ss")));
    setModelStatus("Model: " + QString::fromStdString(cp_.header.model));

    while (auto* item = chipsLayout_->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    const auto features = opendepot::EnabledFeatures(opendepot::GetFlashcode(cp_));
    int row = 0, col = 0;
    for (const auto& f : features) {
        auto* chip = new QLabel(QString::fromStdString(f));
        chip->setObjectName("chip");
        chipsLayout_->addWidget(chip, row, col);
        if (++col == 4) {
            col = 0;
            ++row;
        }
    }
    if (features.empty()) {
        auto* none = new QLabel("No FLASHcode features decoded");
        none->setObjectName("muted");
        chipsLayout_->addWidget(none, 0, 0);
    }

    fillZones(zones_, true);
}

void MainWindow::fillZones(QTableWidget* table, bool groupRows) {
    table->setRowCount(0);
    if (!have_) return;
    std::vector<opendepot::ZoneView> zones;
    opendepot::ParseZones(cp_, zones);

    int index = 0;
    for (const auto& z : zones) {
        if (groupRows) {
            const int row = table->rowCount();
            table->insertRow(row);
            auto* item = new QTableWidgetItem(
                QString("%1    %2 channel(s)")
                    .arg(QString::fromStdString(z.name))
                    .arg(static_cast<int>(z.channels.size())));
            QFont f = table->font();
            f.setBold(true);
            item->setFont(f);
            item->setBackground(QColor("#f8fafc"));
            item->setForeground(QColor("#475569"));
            table->setItem(row, 0, item);
            table->setSpan(row, 0, 1, 5);
            table->setRowHeight(row, 34);
        }
        if (z.channels.empty()) {
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(""));
            table->setItem(row, 1, new QTableWidgetItem("-"));
            table->setItem(row, 2, new QTableWidgetItem(""));
            table->setItem(row, 3, new QTableWidgetItem(""));
            table->setItem(row, 4, new QTableWidgetItem(""));
            continue;
        }
        for (const auto& ch : z.channels) {
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(index++)));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(ch.name)));
            table->setItem(row, 2,
                           new QTableWidgetItem(ch.rxWord == ch.txWord ? "Simple" : "Split"));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(
                                              opendepot::WordToMHz(ch.rxWord), 'f', 5)));
            table->setItem(row, 4, new QTableWidgetItem(QString::number(
                                              opendepot::WordToMHz(ch.txWord), 'f', 5)));
        }
    }
    if (table == zones_) {
        const int h = table->horizontalHeader()->height() + table->rowCount() * 34 + 8;
        table->setMinimumHeight(std::max(160, h));
        table->setMaximumHeight(std::max(160, h));
    }
}

void MainWindow::showZones() {
    pages_->setCurrentIndex(kInfoPage);
    tree_->setCurrentItem(treeRoot_);
    if (infoScroll_ && zones_) infoScroll_->ensureWidgetVisible(zones_);
}

void MainWindow::showRecords() {
    if (!have_) {
        QMessageBox::information(this, "Records", "Open a codeplug first.");
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle("Records");
    dlg.resize(780, 540);
    auto* lay = new QVBoxLayout(&dlg);
    auto* table = new QTableWidget(static_cast<int>(cp_.records.size()), 4);
    table->setHorizontalHeaderLabels({"Record ID", "Partition", "Size", "CRC"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    for (size_t i = 0; i < cp_.records.size(); ++i) {
        const auto& r = cp_.records[i];
        const int row = static_cast<int>(i);
        table->setItem(row, 0,
                       new QTableWidgetItem(QString("0x%1").arg(
                           QString::number(r.id, 16).rightJustified(8, '0'))));
        table->setItem(row, 1,
                       new QTableWidgetItem(QString("0x%1").arg(
                           QString::number(r.type, 16).rightJustified(4, '0'))));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(r.raw.size())));
        table->setItem(row, 3, new QTableWidgetItem(r.verified ? "OK" : "BAD"));
    }
    lay->addWidget(table);
    auto* close = new QPushButton("Close");
    connect(close, &QPushButton::clicked, &dlg, &QDialog::accept);
    lay->addWidget(close, 0, Qt::AlignRight);
    dlg.exec();
}

void MainWindow::generateReport() {
    if (!have_) {
        QMessageBox::information(this, "Service report", "Open a codeplug first.");
        return;
    }
    report_->setPlainText(
        QString::fromStdString(opendepot::ServiceReportText(cp_, std::string())));
    pages_->setCurrentIndex(kReportPage);
}

void MainWindow::runDoctor() {
    if (!have_) {
        QMessageBox::information(this, "Codeplug doctor", "Open a codeplug first.");
        return;
    }
    opendepot::Codeplug copy = cp_;
    opendepot::DoctorResult result;
    opendepot::DoctorCodeplug(copy, result, false);
    QString text;
    if (result.issues.empty()) {
        text = "Codeplug doctor: healthy\n";
    } else {
        text = "Issues:\n";
        for (const auto& s : result.issues)
            text += "- " + QString::fromStdString(s) + "\n";
    }
    if (!result.fixes.empty()) {
        text += "\nSuggested fixes:\n";
        for (const auto& s : result.fixes)
            text += "- " + QString::fromStdString(s) + "\n";
    }
    doctor_->setPlainText(text);
    pages_->setCurrentIndex(kDoctorPage);
}

void MainWindow::compareCodeplug() {
    if (!have_) {
        QMessageBox::information(this, "Diff", "Open a codeplug first.");
        return;
    }
    const QString file = QFileDialog::getOpenFileName(this, "Compare with codeplug", path_,
                                                      "Codeplug (*.cpg);;All files (*)");
    if (file.isEmpty()) return;
    opendepot::Codeplug other;
    if (!opendepot::LoadCodeplug(std::filesystem::path(file.toStdWString()), other)) {
        QMessageBox::critical(this, "Diff",
                              QString::fromStdString("Cannot load: " + other.error));
        return;
    }
    diff_->setPlainText(QString::fromStdString(opendepot::DiffCodeplugsText(cp_, other)));
    pages_->setCurrentIndex(kDiffPage);
}

void MainWindow::readRadio() {
    if (job_) return;
    auto job = std::make_unique<RadioJob>();
    job->isWrite = false;
    RadioJob* j = job.get();
    opendepot::Codeplug base = cp_;
    const std::string port = port_ ? port_->text().trimmed().toStdString() : "COM20";
    appendRadioLog("Reading " + QString::fromStdString(port) + "...");
    showBusy("Reading radio...");
    j->thread = std::thread([j, base, port]() {
        std::string err;
        auto session = opendepot::RadioSession::Open(port, err);
        if (!session) {
            j->error = QString::fromStdString(err);
            j->done = true;
            return;
        }
        opendepot::RadioInfo info;
        session->Info(info, err);
        opendepot::Codeplug tpl = base;
        if (tpl.records.empty()) {
            const std::string model =
                info.model.empty() ? opendepot::BuiltinFirstModel() : info.model;
            std::string berr;
            if (!opendepot::BuiltinTemplate(model, tpl, berr) || tpl.records.empty()) {
                j->error = QString::fromStdString("No template for the connected model");
                j->done = true;
                return;
            }
        }
        std::string model = info.model;
        if (model.empty()) model = tpl.header.model;
        std::vector<opendepot::ReadRequest> modelReqs;
        opendepot::ModelReadRequests(model, modelReqs);
        std::vector<opendepot::RadioReadRequest> reqs;
        if (!modelReqs.empty()) {
            for (const auto& r : modelReqs) reqs.push_back({r.part, r.tag, r.inst, r.len});
        } else {
            for (const auto& r : tpl.records)
                reqs.push_back({r.type, static_cast<uint16_t>(r.id >> 16),
                                static_cast<uint16_t>(r.id & 0xFFFF),
                                static_cast<uint16_t>(r.raw.size() - 6)});
        }
        std::vector<opendepot::Record> recs;
        if (!session->ReadRequests(reqs, recs, err)) {
            session->Close();
            j->error = QString::fromStdString(err);
            j->done = true;
            return;
        }
        session->Close();
        opendepot::Codeplug out = tpl;
        out.records = std::move(recs);
        std::string serial, modelName;
        if (opendepot::ReadIdentityFromCodeplug(out, serial, modelName))
            opendepot::UpdateIdentity(out, serial, modelName);
        std::string verr;
        if (!opendepot::ValidateRadioCodeplug(out, verr)) {
            j->error = QString::fromStdString("Incomplete read: " + verr);
            j->done = true;
            return;
        }
        j->cp = std::move(out);
        j->ok = true;
        j->done = true;
    });
    job_ = std::move(job);
    poll_->start();
}

void MainWindow::writeRadio() {
    if (job_) return;
    if (!have_) {
        QMessageBox::information(this, "Write radio", "Open or read a codeplug first.");
        return;
    }
    std::string verr;
    if (!opendepot::ValidateRadioCodeplug(cp_, verr)) {
        QMessageBox::warning(this, "Write refused",
                             QString::fromStdString(verr) +
                                 "\n\nWriting this codeplug could brick the radio.");
        return;
    }
    const std::string port = port_ ? port_->text().trimmed().toStdString() : "COM20";
    const auto answer = QMessageBox::warning(
        this, "Write to radio",
        QString("Write %1 records to %2?\n\nThis can brick the radio. Keep a verified backup.")
            .arg(static_cast<qulonglong>(cp_.records.size()))
            .arg(QString::fromStdString(port)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    auto job = std::make_unique<RadioJob>();
    job->isWrite = true;
    RadioJob* j = job.get();
    opendepot::Codeplug base = cp_;
    appendRadioLog("Writing " + QString::fromStdString(port) + "...");
    showBusy("Writing radio...");
    j->thread = std::thread([j, base, port]() {
        std::string err;
        auto session = opendepot::RadioSession::Open(port, err);
        if (!session) {
            j->error = QString::fromStdString(err);
            j->done = true;
            return;
        }
        const bool ok = session->WriteRecords(base.records, err);
        session->Close();
        j->ok = ok;
        if (!ok) j->error = QString::fromStdString(err);
        j->done = true;
    });
    job_ = std::move(job);
    poll_->start();
}

void MainWindow::pollRadio() {
    if (!job_ || !job_->done.load()) return;
    poll_->stop();
    finishBusy();
    if (job_->ok && !job_->isWrite) {
        appendRadioLog("Read complete.");
        setCodeplug(std::move(job_->cp), "(radio)");
    } else if (job_->ok && job_->isWrite) {
        appendRadioLog("Write complete. Read back to verify.");
    } else {
        appendRadioLog("Radio error: " + job_->error);
        QMessageBox::critical(this, "Radio", job_->error);
    }
    job_.reset();
}

void MainWindow::showBusy(const QString& text) {
    busy_ = new QProgressDialog(text, QString(), 0, 0, this);
    busy_->setWindowTitle("OpenDepot");
    busy_->setWindowModality(Qt::WindowModal);
    busy_->setCancelButton(nullptr);
    busy_->setMinimumDuration(0);
    busy_->show();
}

void MainWindow::finishBusy() {
    if (busy_) {
        busy_->close();
        busy_->deleteLater();
        busy_ = nullptr;
    }
}

void MainWindow::appendRadioLog(const QString& line) {
    if (radioLog_) radioLog_->appendPlainText(line);
}

void MainWindow::about() {
    QMessageBox::about(this, "About OpenDepot",
                       "OpenDepot\n\nClean-room ASTRO 25 codeplug editor and radio "
                       "transport.\n\nWriting to a radio can brick it. Use at your own "
                       "risk.");
}
