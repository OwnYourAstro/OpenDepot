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
#include <QHash>
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
#include <QSignalBlocker>
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

#include "opendepot/core/model.h"
#include "opendepot/core/radio.h"

namespace {
constexpr int kInfoPage = 0;
constexpr int kZonesPage = 1;
constexpr int kReportPage = 2;
constexpr int kDoctorPage = 3;
constexpr int kDiffPage = 4;
constexpr int kRadioPage = 5;
constexpr int kSectionPage = 6;
constexpr int kRolePage = Qt::UserRole;
constexpr int kRoleSection = Qt::UserRole + 1;
constexpr int kRoleZone = Qt::UserRole + 2;
constexpr int kRoleChannel = Qt::UserRole + 3;
constexpr int kRoleRecordId = Qt::UserRole + 4;

QString HexBytes(const std::vector<uint8_t>& p) {
    QString s;
    s.reserve(int(p.size()) * 3);
    for (size_t i = 0; i < p.size(); ++i) {
        if (i) s += ' ';
        s += QString("%1").arg(p[i], 2, 16, QChar('0')).toUpper();
    }
    return s;
}

bool ParseHex(const QString& text, std::vector<uint8_t>& out) {
    out.clear();
    QString clean;
    for (QChar c : text)
        if (!c.isSpace()) clean += c;
    if (clean.size() % 2) return false;
    for (int i = 0; i < clean.size(); i += 2) {
        bool ok = false;
        const int v = clean.mid(i, 2).toInt(&ok, 16);
        if (!ok || v < 0 || v > 255) return false;
        out.push_back(uint8_t(v));
    }
    return true;
}

uint16_t Be16(const std::vector<uint8_t>& p, size_t off) {
    if (off + 1 >= p.size()) return 0;
    return uint16_t((p[off] << 8) | p[off + 1]);
}

void PutName(std::vector<uint8_t>& p, size_t off, size_t width, const QString& name) {
    if (width == 0 || off + width > p.size()) return;
    std::fill(p.begin() + static_cast<std::ptrdiff_t>(off),
              p.begin() + static_cast<std::ptrdiff_t>(off + width), 0);
    const QByteArray bytes = name.toUtf8();
    const size_t n = std::min<size_t>(size_t(bytes.size()), width - 1);
    std::copy(bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(n),
              p.begin() + static_cast<std::ptrdiff_t>(off));
}
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

    auto* save = file->addAction("&Save");
    save->setShortcut(QKeySequence::Save);
    connect(save, &QAction::triggered, this, &MainWindow::saveCodeplug);

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
    connect(saveBtn, &QAction::triggered, this, &MainWindow::saveCodeplug);
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
    pages_->addWidget(buildZonesPage());
    pages_->addWidget(buildTextPage("Service Report", &report_, "Generate report",
                                    &MainWindow::generateReport));
    pages_->addWidget(buildTextPage("Codeplug Doctor", &doctor_, "Run doctor",
                                    &MainWindow::runDoctor));
    pages_->addWidget(buildTextPage("Codeplug Diff", &diff_, "Compare with codeplug...",
                                    &MainWindow::compareCodeplug));
    pages_->addWidget(buildRadioPage());
    pages_->addWidget(buildSectionPage());
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

    treeRoot_ = new QTreeWidgetItem(tree_, QStringList{"No codeplug"});
    treeRoot_->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
    treeRoot_->setData(0, kRolePage, kInfoPage);

    auto* tools = new QTreeWidgetItem(treeRoot_, QStringList{"Tools"});
    tools->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    tools->setData(0, kRolePage, -1);
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
        it->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        it->setData(0, kRolePage, t.page);
    }

    connect(tree_, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* cur, QTreeWidgetItem*) { navigateTreeItem(cur); });
    connect(tree_, &QTreeWidget::itemSelectionChanged, this, [this] {
        const auto selected = tree_->selectedItems();
        if (!selected.isEmpty()) navigateTreeItem(selected.first());
    });
}

void MainWindow::navigateTreeItem(QTreeWidgetItem* item) {
    if (!item) return;
    const int page = item->data(0, kRolePage).toInt();
    if (page < 0) return;
    if (page == kSectionPage) showSection(item->data(0, kRoleSection).toInt());
    pages_->setCurrentIndex(page);
}

void MainWindow::buildModelTree() {
    if (!treeRoot_) return;
    while (treeRoot_->childCount()) delete treeRoot_->takeChild(0);
    zonesItem_ = nullptr;
    if (!model_) {
        treeRoot_->setText(0, "No codeplug");
        return;
    }
    treeRoot_->setText(0, QString::fromStdString(cp_.header.serial) + "  " +
                             QString::fromStdString(cp_.header.model));

    QHash<QString, QTreeWidgetItem*> groups;
    const auto& sections = model_->Sections();
    for (size_t i = 0; i < sections.size(); ++i) {
        const auto& s = sections[i];
        const QString group = QString::fromStdString(s.group);
        QTreeWidgetItem* gi = groups.value(group);
        if (!gi) {
            gi = new QTreeWidgetItem(treeRoot_, QStringList{group});
            gi->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            gi->setData(0, kRolePage, -1);
            groups.insert(group, gi);
        }
        QString label = QString::fromStdString(s.name);
        if (s.records.size() != 1)
            label += QString("  (%1)").arg(static_cast<int>(s.records.size()));
        auto* it = new QTreeWidgetItem(gi, QStringList{label});
        it->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        int page = kSectionPage;
        if (s.id == "radio_information") {
            page = kInfoPage;
        } else if (s.id == "zone_channel_assignment") {
            page = kZonesPage;
            zonesItem_ = it;
        }
        it->setData(0, kRolePage, page);
        it->setData(0, kRoleSection, static_cast<int>(i));
        it->setToolTip(0, QString("Confidence: %1")
                              .arg(QString::fromUtf8(opendepot::ConfidenceName(s.confidence))));
    }

    auto* tools = new QTreeWidgetItem(treeRoot_, QStringList{"Tools"});
    tools->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    tools->setData(0, kRolePage, -1);
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
        it->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        it->setData(0, kRolePage, t.page);
    }
    tree_->expandAll();
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
    grid->addWidget(makeCard("Model", &modelLabel_), 0, 1);
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
    lay->addStretch(1);

    scroll->setWidget(content);
    return scroll;
}

QWidget* MainWindow::buildZonesPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(12);
    auto* title = new QLabel("Zone Channel Assignment");
    title->setObjectName("pageTitle");
    lay->addWidget(title);
    auto* hint = new QLabel(
        "Select a channel (or a zone header) to edit its fields. Changes update the in-memory "
        "codeplug; use File > Save to write them.");
    hint->setObjectName("muted");
    hint->setWordWrap(true);
    lay->addWidget(hint);

    zones_ = new QTableWidget(0, 5);
    zones_->setHorizontalHeaderLabels({"#", "Channel", "Mode", "RX MHz", "TX MHz"});
    zones_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zones_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    zones_->verticalHeader()->setVisible(false);
    zones_->verticalHeader()->setDefaultSectionSize(34);
    zones_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    zones_->setSelectionBehavior(QAbstractItemView::SelectRows);
    zones_->setShowGrid(false);
    lay->addWidget(zones_, 1);

    auto* editor = new QFrame;
    editor->setObjectName("card");
    auto* el = new QVBoxLayout(editor);
    el->setContentsMargins(16, 14, 16, 14);
    el->setSpacing(10);

    auto* row1 = new QHBoxLayout;
    row1->setSpacing(8);
    row1->addWidget(new QLabel("Zone"));
    zoneNameEdit_ = new QLineEdit;
    row1->addWidget(zoneNameEdit_, 1);
    row1->addWidget(new QLabel("Channel"));
    channelNameEdit_ = new QLineEdit;
    row1->addWidget(channelNameEdit_, 1);
    el->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    row2->setSpacing(8);
    row2->addWidget(new QLabel("RX MHz"));
    rxEdit_ = new QLineEdit;
    rxEdit_->setMaximumWidth(110);
    row2->addWidget(rxEdit_);
    row2->addWidget(new QLabel("TX MHz"));
    txEdit_ = new QLineEdit;
    txEdit_->setMaximumWidth(110);
    row2->addWidget(txEdit_);
    row2->addWidget(new QLabel("RX squelch"));
    rxSqEdit_ = new QLineEdit;
    rxSqEdit_->setMaximumWidth(70);
    row2->addWidget(rxSqEdit_);
    row2->addWidget(new QLabel("TX squelch"));
    txSqEdit_ = new QLineEdit;
    txSqEdit_->setMaximumWidth(70);
    row2->addWidget(txSqEdit_);
    row2->addStretch(1);
    auto* apply = new QPushButton("Apply changes");
    apply->setObjectName("accent");
    connect(apply, &QPushButton::clicked, this, &MainWindow::applyChannelEdit);
    row2->addWidget(apply);
    el->addLayout(row2);
    lay->addWidget(editor);

    connect(zones_, &QTableWidget::itemSelectionChanged, this, [this] {
        const int row = zones_->currentRow();
        if (row < 0) return;
        auto* it = zones_->item(row, 0);
        if (!it) return;
        editZone_ = it->data(kRoleZone).toInt();
        editChannel_ = it->data(kRoleChannel).toInt();
        if (!model_ || editZone_ < 0) return;
        const auto* z = model_->Zone(editZone_);
        if (!z) return;
        zoneNameEdit_->setText(QString::fromStdString(z->name));
        const auto* ch = editChannel_ >= 0 ? model_->Channel(editZone_, editChannel_) : nullptr;
        for (auto* edit : {channelNameEdit_, rxEdit_, txEdit_, rxSqEdit_, txSqEdit_})
            edit->setEnabled(ch != nullptr);
        if (ch) {
            channelNameEdit_->setText(QString::fromStdString(ch->name));
            rxEdit_->setText(QString::number(ch->rxMHz, 'f', 5));
            txEdit_->setText(QString::number(ch->txMHz, 'f', 5));
            rxSqEdit_->setText(QString::number(ch->squelchRx));
            txSqEdit_->setText(QString::number(ch->squelchTx));
        } else {
            channelNameEdit_->clear();
            rxEdit_->clear();
            txEdit_->clear();
            rxSqEdit_->clear();
            txSqEdit_->clear();
        }
    });
    return page;
}

QWidget* MainWindow::buildSectionPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(10);
    sectionTitle_ = new QLabel("Section");
    sectionTitle_->setObjectName("pageTitle");
    lay->addWidget(sectionTitle_);
    sectionSubtitle_ = new QLabel;
    sectionSubtitle_->setObjectName("muted");
    sectionSubtitle_->setWordWrap(true);
    lay->addWidget(sectionSubtitle_);

    sectionRecords_ = new QTableWidget(0, 5);
    sectionRecords_->setHorizontalHeaderLabels(
        {"Record ID", "Tag", "Inst", "Bytes", "Family / Name"});
    sectionRecords_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    sectionRecords_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    sectionRecords_->verticalHeader()->setVisible(false);
    sectionRecords_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sectionRecords_->setSelectionBehavior(QAbstractItemView::SelectRows);
    sectionRecords_->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(sectionRecords_, 2);

    auto* editRow = new QHBoxLayout;
    editRow->setSpacing(12);

    auto* namesCol = new QVBoxLayout;
    auto* namesLabel = new QLabel("Names");
    namesLabel->setObjectName("sectionTitle");
    namesCol->addWidget(namesLabel);
    sectionNames_ = new QTableWidget(0, 2);
    sectionNames_->setHorizontalHeaderLabels({"#", "Name"});
    sectionNames_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    sectionNames_->verticalHeader()->setVisible(false);
    sectionNames_->setSelectionBehavior(QAbstractItemView::SelectRows);
    namesCol->addWidget(sectionNames_, 1);
    sectionApplyNames_ = new QPushButton("Apply names");
    sectionApplyNames_->setObjectName("accent");
    connect(sectionApplyNames_, &QPushButton::clicked, this, &MainWindow::applySectionNames);
    namesCol->addWidget(sectionApplyNames_, 0, Qt::AlignLeft);
    editRow->addLayout(namesCol, 1);

    auto* payloadCol = new QVBoxLayout;
    auto* nameLabel = new QLabel("Name");
    nameLabel->setObjectName("sectionTitle");
    payloadCol->addWidget(nameLabel);
    sectionNameEdit_ = new QLineEdit;
    sectionNameEdit_->setVisible(false);
    payloadCol->addWidget(sectionNameEdit_);
    auto* payloadLabel = new QLabel("Record payload (hex)");
    payloadLabel->setObjectName("sectionTitle");
    payloadCol->addWidget(payloadLabel);
    sectionHex_ = new QPlainTextEdit;
    sectionHex_->setPlaceholderText("Select a record to edit its payload.");
    sectionHex_->setFont(QFont("Consolas", 10));
    payloadCol->addWidget(sectionHex_, 1);
    auto* applyPayload = new QPushButton("Apply payload");
    applyPayload->setObjectName("accent");
    connect(applyPayload, &QPushButton::clicked, this, &MainWindow::applySectionPayload);
    payloadCol->addWidget(applyPayload, 0, Qt::AlignLeft);
    editRow->addLayout(payloadCol, 1);

    lay->addLayout(editRow, 3);

    sectionDetail_ = new QPlainTextEdit;
    sectionDetail_->setReadOnly(true);
    sectionDetail_->setMaximumHeight(130);
    sectionDetail_->setFont(QFont("Consolas", 10));
    lay->addWidget(sectionDetail_);

    connect(sectionRecords_, &QTableWidget::itemSelectionChanged, this,
            [this] { refreshSectionRecord(sectionRecords_->currentRow()); });
    return page;
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

void MainWindow::saveCodeplug() {
    if (!have_) return;
    if (path_.isEmpty() || path_ == "(radio)") {
        saveCodeplugAs();
        return;
    }
    std::string err;
    if (!opendepot::SaveCodeplug(std::filesystem::path(path_.toStdWString()), cp_, err)) {
        QMessageBox::critical(this, "Save codeplug",
                              QString::fromStdString("Cannot save: " + err));
        return;
    }
    dirty_ = false;
    const QFileInfo info(path_);
    setWindowTitle(info.fileName().isEmpty() ? "OpenDepot"
                                             : QString("OpenDepot - %1").arg(info.fileName()));
    updateDashboard();
    statusBar()->showMessage("Saved " + path_, 8000);
}

void MainWindow::saveCodeplugAs() {
    if (!have_) return;
    const QString start = (path_.isEmpty() || path_ == "(radio)") ? QString() : path_;
    const QString file = QFileDialog::getSaveFileName(this, "Save codeplug", start,
                                                      "Codeplug (*.cpg)");
    if (file.isEmpty()) return;
    std::string err;
    if (!opendepot::SaveCodeplug(std::filesystem::path(file.toStdWString()), cp_, err)) {
        QMessageBox::critical(this, "Save codeplug",
                              QString::fromStdString("Cannot save: " + err));
        return;
    }
    path_ = file;
    dirty_ = false;
    const QFileInfo info(path_);
    setWindowTitle(info.fileName().isEmpty() ? "OpenDepot"
                                             : QString("OpenDepot - %1").arg(info.fileName()));
    updateDashboard();
    statusBar()->showMessage("Saved " + file, 8000);
}

void MainWindow::setCodeplug(opendepot::Codeplug cp, const QString& path) {
    cp_ = std::move(cp);
    have_ = true;
    path_ = path;
    openedAt_ = QDateTime::currentDateTime();
    dirty_ = false;
    saveAct_->setEnabled(true);
    model_ = std::make_unique<opendepot::CodeplugModel>(cp_);
    std::string err;
    if (!model_->Reload(err)) {
        model_.reset();
        QMessageBox::warning(this, "Codeplug model", QString::fromStdString(err));
    }
    editZone_ = -1;
    editChannel_ = -1;
    sectionIndex_ = -1;
    buildModelTree();
    updateDashboard();
    pages_->setCurrentIndex(kInfoPage);
    tree_->setCurrentItem(treeRoot_);
    const QFileInfo info(path_);
    setWindowTitle(info.fileName().isEmpty() ? "OpenDepot"
                                             : QString("OpenDepot - %1").arg(info.fileName()));
    statusBar()->showMessage("Loaded " + path, 8000);
}

void MainWindow::updateDashboard() {
    if (!have_) {
        serial_->setText("-");
        modelLabel_->setText("-");
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
    modelLabel_->setText(QString::fromStdString(cp_.header.model));
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
    if (!table) return;
    table->setRowCount(0);
    if (!model_) return;
    int index = 0;
    for (const auto& z : model_->Zones()) {
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
            item->setData(kRoleZone, z.index);
            item->setData(kRoleChannel, -1);
            table->setItem(row, 0, item);
            table->setSpan(row, 0, 1, 5);
            table->setRowHeight(row, 34);
        }
        if (z.channels.empty()) {
            const int row = table->rowCount();
            table->insertRow(row);
            for (int c = 0; c < 5; ++c) {
                auto* it = new QTableWidgetItem(c == 1 ? "-" : "");
                it->setData(kRoleZone, z.index);
                it->setData(kRoleChannel, -1);
                table->setItem(row, c, it);
            }
            continue;
        }
        for (const auto& ch : z.channels) {
            const int row = table->rowCount();
            table->insertRow(row);
            auto add = [&](int col, const QString& text) {
                auto* it = new QTableWidgetItem(text);
                it->setData(kRoleZone, z.index);
                it->setData(kRoleChannel, ch.index);
                table->setItem(row, col, it);
            };
            add(0, QString::number(index++));
            add(1, QString::fromStdString(ch.name));
            add(2, ch.rxWord == ch.txWord ? "Simple" : "Split");
            add(3, QString::number(ch.rxMHz, 'f', 5));
            add(4, QString::number(ch.txMHz, 'f', 5));
        }
    }
}

void MainWindow::selectChannelRow(int zone, int channel) {
    if (!zones_) return;
    for (int row = 0; row < zones_->rowCount(); ++row) {
        auto* it = zones_->item(row, 0);
        if (!it) continue;
        if (it->data(kRoleZone).toInt() == zone && it->data(kRoleChannel).toInt() == channel) {
            zones_->selectRow(row);
            return;
        }
    }
}

const opendepot::RecordNode* MainWindow::findNode(uint32_t id) const {
    if (!model_) return nullptr;
    for (const auto& n : model_->RecordNodes())
        if (n.id == id) return &n;
    return nullptr;
}

void MainWindow::showSection(int index) {
    if (!model_ || index < 0 || index >= static_cast<int>(model_->Sections().size())) return;
    sectionIndex_ = index;
    const auto& s = model_->Sections()[index];
    sectionTitle_->setText(QString::fromStdString(s.name));
    sectionSubtitle_->setText(QString("%1  -  %2 record(s)  -  confidence: %3")
                                  .arg(QString::fromStdString(s.group))
                                  .arg(static_cast<int>(s.records.size()))
                                  .arg(QString::fromUtf8(opendepot::ConfidenceName(s.confidence))));
    refreshSectionList();
}

void MainWindow::refreshSectionList() {
    if (!model_ || !sectionRecords_ || sectionIndex_ < 0 ||
        sectionIndex_ >= static_cast<int>(model_->Sections().size()))
        return;
    const QSignalBlocker block(sectionRecords_);
    sectionRecords_->setRowCount(0);
    const auto& s = model_->Sections()[sectionIndex_];
    for (uint32_t id : s.records) {
        const auto* n = findNode(id);
        if (!n) continue;
        const int row = sectionRecords_->rowCount();
        sectionRecords_->insertRow(row);
        auto* idItem = new QTableWidgetItem(
            QString("0x%1").arg(static_cast<qulonglong>(id), 8, 16, QChar('0')).toUpper());
        idItem->setData(kRoleRecordId, id);
        sectionRecords_->setItem(row, 0, idItem);
        sectionRecords_->setItem(
            row, 1,
            new QTableWidgetItem(
                QString("0x%1").arg(static_cast<int>(n->tag), 4, 16, QChar('0')).toUpper()));
        sectionRecords_->setItem(row, 2, new QTableWidgetItem(QString::number(n->inst)));
        sectionRecords_->setItem(row, 3, new QTableWidgetItem(QString::number(n->length)));
        QString desc = QString::fromStdString(n->family);
        if (!n->name.empty()) desc += "  -  " + QString::fromStdString(n->name);
        sectionRecords_->setItem(row, 4, new QTableWidgetItem(desc));
    }
    if (sectionRecords_->rowCount() > 0) sectionRecords_->selectRow(0);
}

void MainWindow::refreshSectionRecord(int row) {
    if (!sectionRecords_ || row < 0) return;
    auto* idItem = sectionRecords_->item(row, 0);
    if (!idItem) return;
    const uint32_t id = idItem->data(kRoleRecordId).toUInt();
    const auto* n = findNode(id);
    if (!n) return;

    sectionHex_->setPlainText(HexBytes(n->payload));
    const bool isTable = n->family == "name_table" && !n->names.empty();
    const bool isName = n->family == "name";
    sectionNames_->setVisible(isTable);
    sectionNameEdit_->setVisible(isName);
    sectionApplyNames_->setVisible(isTable || isName);
    if (isTable) {
        sectionNames_->setRowCount(0);
        for (size_t i = 0; i < n->names.size(); ++i) {
            sectionNames_->insertRow(int(i));
            sectionNames_->setItem(int(i), 0, new QTableWidgetItem(QString::number(i + 1)));
            sectionNames_->setItem(int(i), 1,
                                   new QTableWidgetItem(QString::fromStdString(n->names[i])));
        }
    } else {
        sectionNames_->setRowCount(0);
    }
    if (isName) sectionNameEdit_->setText(QString::fromStdString(n->name));
    else sectionNameEdit_->clear();

    QString detail;
    detail += QString("Record 0x%1   tag 0x%2   instance %3   %4 bytes\n")
                  .arg(static_cast<qulonglong>(id), 8, 16, QChar('0'))
                  .arg(static_cast<int>(n->tag), 4, 16, QChar('0'))
                  .arg(static_cast<int>(n->inst))
                  .arg(n->length)
                  .toUpper();
    detail += QString("Family: %1   Section: %2\n")
                  .arg(QString::fromStdString(n->family),
                       QString::fromStdString(n->section));
    if (!n->strings.empty()) {
        QStringList ss;
        for (const auto& s : n->strings) ss << QString::fromStdString(s);
        detail += "Strings: " + ss.join(" | ") + "\n";
    }
    if (!n->references.empty()) {
        QStringList rs;
        for (const auto& r : n->references)
            rs << QString("0x%1/%2@%3")
                      .arg(static_cast<int>(r.tag), 4, 16, QChar('0'))
                      .arg(static_cast<int>(r.inst))
                      .arg(r.offset);
        detail += "References: " + rs.join(", ") + "\n";
    }
    sectionDetail_->setPlainText(detail);
}

void MainWindow::applySectionPayload() {
    if (!model_ || !sectionRecords_) return;
    const int row = sectionRecords_->currentRow();
    if (row < 0) return;
    auto* idItem = sectionRecords_->item(row, 0);
    if (!idItem) return;
    const uint32_t id = idItem->data(kRoleRecordId).toUInt();
    const auto* n = findNode(id);
    if (!n) return;
    std::vector<uint8_t> bytes;
    if (!ParseHex(sectionHex_->toPlainText(), bytes)) {
        QMessageBox::warning(this, "Payload", "The payload must be an even number of hex digits.");
        return;
    }
    const uint16_t tag = n->tag;
    const uint16_t inst = n->inst;
    if (!model_->SetRecordPayload(tag, inst, bytes)) {
        QMessageBox::warning(this, "Payload", "The record payload could not be replaced.");
        return;
    }
    refreshAfterEdit();
    if (row < sectionRecords_->rowCount()) sectionRecords_->selectRow(row);
    statusBar()->showMessage("Record payload updated in memory - File > Save to keep it.", 8000);
}

void MainWindow::applySectionNames() {
    if (!model_ || !sectionRecords_) return;
    const int row = sectionRecords_->currentRow();
    if (row < 0) return;
    auto* idItem = sectionRecords_->item(row, 0);
    if (!idItem) return;
    const uint32_t id = idItem->data(kRoleRecordId).toUInt();
    const auto* n = findNode(id);
    if (!n) return;
    std::vector<uint8_t> payload = n->payload;
    bool changed = false;
    if (n->family == "name_table") {
        const bool special = n->tag == 0x0FFF;
        const int count = special ? Be16(payload, 4) : Be16(payload, 6);
        const int entrySize = special ? 17 : Be16(payload, 2);
        const size_t base = special ? 6 : 8;
        if (entrySize < 2) return;
        const int rows = std::min(count, sectionNames_->rowCount());
        for (int i = 0; i < rows; ++i) {
            auto* nameItem = sectionNames_->item(i, 1);
            if (!nameItem) continue;
            PutName(payload, base + size_t(i) * size_t(entrySize), size_t(entrySize),
                    nameItem->text());
            changed = true;
        }
    } else if (n->family == "name") {
        PutName(payload, 6, payload.size() - 6, sectionNameEdit_->text());
        changed = true;
    }
    if (!changed) return;
    const uint16_t tag = n->tag;
    const uint16_t inst = n->inst;
    if (!model_->SetRecordPayload(tag, inst, payload)) {
        QMessageBox::warning(this, "Names", "The name record could not be updated.");
        return;
    }
    refreshAfterEdit();
    if (row < sectionRecords_->rowCount()) sectionRecords_->selectRow(row);
    statusBar()->showMessage("Names updated in memory - File > Save to keep them.", 8000);
}

void MainWindow::applyChannelEdit() {
    if (!model_ || editZone_ < 0) return;
    const auto* z = model_->Zone(editZone_);
    if (!z) return;
    const std::string oldZone = z->name;
    const auto* ch = editChannel_ >= 0 ? model_->Channel(editZone_, editChannel_) : nullptr;
    const std::string oldName = ch ? ch->name : std::string();
    const uint32_t oldRxWord = ch ? ch->rxWord : 0;
    const uint32_t oldTxWord = ch ? ch->txWord : 0;
    const int oldRxSq = ch ? ch->squelchRx : 0;
    const int oldTxSq = ch ? ch->squelchTx : 0;

    bool changed = false;
    const std::string newZone = zoneNameEdit_->text().toStdString();
    if (newZone != oldZone) {
        if (!model_->SetZoneName(editZone_, newZone)) {
            QMessageBox::warning(this, "Zone", "The zone name could not be updated.");
            return;
        }
        changed = true;
    }
    if (ch) {
        const std::string newName = channelNameEdit_->text().toStdString();
        if (newName != oldName) {
            if (!model_->SetChannelName(editZone_, editChannel_, newName)) {
                QMessageBox::warning(this, "Channel", "The channel name could not be updated.");
                return;
            }
            changed = true;
        }
        bool ok = false;
        const double rx = rxEdit_->text().toDouble(&ok);
        if (!ok || rx <= 0) {
            QMessageBox::warning(this, "Channel", "RX MHz is not a valid frequency.");
            return;
        }
        const double tx = txEdit_->text().toDouble(&ok);
        if (!ok || tx <= 0) {
            QMessageBox::warning(this, "Channel", "TX MHz is not a valid frequency.");
            return;
        }
        if (opendepot::MHzToWord(rx) != oldRxWord || opendepot::MHzToWord(tx) != oldTxWord) {
            if (!model_->SetChannelFrequency(editZone_, editChannel_, rx, tx)) {
                QMessageBox::warning(this, "Channel", "The frequency could not be updated.");
                return;
            }
            changed = true;
        }
        const int rxSq = rxSqEdit_->text().toInt(&ok);
        if (!ok || rxSq < 0 || rxSq > 255) {
            QMessageBox::warning(this, "Channel", "RX squelch must be 0-255.");
            return;
        }
        const int txSq = txSqEdit_->text().toInt(&ok);
        if (!ok || txSq < 0 || txSq > 255) {
            QMessageBox::warning(this, "Channel", "TX squelch must be 0-255.");
            return;
        }
        if (rxSq != oldRxSq || txSq != oldTxSq) {
            if (!model_->SetChannelSquelch(editZone_, editChannel_, rxSq, txSq)) {
                QMessageBox::warning(this, "Channel", "The squelch values could not be updated.");
                return;
            }
            changed = true;
        }
    }
    if (!changed) {
        statusBar()->showMessage("No changes to apply.", 5000);
        return;
    }
    const int zone = editZone_, channel = editChannel_;
    refreshAfterEdit();
    selectChannelRow(zone, channel);
    statusBar()->showMessage("Channel updated in memory - File > Save to keep it.", 8000);
}

void MainWindow::refreshAfterEdit() {
    dirty_ = true;
    const QFileInfo info(path_);
    setWindowTitle(info.fileName().isEmpty() ? "OpenDepot *"
                                             : QString("OpenDepot * - %1").arg(info.fileName()));
    updateDashboard();
    if (sectionIndex_ >= 0) refreshSectionList();
}

void MainWindow::refreshZones() {
    if (!model_ || !zones_) return;
    fillZones(zones_, true);
    selectChannelRow(editZone_, editChannel_);
}

void MainWindow::showZones() {
    if (zonesItem_) tree_->setCurrentItem(zonesItem_);
    pages_->setCurrentIndex(kZonesPage);
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
