// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include "MainWindow.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <QAction>
#include <QAbstractItemView>
#include <QButtonGroup>
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

#include "opendepot/core/field_catalog.h"
#include "opendepot/core/depot_profiles.h"
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
constexpr int kRadioWidePage = 7;
constexpr int kCatalogPage = 8;
constexpr int kCpsTablesPage = 9;
constexpr int kRolePage = Qt::UserRole;
constexpr int kRoleSection = Qt::UserRole + 1;
constexpr int kRoleZone = Qt::UserRole + 2;
constexpr int kRoleChannel = Qt::UserRole + 3;
constexpr int kRoleRecordId = Qt::UserRole + 4;

QString EnumText(const QString& field, uint64_t raw) {
    if (field == "Out of Range Indicator" || field == "Imbalanced Coverage Indicator" ||
        field == "System Registration Indicator") {
        static const char* values[] = {"No Indication", "Alert Only", "Display Only",
                                       "Alert & Display"};
        return raw < 4 ? QString::fromLatin1(values[raw]) : QString();
    }
    if (field == "Time format") {
        static const char* values[] = {"12 Hour", "24 Hour", "12 Hour with Seconds",
                                       "24 Hour with Seconds"};
        return raw < 4 ? QString::fromLatin1(values[raw]) : QString();
    }
    if (field == "Date Format") {
        static const char* values[] = {"MM/DD/YYYY", "DD/MM/YYYY", "YYYY/MM/DD", "MM/DD/YY"};
        return raw < 4 ? QString::fromLatin1(values[raw]) : QString();
    }
    if (field == "Channel Announcement Priority") return raw ? "High" : "Normal";
    if (field == "Key Type") {
        static const char* values[] = {"None", "Advanced System Key", "Advanced WACN Key"};
        return raw < 3 ? QString::fromLatin1(values[raw]) : QString();
    }
    return {};
}

enum class Glyph {
    Radio, Sliders, List, Wave, Globe, Control, Display, Phone, Shield, Layers, Antenna,
    Users, Keypad, Scan, Document, Wrench, Gear, Table, Grid, Chat, Star, Diff, Info,
    Diamond, Folder
};

QIcon DrawGlyph(Glyph glyph, const QColor& color) {
    QPixmap pm(18, 18);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 1.55, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    const QPointF c(9, 9);

    switch (glyph) {
    case Glyph::Radio:
        p.drawRoundedRect(QRectF(2.5, 7, 12, 8), 2, 2);
        p.drawLine(QPointF(13, 7), QPointF(15, 2.5));
        p.drawEllipse(QPointF(6, 11), 1.6, 1.6);
        p.drawLine(QPointF(9.5, 10), QPointF(12, 10));
        p.drawLine(QPointF(9.5, 12.5), QPointF(12, 12.5));
        break;
    case Glyph::Sliders:
        for (int y : {4, 9, 14}) p.drawLine(QPointF(2.5, y), QPointF(15.5, y));
        p.drawEllipse(QPointF(6, 4), 1.7, 1.7);
        p.drawEllipse(QPointF(12, 9), 1.7, 1.7);
        p.drawEllipse(QPointF(8, 14), 1.7, 1.7);
        break;
    case Glyph::List:
        for (int y : {4, 9, 14}) {
            p.drawEllipse(QPointF(3.4, y), 1.05, 1.05);
            p.drawLine(QPointF(6.2, y), QPointF(15.5, y));
        }
        break;
    case Glyph::Wave:
        p.drawArc(QRectF(1.5, 5, 8, 11), 70 * 16, 150 * 16);
        p.drawArc(QRectF(4, 3, 11, 14), 70 * 16, 150 * 16);
        p.drawArc(QRectF(7, 1, 14, 17), 70 * 16, 150 * 16);
        break;
    case Glyph::Globe:
        p.drawEllipse(c, 6.7, 6.7);
        p.drawEllipse(QRectF(4.5, 2.3, 9, 13.4));
        p.drawLine(QPointF(2.5, 9), QPointF(15.5, 9));
        break;
    case Glyph::Control:
        p.drawEllipse(QPointF(9, 8), 4.5, 4.5);
        p.drawEllipse(QPointF(9, 8), 1.3, 1.3);
        p.drawLine(QPointF(9, 12.5), QPointF(9, 16));
        p.drawLine(QPointF(5.5, 16), QPointF(12.5, 16));
        break;
    case Glyph::Display:
        p.drawRoundedRect(QRectF(2, 3, 14, 10), 1.5, 1.5);
        p.drawLine(QPointF(9, 13), QPointF(9, 16));
        p.drawLine(QPointF(6, 16), QPointF(12, 16));
        break;
    case Glyph::Phone:
        p.drawPath([] {
            QPainterPath path;
            path.moveTo(5, 2.5);
            path.cubicTo(2.5, 5, 4.5, 11, 9.5, 14.5);
            path.cubicTo(12, 16, 15, 16.5, 16, 13.5);
            path.lineTo(12.5, 11.5);
            path.lineTo(10.5, 13);
            path.cubicTo(8.5, 12, 6.5, 9.5, 6.2, 7);
            path.lineTo(8.5, 6);
            path.closeSubpath();
            return path;
        }());
        break;
    case Glyph::Shield:
        p.drawPath([] {
            QPainterPath path;
            path.moveTo(9, 1.8);
            path.lineTo(15, 4.3);
            path.lineTo(15, 9.2);
            path.cubicTo(15, 13.3, 12.2, 15.6, 9, 16.7);
            path.cubicTo(5.8, 15.6, 3, 13.3, 3, 9.2);
            path.lineTo(3, 4.3);
            path.closeSubpath();
            return path;
        }());
        p.drawLine(QPointF(6.4, 8.7), QPointF(8.3, 10.5));
        p.drawLine(QPointF(8.3, 10.5), QPointF(12, 6.3));
        break;
    case Glyph::Layers:
        for (int dy : {-4, 0, 4}) {
            p.drawLine(QPointF(9, 3 + dy), QPointF(15, 6 + dy));
            p.drawLine(QPointF(15, 6 + dy), QPointF(9, 9 + dy));
            p.drawLine(QPointF(9, 9 + dy), QPointF(3, 6 + dy));
            p.drawLine(QPointF(3, 6 + dy), QPointF(9, 3 + dy));
        }
        break;
    case Glyph::Antenna:
        p.drawLine(QPointF(9, 2.5), QPointF(5.5, 15.5));
        p.drawLine(QPointF(9, 2.5), QPointF(12.5, 15.5));
        p.drawLine(QPointF(6.8, 10.5), QPointF(11.2, 10.5));
        p.drawLine(QPointF(6, 5.5), QPointF(12, 5.5));
        p.drawLine(QPointF(4.5, 15.5), QPointF(13.5, 15.5));
        break;
    case Glyph::Users:
        p.drawEllipse(QPointF(7, 6), 2.5, 2.5);
        p.drawEllipse(QPointF(13, 7.2), 1.8, 1.8);
        p.drawArc(QRectF(2.5, 9, 9, 7), 0, 180 * 16);
        p.drawArc(QRectF(10.5, 10, 6, 6), 0, 140 * 16);
        break;
    case Glyph::Keypad:
        for (int y = 3; y <= 15; y += 6)
            for (int x = 3; x <= 15; x += 6) p.drawEllipse(QPointF(x, y), 1.35, 1.35);
        break;
    case Glyph::Scan:
        p.drawEllipse(QPointF(7.6, 7.6), 4.8, 4.8);
        p.drawLine(QPointF(11.2, 11.2), QPointF(16, 16));
        p.drawLine(QPointF(5.4, 7.6), QPointF(9.8, 7.6));
        break;
    case Glyph::Document:
        p.drawRoundedRect(QRectF(3.5, 2, 11, 14), 1.4, 1.4);
        p.drawLine(QPointF(6, 6), QPointF(12, 6));
        p.drawLine(QPointF(6, 9), QPointF(12, 9));
        p.drawLine(QPointF(6, 12), QPointF(10, 12));
        break;
    case Glyph::Wrench:
        p.drawLine(QPointF(3.5, 14.5), QPointF(10.5, 7.5));
        p.drawArc(QRectF(9, 2, 7.5, 7.5), 30 * 16, 210 * 16);
        p.drawArc(QRectF(2, 10.5, 6, 6), 30 * 16, 210 * 16);
        break;
    case Glyph::Gear:
        p.drawEllipse(c, 4.2, 4.2);
        for (int i = 0; i < 8; ++i) {
            const double a = i * 3.14159265358979323846 / 4.0;
            p.drawLine(QPointF(9 + std::cos(a) * 5, 9 + std::sin(a) * 5),
                       QPointF(9 + std::cos(a) * 7.2, 9 + std::sin(a) * 7.2));
        }
        break;
    case Glyph::Table:
        p.drawRoundedRect(QRectF(2, 3, 14, 12), 1.4, 1.4);
        p.drawLine(QPointF(2, 7), QPointF(16, 7));
        p.drawLine(QPointF(2, 11), QPointF(16, 11));
        p.drawLine(QPointF(6, 3), QPointF(6, 15));
        p.drawLine(QPointF(11, 3), QPointF(11, 15));
        break;
    case Glyph::Grid:
        p.drawRoundedRect(QRectF(2.5, 2.5, 5.2, 5.2), 1, 1);
        p.drawRoundedRect(QRectF(10.3, 2.5, 5.2, 5.2), 1, 1);
        p.drawRoundedRect(QRectF(2.5, 10.3, 5.2, 5.2), 1, 1);
        p.drawRoundedRect(QRectF(10.3, 10.3, 5.2, 5.2), 1, 1);
        break;
    case Glyph::Chat:
        p.drawRoundedRect(QRectF(2, 3, 14, 10), 3, 3);
        p.drawLine(QPointF(6, 13), QPointF(5, 16));
        p.drawLine(QPointF(5, 16), QPointF(9, 13));
        p.drawLine(QPointF(5.5, 8), QPointF(12.5, 8));
        break;
    case Glyph::Star:
        p.drawPath([] {
            QPainterPath path;
            for (int i = 0; i < 10; ++i) {
                const double a = -3.14159265358979323846 / 2 + i * 3.14159265358979323846 / 5;
                const double r = i % 2 ? 3.4 : 7.0;
                const QPointF pt(9 + std::cos(a) * r, 9 + std::sin(a) * r);
                i ? path.lineTo(pt) : path.moveTo(pt);
            }
            path.closeSubpath();
            return path;
        }());
        break;
    case Glyph::Diff:
        p.drawLine(QPointF(3, 6), QPointF(14, 6));
        p.drawLine(QPointF(11, 3), QPointF(14, 6));
        p.drawLine(QPointF(14, 12), QPointF(3, 12));
        p.drawLine(QPointF(6, 15), QPointF(3, 12));
        break;
    case Glyph::Info:
        p.drawEllipse(c, 6.8, 6.8);
        p.drawLine(QPointF(9, 8), QPointF(9, 13));
        p.drawEllipse(QPointF(9, 5.3), 0.9, 0.9);
        break;
    case Glyph::Diamond:
        p.drawLine(QPointF(9, 2), QPointF(16, 9));
        p.drawLine(QPointF(16, 9), QPointF(9, 16));
        p.drawLine(QPointF(9, 16), QPointF(2, 9));
        p.drawLine(QPointF(2, 9), QPointF(9, 2));
        break;
    case Glyph::Folder:
        p.drawPath([] {
            QPainterPath path;
            path.moveTo(2, 5);
            path.lineTo(7, 5);
            path.lineTo(8.5, 7);
            path.lineTo(16, 7);
            path.lineTo(16, 14.5);
            path.lineTo(2, 14.5);
            path.closeSubpath();
            return path;
        }());
        break;
    }
    return QIcon(pm);
}

struct GlyphSpec {
    const char* match;
    Glyph glyph;
    const char* color;
};

QIcon TreeIcon(const QString& text, bool isGroup) {
    static const GlyphSpec specs[] = {
        {"Radio Information", Glyph::Info, "#38bdf8"},
        {"Radio Wide", Glyph::Sliders, "#60a5fa"},
        {"Test Channel", Glyph::List, "#60a5fa"},
        {"Narrow Bandwidth", Glyph::Wave, "#38bdf8"},
        {"NAT", Glyph::Globe, "#38bdf8"},
        {"Control", Glyph::Control, "#60a5fa"},
        {"Display", Glyph::Display, "#22d3ee"},
        {"Menu", Glyph::Display, "#22d3ee"},
        {"Phone", Glyph::Phone, "#c084fc"},
        {"Secure", Glyph::Shield, "#fb7185"},
        {"Encryption", Glyph::Shield, "#fb7185"},
        {"Conventional", Glyph::Radio, "#34d399"},
        {"Personality", Glyph::Users, "#34d399"},
        {"Alias", Glyph::Chat, "#a3e635"},
        {"MDC", Glyph::Diamond, "#fbbf24"},
        {"Quick-Call", Glyph::Star, "#fbbf24"},
        {"DTMF", Glyph::Keypad, "#2dd4bf"},
        {"MPL", Glyph::List, "#34d399"},
        {"Scan", Glyph::Scan, "#22d3ee"},
        {"Zone", Glyph::Layers, "#60a5fa"},
        {"Trunk", Glyph::Antenna, "#fb923c"},
        {"Talkgroup", Glyph::Users, "#fb923c"},
        {"System", Glyph::Gear, "#fb923c"},
        {"List", Glyph::List, "#94a3b8"},
        {"Directory", Glyph::Document, "#94a3b8"},
        {"Tools", Glyph::Wrench, "#cbd5e1"},
        {"Report", Glyph::Document, "#22d3ee"},
        {"Doctor", Glyph::Info, "#fb7185"},
        {"Diff", Glyph::Diff, "#a78bfa"},
        {"Radio", Glyph::Radio, "#60a5fa"},
        {"CPS Tables", Glyph::Table, "#38bdf8"},
        {"Field Catalog", Glyph::Grid, "#38bdf8"},
    };
    for (const auto& spec : specs) {
        if (text.contains(QString::fromLatin1(spec.match), Qt::CaseInsensitive))
            return DrawGlyph(spec.glyph, QColor(spec.color));
    }
    return DrawGlyph(isGroup ? Glyph::Folder : Glyph::Document,
                    QColor(isGroup ? "#64748b" : "#94a3b8"));
}

bool WriteFieldBits(std::vector<uint8_t>& data, uint16_t off, uint16_t bits, uint8_t msb,
                    uint64_t value) {
    if (bits == 0 || bits > 32) return false;
    const size_t pos = size_t(off) * 8 + (7 - msb);
    if (pos + bits > data.size() * 8) return false;
    for (uint16_t i = 0; i < bits; ++i) {
        const size_t bit = pos + i;
        const bool set = (value >> (bits - 1 - i)) & 1;
        const uint8_t mask = uint8_t(1u << (7 - (bit % 8)));
        if (set)
            data[bit / 8] |= mask;
        else
            data[bit / 8] &= uint8_t(~mask);
    }
    return true;
}

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

QStringList AvailableComPorts() {
    QStringList ports;
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &key) ==
        ERROR_SUCCESS) {
        wchar_t name[256];
        wchar_t data[256];
        for (DWORD i = 0;; ++i) {
            DWORD nameLen = 256;
            DWORD dataLen = sizeof(data);
            DWORD type = 0;
            const LONG r = RegEnumValueW(key, i, name, &nameLen, nullptr, &type,
                                         reinterpret_cast<LPBYTE>(data), &dataLen);
            if (r == ERROR_NO_MORE_ITEMS) break;
            if (r != ERROR_SUCCESS) break;
            if (type == REG_SZ) ports << QString::fromWCharArray(data).trimmed();
        }
        RegCloseKey(key);
    }
    ports.removeDuplicates();
    std::sort(ports.begin(), ports.end(), [](const QString& a, const QString& b) {
        const bool ca = a.startsWith("COM", Qt::CaseInsensitive);
        const bool cb = b.startsWith("COM", Qt::CaseInsensitive);
        if (ca != cb) return ca;
        return a.mid(3).toInt() < b.mid(3).toInt();
    });
    return ports;
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

    auto* exportTxt = file->addAction("Export Plain &Text...");
    connect(exportTxt, &QAction::triggered, this, &MainWindow::exportPlainText);

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

    auto* depot = tools->addMenu("&Depot");
    auto* create = depot->addAction(DrawGlyph(Glyph::Document, QColor("#38bdf8")),
                                    "&Create Codeplug...");
    connect(create, &QAction::triggered, this, &MainWindow::createCodeplug);
    auto* write = depot->addAction(DrawGlyph(Glyph::Radio, QColor("#fb923c")),
                                   "&Write Codeplug...");
    connect(write, &QAction::triggered, this, &MainWindow::writeRadio);

    auto* radio = menuBar()->addMenu("&Radio");
    connect(radio->addAction("&Read Radio"), &QAction::triggered, this, &MainWindow::readRadio);
    connect(radio->addAction("&Write Radio..."), &QAction::triggered, this,
            &MainWindow::writeRadio);

    auto* help = menuBar()->addMenu("&Help");
    connect(help->addAction("&About"), &QAction::triggered, this, &MainWindow::about);

    auto* bar = addToolBar("Main");
    bar->setMovable(false);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->setIconSize(QSize(18, 18));
    auto add = [&](const QString& text, Glyph glyph, const char* color,
                   void (MainWindow::*slot)()) {
        auto* a = bar->addAction(DrawGlyph(glyph, QColor(color)), text);
        connect(a, &QAction::triggered, this, slot);
    };
    add("Open", Glyph::Folder, "#38bdf8", &MainWindow::openCodeplug);
    add("Save", Glyph::Document, "#94a3b8", &MainWindow::saveCodeplug);
    bar->addSeparator();
    add("Read Radio", Glyph::Radio, "#34d399", &MainWindow::readRadio);
    add("Write Radio", Glyph::Radio, "#fb923c", &MainWindow::writeRadio);
}

void MainWindow::setupUi() {
    auto* central = new QWidget;
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* treePanel = new QWidget;
    treePanel->setObjectName("treePanel");
    treePanel->setFixedWidth(300);
    auto* tl = new QVBoxLayout(treePanel);
    tl->setContentsMargins(14, 18, 14, 12);
    tl->setSpacing(8);

    auto* brandRow = new QHBoxLayout;
    brandRow->setSpacing(11);
    auto* brandMark = new QLabel("OD");
    brandMark->setObjectName("brandMark");
    brandMark->setAlignment(Qt::AlignCenter);
    brandMark->setFixedSize(36, 36);
    auto* brandText = new QVBoxLayout;
    brandText->setSpacing(0);
    auto* brand = new QLabel("OpenDepot");
    brand->setObjectName("brand");
    auto* brandSub = new QLabel("ASTRO 25 WORKSPACE");
    brandSub->setObjectName("brandSub");
    brandText->addWidget(brand);
    brandText->addWidget(brandSub);
    brandRow->addWidget(brandMark);
    brandRow->addLayout(brandText, 1);
    tl->addLayout(brandRow);

    auto* divider = new QFrame;
    divider->setObjectName("sidebarDivider");
    divider->setFrameShape(QFrame::HLine);
    tl->addWidget(divider);

    auto* heading = new QLabel("Codeplug");
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
    pages_->addWidget(buildRadioWidePage());
    pages_->addWidget(buildFieldCatalogPage());
    pages_->addWidget(buildCpsTablesPage());
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
    tree_->setIconSize(QSize(18, 18));

    treeRoot_ = new QTreeWidgetItem(tree_, QStringList{"No codeplug"});
    treeRoot_->setIcon(0, TreeIcon("Radio", false));
    treeRoot_->setData(0, kRolePage, kInfoPage);

    auto* tools = new QTreeWidgetItem(treeRoot_, QStringList{"Tools"});
    tools->setIcon(0, TreeIcon("Tools", true));
    tools->setData(0, kRolePage, -1);
    struct ToolItem {
        const char* name;
        int page;
    };
    const ToolItem toolItems[] = {{"Service Report", kReportPage},
                                  {"Codeplug Doctor", kDoctorPage},
                                  {"Diff", kDiffPage},
                                  {"Radio", kRadioPage},
                                  {"CPS Tables", kCpsTablesPage},
                                  {"Field Catalog", kCatalogPage}};
    for (const auto& t : toolItems) {
        auto* it = new QTreeWidgetItem(tools, QStringList{QString::fromLatin1(t.name)});
        it->setIcon(0, TreeIcon(QString::fromLatin1(t.name), false));
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
            gi->setIcon(0, TreeIcon(group, true));
            gi->setData(0, kRolePage, -1);
            groups.insert(group, gi);
        }
        QString label = QString::fromStdString(s.name);
        if (s.records.size() != 1)
            label += QString("  (%1)").arg(static_cast<int>(s.records.size()));
        auto* it = new QTreeWidgetItem(gi, QStringList{label});
        it->setIcon(0, TreeIcon(label, false));

        int page = kSectionPage;
        if (s.id == "radio_information") {
            page = kInfoPage;
        } else if (s.id == "radio_wide") {
            page = kRadioWidePage;
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

    tools->setData(0, kRolePage, -1);
    struct ToolItem {
        const char* name;
        int page;
    };
    const ToolItem toolItems[] = {{"Service Report", kReportPage},
                                  {"Codeplug Doctor", kDoctorPage},
                                  {"Diff", kDiffPage},
                                  {"Radio", kRadioPage},
                                  {"CPS Tables", kCpsTablesPage},
                                  {"Field Catalog", kCatalogPage}};
    for (const auto& t : toolItems) {
        auto* it = new QTreeWidgetItem(tools, QStringList{QString::fromLatin1(t.name)});
        it->setIcon(0, TreeIcon(QString::fromLatin1(t.name), false));
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

QWidget* MainWindow::buildRadioWidePage() {
    auto* page = new QWidget;
    rwBound_.clear();
    rwTabLayouts_.clear();
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(10);
    auto* title = new QLabel("Radio Wide");
    title->setObjectName("pageTitle");
    lay->addWidget(title);
    auto* banner = new QLabel(
        "Radio Wide fields decoded from codeplug records 0x000A and 0x000C. General, Emergency, "
        "Alert Tones, Time-Out Timers, Alarm, and Home Mode controls are live. Use \"Named Fields\" "
        "for every named field or \"Raw Radio Wide records\" for bytes.");
    banner->setObjectName("muted");
    banner->setWordWrap(true);
    lay->addWidget(banner);

    auto* stack = new QStackedWidget;
    auto* tabBar = new QWidget;
    auto* tabGrid = new QGridLayout(tabBar);
    tabGrid->setContentsMargins(0, 0, 0, 6);
    tabGrid->setSpacing(0);
    auto* tabGroup = new QButtonGroup(page);
    tabGroup->setExclusive(true);
    int nextPage = 0;
    auto addPage = [&](const QString& name, QWidget* w, int row, int col) {
        stack->addWidget(w);
        auto* btn = new QPushButton(name);
        btn->setObjectName("cpsTab");
        btn->setCheckable(true);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tabGroup->addButton(btn);
        const int idx = nextPage++;
        connect(btn, &QPushButton::clicked, this, [stack, idx] { stack->setCurrentIndex(idx); });
        tabGrid->addWidget(btn, row, col);
        return btn;
    };

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* cl = new QVBoxLayout(content);
    cl->setContentsMargins(16, 14, 16, 16);
    cl->setSpacing(12);

    auto* flags = new QGroupBox("General");
    auto* fl = new QGridLayout(flags);
    fl->setHorizontalSpacing(18);
    fl->setVerticalSpacing(8);
    auto bindField = [&](const char* name, QWidget* widget) {
        rwBound_.insert(QString::fromLatin1(name), widget);
        widget->setEnabled(true);
    };
    auto addCheck = [&](const char* text, int row, int col) {
        auto* cb = new QCheckBox(QString::fromLatin1(text));
        fl->addWidget(cb, row, col);
        bindField(text, cb);
        return cb;
    };
    addCheck("Radio Cloning", 0, 0);
    addCheck("Block Pending CA/PC", 0, 1);
    addCheck("Selectable Keypad Mute", 1, 0);
    addCheck("Power Button", 1, 1);
    addCheck("Last Selected Channel Per Zone Enable", 2, 0);
    addCheck("Preset Zone and Channel Enable", 3, 0);

    fl->addWidget(new QLabel("Maximum Channels (Talkgroups):"), 4, 0);
    rwMaxChannels_ = new QSpinBox;
    rwMaxChannels_->setRange(0, 65535);
    rwMaxChannels_->setMaximumWidth(120);
    connect(rwMaxChannels_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &MainWindow::setMaxChannels);
    fl->addWidget(rwMaxChannels_, 4, 1);

    auto addCombo = [&](const char* label, const char* field, int row) {
        fl->addWidget(new QLabel(QString::fromLatin1(label)), row, 0);
        auto* cb = new QComboBox;
        cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        fl->addWidget(cb, row, 1);
        bindField(field, cb);
        return cb;
    };
    addCombo("Out of Range Indicator:", "Out of Range Indicator", 5);
    addCombo("Imbalanced Coverage Indicator:", "Imbalanced Coverage Indicator", 6);
    addCombo("System Registration Indicator:", "System Registration Indicator", 7);
    addCombo("Time Format:", "Time format", 8);
    addCombo("Date Format:", "Date Format", 9);
    addCombo("Channel Announcement Priority:", "Channel Announcement Priority", 10);
    auto* chanAnn = new QCheckBox("Channel Announcement");
    fl->addWidget(chanAnn, 11, 0);
    bindField("Channel Announcement", chanAnn);
    cl->addWidget(flags);

    auto* power = new QGroupBox("Power Management");
    auto* pl = new QGridLayout(power);
    pl->addWidget(new QLabel("Wake Up Delay:"), 0, 0);
    auto* wake = new QSpinBox;
    wake->setRange(0, 65535);
    wake->setEnabled(false);
    pl->addWidget(wake, 0, 1);
    pl->addWidget(new QLabel("LCP Echo Request Attempts:"), 1, 0);
    auto* lcp = new QSpinBox;
    lcp->setRange(0, 255);
    lcp->setEnabled(false);
    pl->addWidget(lcp, 1, 1);
    cl->addWidget(power);

    auto* user = new QGroupBox("User Information");
    auto* ul = new QGridLayout(user);
    auto* ars = new QCheckBox("ARS User Login Enable");
    ul->addWidget(ars, 0, 0, 1, 2);
    bindField("ARS User Login Enable", ars);
    ul->addWidget(new QLabel("Soft ID/Username:"), 1, 0);
    auto* soft = new QLineEdit;
    soft->setEnabled(false);
    ul->addWidget(soft, 1, 1);
    ul->addWidget(new QLabel("PIN/Password:"), 2, 0);
    auto* pin = new QLineEdit;
    pin->setEchoMode(QLineEdit::Password);
    ul->addWidget(pin, 2, 1);
    bindField("Protected Zone Password", pin);
    auto* unitEnable = new QCheckBox("User Login Unit ID Enable");
    ul->addWidget(unitEnable, 3, 0, 1, 2);
    bindField("User Login Unit ID Enable", unitEnable);
    ul->addWidget(new QLabel("User Login Unit ID:"), 4, 0);
    auto* unit = new QLineEdit;
    ul->addWidget(unit, 4, 1);
    bindField("User Login Unit ID", unit);
    ul->addWidget(new QLabel("Cached Credential User Login Mode:"), 5, 0);
    auto* cached = new QComboBox;
    ul->addWidget(cached, 5, 1);
    bindField("Cached Credential User Login Mode", cached);
    auto* aliasEnable = new QCheckBox("Radio ID Alias Enable");
    ul->addWidget(aliasEnable, 6, 0, 1, 2);
    bindField("Radio ID Alias Enable", aliasEnable);
    ul->addWidget(new QLabel("Radio ID Alias:"), 7, 0);
    auto* alias = new QLineEdit;
    ul->addWidget(alias, 7, 1);
    bindField("Radio ID Alias", alias);
    cl->addWidget(user);

    auto* owner = new QGroupBox("Owner ID and Write Protect");
    auto* ol = new QGridLayout(owner);
    ol->addWidget(new QLabel("Key Type"), 0, 0);
    auto* keyType = new QComboBox;
    ol->addWidget(keyType, 0, 1);
    bindField("Key Type", keyType);
    ol->addWidget(new QLabel("Owner System ID"), 1, 0);
    auto* ownerId = new QLineEdit;
    ol->addWidget(ownerId, 1, 1);
    bindField("Owner System ID", ownerId);
    auto* disableWp = new QCheckBox("Disable Write Protect");
    ol->addWidget(disableWp, 2, 0, 1, 2);
    bindField("Disable Write Protect", disableWp);
    cl->addWidget(owner);

    cl->addStretch(1);
    scroll->setWidget(content);

    auto* generalButton = addPage("General", scroll, 3, 0);
    auto addTableTab = [&](const QString& name, int row, int col) {
        auto* content = new QWidget;
        auto* wl = new QVBoxLayout(content);
        wl->setContentsMargins(12, 10, 12, 10);
        wl->setSpacing(10);
        auto* scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidget(content);
        rwTabLayouts_.insert(name, wl);
        addPage(name, scroll, row, col);
    };
    const char* generalTabs[] = {"Emergency", "TPS", "Alert Tones", "Time-Out Timers", "Alarm",
                                 "Home Mode"};
    for (int i = 0; i < 6; ++i) {
        if (QString::fromLatin1(generalTabs[i]) == "TPS") {
            addTableTab(QString::fromLatin1(generalTabs[i]), 3, i + 1);
            continue;
        }
        auto* w = new QWidget;
        auto* wl = new QVBoxLayout(w);
        if (QString::fromLatin1(generalTabs[i]) == "Emergency") {
            auto* form = new QGridLayout;
            form->setHorizontalSpacing(18);
            form->setVerticalSpacing(8);
            int row = 0;
            auto addSpin = [&](const char* label, const char* field, int maximum) {
                form->addWidget(new QLabel(QString::fromLatin1(label)), row, 0);
                auto* spin = new QSpinBox;
                spin->setRange(0, maximum);
                form->addWidget(spin, row++, 1);
                bindField(field, spin);
            };
            auto addCheck = [&](const char* field) {
                auto* check = new QCheckBox(QString::fromLatin1(field));
                form->addWidget(check, row++, 0, 1, 2);
                bindField(field, check);
            };
            addCheck("Emergency Call De-key Sidetone");
            addCheck("Emergency Acknowledgement");
            addSpin("Emergency Steering Zone:", "Emergency Steering Zone", 255);
            addSpin("Emergency Steering Channel:", "Emergency Steering Channel", 65535);
            addSpin("Short Keypress Duration for Emergency:", "Short Keypress Duration for Emergency", 15);
            addSpin("Long Keypress Duration for Emergency:", "Long Keypress Duration for Emergency", 255);
            addSpin("Emergency Alarm Display Time:", "Emergency Alarm Receive Display Time", 255);
            addSpin("Emergency Alarm Retry Ignore Time:", "Emergency Alarm Receive Retry Ignore Time",
                    255);
            addSpin("Emergency Alarm Retry Rate:", "Emergency Alarm Retry Rate", 255);
            wl->addLayout(form);
        } else if (QString::fromLatin1(generalTabs[i]) == "Alert Tones") {
            auto* form = new QGridLayout;
            form->setHorizontalSpacing(18);
            form->setVerticalSpacing(8);
            int row = 0;
            auto addSpin = [&](const char* label, const char* field, int maximum) {
                form->addWidget(new QLabel(QString::fromLatin1(label)), row, 0);
                auto* spin = new QSpinBox;
                spin->setRange(0, maximum);
                form->addWidget(spin, row++, 1);
                bindField(field, spin);
            };
            auto addCheck = [&](const char* field) {
                auto* check = new QCheckBox(QString::fromLatin1(field));
                form->addWidget(check, row++, 0, 1, 2);
                bindField(field, check);
            };
            addCheck("Alert Tones");
            addCheck("Defeat All Tones When Muted");
            addCheck("Fixed Alert Tone Enable");
            addCheck("Call Alert Tone Auto Reset");
            addSpin("Minimum Alert Tone Volume:", "Minimum Alert Tone Volume", 255);
            addSpin("Maximum Alert Tone Volume:", "Maximum Alert Tone Volume", 255);
            addSpin("Alert Tone Volume Offset:", "Alert Tone Volume Offset", 255);
            addSpin("Fixed Alert Tone Volume Setting:", "Fixed Alert Tone Volume Setting", 255);
            wl->addLayout(form);
        } else if (QString::fromLatin1(generalTabs[i]) == "Alarm") {
            auto* form = new QGridLayout;
            form->setHorizontalSpacing(18);
            form->setVerticalSpacing(8);
            int row = 0;
            auto addCheck = [&](const char* field) {
                auto* check = new QCheckBox(QString::fromLatin1(field));
                form->addWidget(check, row++, 0, 1, 2);
                bindField(field, check);
            };
            auto addSpin = [&](const char* label, const char* field, int maximum) {
                form->addWidget(new QLabel(QString::fromLatin1(label)), row, 0);
                auto* spin = new QSpinBox;
                spin->setRange(0, maximum);
                form->addWidget(spin, row++, 1);
                bindField(field, spin);
            };
            auto addCombo = [&](const char* label, const char* field) {
                form->addWidget(new QLabel(QString::fromLatin1(label)), row, 0);
                auto* combo = new QComboBox;
                form->addWidget(combo, row++, 1);
                bindField(field, combo);
            };
            addCheck("Silent Alarm");
            addCheck("Emergency Call De-key Sidetone");
            addCombo("Emergency Alarm Indicator:", "Emergency Alarm Receive Indicator Type");
            addCombo("Emergency Alert Tone Set:", "Emergency Rx/Tx Alert Tone Set");
            addSpin("Audible Emergency Beacon:", "Audible Emergency Beacon", 255);
            addSpin("Emergency Alarm Display Time:", "Emergency Alarm Receive Display Time", 255);
            addSpin("Emergency Alarm Retry Ignore Time:", "Emergency Alarm Receive Retry Ignore Time",
                    255);
            addSpin("Emergency Alarm Retry Rate:", "Emergency Alarm Retry Rate", 255);
            wl->addLayout(form);
        } else if (QString::fromLatin1(generalTabs[i]) == "Home Mode") {
            auto* form = new QGridLayout;
            form->setHorizontalSpacing(18);
            form->setVerticalSpacing(8);
            auto* enabled = new QCheckBox("Home Mode Selection");
            form->addWidget(enabled, 0, 0, 1, 2);
            bindField("Home Mode Selection", enabled);
            form->addWidget(new QLabel("Home Mode Zone:"), 1, 0);
            auto* zone = new QSpinBox;
            zone->setRange(0, 255);
            form->addWidget(zone, 1, 1);
            bindField("Home Mode Zone", zone);
            form->addWidget(new QLabel("Home Mode Channel:"), 2, 0);
            auto* channel = new QSpinBox;
            channel->setRange(0, 65535);
            form->addWidget(channel, 2, 1);
            bindField("Home Mode Channel", channel);
            auto* powerLast = new QCheckBox("Power Up On Last Selected Zone and Channel");
            form->addWidget(powerLast, 3, 0, 1, 2);
            bindField("Power Up On Last Selected Zone and Channel", powerLast);
            wl->addLayout(form);
        } else if (QString::fromLatin1(generalTabs[i]) == "Time-Out Timers") {
            auto* form = new QGridLayout;
            form->setHorizontalSpacing(18);
            form->setVerticalSpacing(8);
            for (int row = 1; row <= 4; ++row) {
                form->addWidget(new QLabel(QString("Radio Wide Time Out Timer Row %1 Time:").arg(row)),
                                row - 1, 0);
                auto* spin = new QSpinBox;
                spin->setRange(0, 31);
                form->addWidget(spin, row - 1, 1);
                bindField(QString("Radio Wide Time Out Timer Row %1 Time").arg(row).toLatin1().constData(),
                          spin);
            }
            wl->addLayout(form);
        } else {
            auto* lbl = new QLabel("This page is not decoded yet.");
            lbl->setObjectName("muted");
            wl->addWidget(lbl);
        }
        wl->addStretch(1);
        addPage(QString::fromLatin1(generalTabs[i]), w, 3, i + 1);
    }
    const char* rows[3][5] = {
        {"Tx Audio Control Per Mode", "Digital Audio Options", "PA/Siren", "Advanced", nullptr},
        {"Advanced II", "VRS/DVRS", "VRS Private Line", "VRS Advanced", nullptr},
        {"VRS Personality", "Rx Audio Control", "Data", "Labtool", "Control Head"},
    };
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5 && rows[row][col]; ++col) {
            addTableTab(QString::fromLatin1(rows[row][col]), row, col);
        }
    }
    auto* named = new QWidget;
    auto* nl = new QVBoxLayout(named);
    auto* namedHint = new QLabel(
        "Every named Radio Wide field, decoded from record 0x000A / 0x000C using "
        "the Depot field registry.");
    namedHint->setObjectName("muted");
    namedHint->setWordWrap(true);
    nl->addWidget(namedHint);
    rwNamedFields_ = new QTableWidget(0, 6);
    rwNamedFields_->setHorizontalHeaderLabels({"Record", "Field", "Off", "Bits", "MSB", "Value"});
    rwNamedFields_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    rwNamedFields_->verticalHeader()->setVisible(false);
    rwNamedFields_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rwNamedFields_->setShowGrid(false);
    nl->addWidget(rwNamedFields_, 1);
    addPage("Named Fields", named, 4, 0);

    auto* allFields = new QWidget;
    rwAllForm_ = new QWidget;
    rwAllFormLayout_ = new QVBoxLayout(rwAllForm_);
    rwAllFormLayout_->setContentsMargins(12, 10, 12, 10);
    rwAllFormLayout_->setSpacing(10);
    auto* allScroll = new QScrollArea;
    allScroll->setWidgetResizable(true);
    allScroll->setFrameShape(QFrame::NoFrame);
    allScroll->setWidget(rwAllForm_);
    auto* allLay = new QVBoxLayout(allFields);
    allLay->setContentsMargins(0, 0, 0, 0);
    allLay->addWidget(allScroll);
    addPage("All Options", allFields, 4, 1);

    for (int col = 0; col < 7; ++col) tabGrid->setColumnStretch(col, 1);
    stack->setCurrentIndex(0);
    generalButton->setChecked(true);
    lay->addWidget(tabBar, 0);
    lay->addWidget(stack, 1);

    auto* raw = new QPushButton("Raw Radio Wide records");
    connect(raw, &QPushButton::clicked, this, [this] {
        if (!model_) return;
        const auto& sections = model_->Sections();
        for (size_t i = 0; i < sections.size(); ++i) {
            if (sections[i].id == "radio_wide") {
                showSection(static_cast<int>(i));
                pages_->setCurrentIndex(kSectionPage);
                return;
            }
        }
    });
    lay->addWidget(raw, 0, Qt::AlignLeft);
    return page;
}

void MainWindow::refreshRadioWidePage() {
    if (!rwMaxChannels_) return;
    int value = 0;
    const auto* r = FindRecord(cp_, 0x000A, 0);
    bool haveRecord = false;
    if (r) {
        auto p = RecordPayload(*r);
        if (p.size() >= 10) {
            value = p[8] | (p[9] << 8);
            haveRecord = true;
        }
    }
    const QSignalBlocker block(rwMaxChannels_);
    rwMaxChannels_->setValue(value);
    rwMaxChannels_->setEnabled(haveRecord);

    size_t allFieldCount = 0;
    const auto* allFields = opendepot::FieldCatalog(allFieldCount);
    for (auto it = rwBound_.cbegin(); it != rwBound_.cend(); ++it) {
        const opendepot::FieldDef* field = nullptr;
        for (size_t i = 0; i < allFieldCount; ++i) {
            const auto& candidate = allFields[i];
            const QString group = QString::fromLatin1(candidate.group ? candidate.group : "");
            if (group != "0xf1a8450" && group != "0xf1a7a30") continue;
            if (it.key() == QString::fromLatin1(candidate.name ? candidate.name : "")) {
                field = &candidate;
                break;
            }
        }
        if (!field || field->bits == 0) continue;
        const uint16_t tag = QString::fromLatin1(field->group) == "0xf1a8450" ? 0x000A : 0x000C;
        const auto* record = FindRecord(cp_, tag, 0);
        if (!record) continue;
        const auto payload = RecordPayload(*record);
        if (size_t(field->off) + size_t((field->bits + 7) / 8) > payload.size()) continue;
        const uint64_t raw = opendepot::ReadFieldBits(payload.data(), payload.size(), field->off,
                                                      field->bits, field->msb);
        QWidget* widget = it.value();
        if (auto* cb = qobject_cast<QCheckBox*>(widget)) {
            cb->setChecked(raw != 0);
            cb->setEnabled(true);
        } else if (auto* combo = qobject_cast<QComboBox*>(widget)) {
            if (combo->count() == 0) {
                const QString key = it.key();
                QStringList options;
                if (key == "Out of Range Indicator" || key == "Imbalanced Coverage Indicator" ||
                    key == "System Registration Indicator") {
                    options = {"No Indication", "Alert Only", "Display Only", "Alert & Display"};
                } else if (key == "Time format") {
                    options = {"12 Hour", "24 Hour", "12 Hour with Seconds",
                               "24 Hour with Seconds"};
                } else if (key == "Date Format") {
                    options = {"MM/DD/YYYY", "DD/MM/YYYY", "YYYY/MM/DD", "MM/DD/YY"};
                } else if (key == "Channel Announcement Priority") {
                    options = {"Normal", "High"};
                } else {
                    const uint64_t values = field->bits >= 5 ? 32 : (uint64_t(1) << field->bits);
                    for (uint64_t i = 0; i < values; ++i) options << QString::number(i);
                }
                combo->addItems(options);
            }
            const int index = static_cast<int>(raw);
            while (combo->count() <= index) combo->addItem(QString::number(combo->count()));
            combo->setCurrentIndex(index);
            combo->setEnabled(true);
        } else if (auto* edit = qobject_cast<QLineEdit*>(widget)) {
            QString text;
            if (field->bits >= 8) {
                const size_t bytes = size_t(field->bits + 7) / 8;
                std::string s;
                for (size_t i = 0; i < bytes && field->off + i < payload.size(); ++i) {
                    const uint8_t c = payload[field->off + i];
                    if (c == 0) break;
                    s.push_back(char(c));
                }
                text = QString::fromLatin1(s.c_str());
            }
            if (text.isEmpty()) text = QString::number(static_cast<qulonglong>(raw));
            edit->setText(text);
            edit->setEnabled(true);
        } else if (auto* spin = qobject_cast<QSpinBox*>(widget)) {
            spin->setValue(static_cast<int>(raw));
            spin->setEnabled(true);
        }
    }

    for (auto it = rwTabLayouts_.cbegin(); it != rwTabLayouts_.cend(); ++it) {
        QVBoxLayout* layout = it.value();
        if (!layout) continue;
        while (auto* item = layout->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        QStringList terms;
        const QString tab = it.key();
        if (tab.contains("Tx Audio")) {
            terms = {"Tx", "Gain", "Microphone", "Audio", "AGC", "Balance"};
        } else if (tab.contains("Rx Audio")) {
            terms = {"Rx", "Audio", "Volume", "AGC", "Gain"};
        } else if (tab.contains("Digital Audio")) {
            terms = {"Digital", "Audio", "Gain", "AGC", "Balance"};
        } else if (tab.contains("VRS")) {
            terms = {"VRS", "DVRS"};
        } else if (tab == "Data") {
            terms = {"Data", "IP", "TCP", "UDP", "Context", "Packet", "SNMP"};
        } else if (tab == "Control Head") {
            terms = {"Control Head"};
        } else if (tab == "PA/Siren") {
            terms = {"PA", "Siren"};
        } else if (tab == "Labtool") {
            terms = {"Labtool"};
        } else if (tab == "TPS") {
            terms = {"TPS", "Talkgroup", "Priority"};
        } else {
            terms = {"Advanced", "Control", "Timer", "Power", "Audio"};
        }
        auto* box = new QGroupBox(tab + " options");
        auto* grid = new QGridLayout(box);
        grid->setHorizontalSpacing(18);
        grid->setVerticalSpacing(7);
        int row = 0;
        for (size_t i = 0; i < allFieldCount; ++i) {
            const auto& field = allFields[i];
            const QString group = QString::fromLatin1(field.group ? field.group : "");
            if (group != "0xf1a8450" && group != "0xf1a7a30") continue;
            const QString name = QString::fromLatin1(field.name ? field.name : "");
            bool match = false;
            for (const auto& term : terms) {
                if (name.contains(term, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            if (!match || field.bits == 0 || field.bits > 32) continue;
            const uint16_t tag = group == "0xf1a8450" ? 0x000A : 0x000C;
            const auto* record = FindRecord(cp_, tag, 0);
            if (!record) continue;
            const auto payload = RecordPayload(*record);
            if (size_t(field.off) + size_t((field.bits + 7) / 8) > payload.size()) continue;
            const uint64_t raw = opendepot::ReadFieldBits(payload.data(), payload.size(), field.off,
                                                          field.bits, field.msb);
            grid->addWidget(new QLabel(name + ":"), row, 0);
            QWidget* editor = nullptr;
            if (field.bits == 1) {
                auto* check = new QCheckBox;
                check->setChecked(raw != 0);
                editor = check;
                const opendepot::FieldDef fd = field;
                connect(check, &QCheckBox::toggled, this, [this, tag, fd](bool on) {
                    const auto* rec = FindRecord(cp_, tag, 0);
                    if (!rec) return;
                    auto p = RecordPayload(*rec);
                    WriteFieldBits(p, fd.off, fd.bits, fd.msb, on ? 1 : 0);
                    if (!model_->SetRecordPayload(tag, 0, p)) return;
                    refreshAfterEdit();
                    refreshRadioWidePage();
                });
            } else if (field.bits <= 16) {
                auto* spin = new QSpinBox;
                spin->setRange(0, (1 << field.bits) - 1);
                spin->setValue(static_cast<int>(raw));
                editor = spin;
                const opendepot::FieldDef fd = field;
                connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                        [this, tag, fd](int next) {
                            const auto* rec = FindRecord(cp_, tag, 0);
                            if (!rec) return;
                            auto p = RecordPayload(*rec);
                            WriteFieldBits(p, fd.off, fd.bits, fd.msb, uint64_t(next));
                            if (!model_->SetRecordPayload(tag, 0, p)) return;
                            refreshAfterEdit();
                            refreshRadioWidePage();
                        });
            } else {
                auto* edit = new QLineEdit(QString("0x%1").arg(static_cast<qulonglong>(raw), 0, 16));
                edit->setReadOnly(true);
                editor = edit;
            }
            grid->addWidget(editor, row, 1);
            grid->setColumnStretch(1, 1);
            ++row;
        }
        if (row == 0) {
            auto* none = new QLabel("No decoded fields in this tab yet.");
            none->setObjectName("muted");
            layout->addWidget(none);
        } else {
            layout->addWidget(box);
        }
        layout->addStretch(1);
    }

    if (!rwNamedFields_) return;
    rwNamedFields_->setRowCount(0);
    size_t fieldCount = 0;
    const auto* fields = opendepot::FieldCatalog(fieldCount);
    for (size_t i = 0; i < fieldCount; ++i) {
        const auto& field = fields[i];
        uint16_t tag = 0;
        if (std::string(field.group ? field.group : "") == "0xf1a8450") {
            tag = 0x000A;
        } else if (std::string(field.group ? field.group : "") == "0xf1a7a30") {
            tag = 0x000C;
        } else {
            continue;
        }
        const auto* record = FindRecord(cp_, tag, 0);
        if (!record) continue;
        const auto payload = RecordPayload(*record);
        const int row = rwNamedFields_->rowCount();
        rwNamedFields_->insertRow(row);
        rwNamedFields_->setItem(row, 0,
                                new QTableWidgetItem(QString("0x%1").arg(tag, 4, 16, QChar('0'))));
        rwNamedFields_->setItem(
            row, 1, new QTableWidgetItem(QString::fromUtf8(field.name ? field.name : "")));
        rwNamedFields_->setItem(row, 2, new QTableWidgetItem(QString::number(field.off)));
        rwNamedFields_->setItem(row, 3, new QTableWidgetItem(QString::number(field.bits)));
        rwNamedFields_->setItem(row, 4, new QTableWidgetItem(QString::number(field.msb)));
        QString valueText = "-";
        if (field.bits > 0) {
            const uint64_t bits =
                opendepot::ReadFieldBits(payload.data(), payload.size(), field.off, field.bits,
                                         field.msb);
            const QString label = EnumText(field.name ? field.name : "", bits);
            valueText = !label.isEmpty()
                            ? label
                            : field.bits > 32
                                  ? QString("0x%1").arg(static_cast<qulonglong>(bits), 0, 16)
                                  : QString::number(static_cast<qulonglong>(bits));
        }
        rwNamedFields_->setItem(row, 5, new QTableWidgetItem(valueText));
    }

    if (rwAllFormLayout_) {
        while (auto* item = rwAllFormLayout_->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        for (uint16_t tag : {uint16_t(0x000A), uint16_t(0x000C)}) {
            const auto* record = FindRecord(cp_, tag, 0);
            if (!record) continue;
            const auto payload = RecordPayload(*record);
            auto* box = new QGroupBox(QString("Record 0x%1").arg(tag, 4, 16, QChar('0')));
            auto* grid = new QGridLayout(box);
            grid->setHorizontalSpacing(18);
            grid->setVerticalSpacing(7);
            int row = 0;
            for (size_t i = 0; i < allFieldCount; ++i) {
                const auto& field = allFields[i];
                if (!field.name || !field.recordTag || field.bits == 0 || field.bits > 32) continue;
                const QString fTag = QString::fromLatin1(field.recordTag).toUpper();
                if (fTag != QString("0x%1").arg(tag, 4, 16, QChar('0')).toUpper()) continue;
                const QString name = QString::fromUtf8(field.name);
                grid->addWidget(new QLabel(name + ":"), row, 0);
                const uint64_t value = opendepot::ReadFieldBits(
                    payload.data(), payload.size(), field.off, field.bits, field.msb);
                QWidget* editor = nullptr;
                if (field.bits == 1) {
                    auto* check = new QCheckBox;
                    check->setChecked(value != 0);
                    editor = check;
                    const opendepot::FieldDef fd = field;
                    connect(check, &QCheckBox::toggled, this, [this, tag, fd](bool on) {
                        const auto* rec = FindRecord(cp_, tag, 0);
                        if (!rec) return;
                        auto p = RecordPayload(*rec);
                        WriteFieldBits(p, fd.off, fd.bits, fd.msb, on ? 1 : 0);
                        if (!model_->SetRecordPayload(tag, 0, p)) return;
                        refreshAfterEdit();
                        refreshRadioWidePage();
                    });
                } else if (field.bits <= 16) {
                    auto* spin = new QSpinBox;
                    spin->setRange(0, (1 << field.bits) - 1);
                    spin->setValue(static_cast<int>(value));
                    editor = spin;
                    const opendepot::FieldDef fd = field;
                    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                            [this, tag, fd](int next) {
                                const auto* rec = FindRecord(cp_, tag, 0);
                                if (!rec) return;
                                auto p = RecordPayload(*rec);
                                WriteFieldBits(p, fd.off, fd.bits, fd.msb, uint64_t(next));
                                if (!model_->SetRecordPayload(tag, 0, p)) return;
                                refreshAfterEdit();
                                refreshRadioWidePage();
                            });
                } else {
                    auto* text = new QLineEdit(
                        QString("0x%1").arg(static_cast<qulonglong>(value), 0, 16));
                    text->setReadOnly(true);
                    editor = text;
                }
                grid->addWidget(editor, row, 1);
                grid->setColumnStretch(1, 1);
                ++row;
            }
            rwAllFormLayout_->addWidget(box);
        }
        rwAllFormLayout_->addStretch(1);
    }
}

void MainWindow::setMaxChannels(int value) {
    if (!model_) return;
    const auto* r = FindRecord(cp_, 0x000A, 0);
    if (!r) return;
    auto p = RecordPayload(*r);
    if (p.size() < 10) return;
    p[8] = uint8_t(value & 0xFF);
    p[9] = uint8_t((value >> 8) & 0xFF);
    if (!model_->SetRecordPayload(0x000A, 0, p)) return;
    refreshAfterEdit();
    statusBar()->showMessage("Maximum Channels updated in memory - File > Save to keep it.", 8000);
}

QWidget* MainWindow::buildFieldCatalogPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(12);

    auto* title = new QLabel("Field Catalog");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    size_t fieldCount = 0;
    opendepot::FieldCatalog(fieldCount);
    size_t tableCount = 0;
    const auto* tables = opendepot::TableCatalog(tableCount);
    size_t columnCount = 0;
    size_t boundCount = 0;
    for (size_t i = 0; i < tableCount; ++i) {
        columnCount += tables[i].columnCount;
        boundCount += tables[i].boundColumnCount;
    }
    auto* subtitle = new QLabel(
        QString("%1 fields · %2 tables · %3/%4 columns bound")
            .arg(static_cast<qulonglong>(fieldCount))
            .arg(static_cast<qulonglong>(tableCount))
            .arg(static_cast<qulonglong>(boundCount))
            .arg(static_cast<qulonglong>(columnCount)));
    subtitle->setObjectName("muted");
    lay->addWidget(subtitle);

    catalogTables_ = new QTableWidget(0, 4);
    catalogTables_->setHorizontalHeaderLabels({"Table", "Core section", "Columns", "Bound"});
    catalogTables_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    catalogTables_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    catalogTables_->verticalHeader()->setVisible(false);
    catalogTables_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    catalogTables_->setSelectionBehavior(QAbstractItemView::SelectRows);
    catalogTables_->setSelectionMode(QAbstractItemView::SingleSelection);
    catalogTables_->setShowGrid(false);
    catalogTables_->setMaximumHeight(240);

    catalogColumns_ = new QTableWidget(0, 9);
    catalogColumns_->setHorizontalHeaderLabels(
        {"#", "Column", "Match", "Field", "Name", "Group", "Off", "Bits", "MSB"});
    catalogColumns_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    catalogColumns_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    catalogColumns_->verticalHeader()->setVisible(false);
    catalogColumns_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    catalogColumns_->setSelectionBehavior(QAbstractItemView::SelectRows);
    catalogColumns_->setShowGrid(false);

    for (size_t i = 0; i < tableCount; ++i) {
        const auto& table = tables[i];
        const int row = catalogTables_->rowCount();
        catalogTables_->insertRow(row);
        catalogTables_->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(table.title)));
        catalogTables_->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(
            table.coreSection ? table.coreSection : "-")));
        catalogTables_->setItem(row, 2,
                                new QTableWidgetItem(QString::number(table.columnCount)));
        catalogTables_->setItem(row, 3,
                                new QTableWidgetItem(QString::number(table.boundColumnCount)));
    }
    connect(catalogTables_, &QTableWidget::itemSelectionChanged, this, [this] {
        fillFieldCatalog(catalogTables_->currentRow());
    });

    lay->addWidget(new QLabel("Tables"));
    lay->addWidget(catalogTables_);
    lay->addWidget(new QLabel("Columns and field bindings"));
    lay->addWidget(catalogColumns_, 1);
    if (catalogTables_->rowCount() > 0) {
        catalogTables_->selectRow(0);
        fillFieldCatalog(0);
    }
    return page;
}

void MainWindow::fillFieldCatalog(int tableIndex) {
    if (!catalogColumns_ || tableIndex < 0) return;
    size_t tableCount = 0;
    const auto* tables = opendepot::TableCatalog(tableCount);
    if (size_t(tableIndex) >= tableCount) return;
    const auto& table = tables[tableIndex];
    catalogColumns_->setRowCount(0);
    for (size_t c = 0; c < table.columnCount; ++c) {
        const auto& column = table.columns[c];
        if (column.bindingCount == 0) {
            const int row = catalogColumns_->rowCount();
            catalogColumns_->insertRow(row);
            catalogColumns_->setItem(row, 0, new QTableWidgetItem(QString::number(c)));
            catalogColumns_->setItem(row, 1,
                                     new QTableWidgetItem(QString::fromUtf8(column.name)));
            catalogColumns_->setItem(row, 2,
                                     new QTableWidgetItem(QString::fromUtf8(column.match)));
            for (int col = 3; col < 9; ++col)
                catalogColumns_->setItem(row, col, new QTableWidgetItem("-"));
            continue;
        }
        for (size_t b = 0; b < column.bindingCount; ++b) {
            const auto& bind = column.bindings[b];
            const auto* field = opendepot::FindField(bind.fid);
            const int row = catalogColumns_->rowCount();
            catalogColumns_->insertRow(row);
            catalogColumns_->setItem(row, 0, new QTableWidgetItem(QString::number(c)));
            catalogColumns_->setItem(row, 1,
                                     new QTableWidgetItem(QString::fromUtf8(column.name)));
            catalogColumns_->setItem(row, 2,
                                     new QTableWidgetItem(QString::fromUtf8(column.match)));
            catalogColumns_->setItem(row, 3, new QTableWidgetItem(QString::number(bind.fid)));
            catalogColumns_->setItem(
                row, 4, new QTableWidgetItem(QString::fromUtf8(
                    field && field->name ? field->name : "")));
            catalogColumns_->setItem(row, 5,
                                     new QTableWidgetItem(QString::fromUtf8(bind.group)));
            catalogColumns_->setItem(row, 6, new QTableWidgetItem(QString::number(bind.off)));
            catalogColumns_->setItem(row, 7, new QTableWidgetItem(QString::number(bind.bits)));
            catalogColumns_->setItem(row, 8, new QTableWidgetItem(QString::number(bind.msb)));
        }
    }
}

QWidget* MainWindow::buildCpsTablesPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    lay->setContentsMargins(32, 26, 32, 32);
    lay->setSpacing(12);

    auto* title = new QLabel("CPS Tables");
    title->setObjectName("pageTitle");
    lay->addWidget(title);
    auto* hint = new QLabel(
        "CPS-style table pages. Zone Channel Assignment and alias tables read the "
        "decoded codeplug model; tables without a row decoder show their columns "
        "and field bindings.");
    hint->setObjectName("muted");
    hint->setWordWrap(true);
    lay->addWidget(hint);

    auto* split = new QHBoxLayout;
    cpsTableList_ = new QTableWidget(0, 1);
    cpsTableList_->setHorizontalHeaderLabels({"Table"});
    cpsTableList_->horizontalHeader()->setStretchLastSection(true);
    cpsTableList_->verticalHeader()->setVisible(false);
    cpsTableList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cpsTableList_->setSelectionBehavior(QAbstractItemView::SelectRows);
    cpsTableList_->setMaximumWidth(280);
    size_t tableCount = 0;
    const auto* tables = opendepot::TableCatalog(tableCount);
    for (size_t i = 0; i < tableCount; ++i) {
        const int row = cpsTableList_->rowCount();
        cpsTableList_->insertRow(row);
        cpsTableList_->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(tables[i].title)));
    }
    split->addWidget(cpsTableList_, 0);

    auto* right = new QVBoxLayout;
    cpsTableInfo_ = new QLabel;
    cpsTableInfo_->setObjectName("muted");
    cpsTableInfo_->setWordWrap(true);
    right->addWidget(cpsTableInfo_);
    cpsGrid_ = new QTableWidget;
    cpsGrid_->verticalHeader()->setVisible(false);
    cpsGrid_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cpsGrid_->setShowGrid(false);
    right->addWidget(cpsGrid_, 1);
    split->addLayout(right, 1);
    lay->addLayout(split, 1);

    connect(cpsTableList_, &QTableWidget::itemSelectionChanged, this, [this] {
        fillCpsTable(cpsTableList_->currentRow());
    });
    if (cpsTableList_->rowCount() > 0) {
        int defaultRow = 0;
        for (size_t i = 0; i < tableCount; ++i) {
            if (QString::fromUtf8(tables[i].key) == "zone_channel_assignment") {
                defaultRow = int(i);
                break;
            }
        }
        cpsTableList_->selectRow(defaultRow);
        fillCpsTable(defaultRow);
    }
    return page;
}

void MainWindow::fillCpsTable(int tableIndex) {
    if (!cpsGrid_ || tableIndex < 0) return;
    size_t tableCount = 0;
    const auto* tables = opendepot::TableCatalog(tableCount);
    if (size_t(tableIndex) >= tableCount) return;
    const auto& table = tables[tableIndex];

    cpsGrid_->clear();
    cpsGrid_->setRowCount(0);
    cpsGrid_->setColumnCount(int(table.columnCount));
    QStringList headers;
    for (size_t c = 0; c < table.columnCount; ++c)
        headers << QString::fromUtf8(table.columns[c].name);
    cpsGrid_->setHorizontalHeaderLabels(headers);
    cpsGrid_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    cpsGrid_->horizontalHeader()->setStretchLastSection(false);

    QString info = QString("%1 - %2 columns, %3 bound")
                       .arg(QString::fromUtf8(table.title))
                       .arg(table.columnCount)
                       .arg(table.boundColumnCount);
    const QString key = QString::fromUtf8(table.key);
    if (key == "zone_channel_assignment" && model_) {
        int rows = 0;
        for (const auto& zone : model_->Zones()) {
            for (const auto& channel : zone.channels) {
                const int row = cpsGrid_->rowCount();
                cpsGrid_->insertRow(row);
                cpsGrid_->setVerticalHeaderItem(
                    row, new QTableWidgetItem(QString::fromStdString(zone.name)));
                for (size_t c = 0; c < table.columnCount; ++c) {
                    const QString name = QString::fromUtf8(table.columns[c].name).trimmed();
                    QString value;
                    bool assigned = false;
                    if (name == "Channel Name") {
                        value = QString::fromStdString(channel.name);
                        assigned = true;
                    } else if (name == "Receive Frequency") {
                        value = QString::number(channel.rxMHz, 'f', 5);
                        assigned = true;
                    } else if (name == "Transmit Frequency") {
                        value = QString::number(channel.txMHz, 'f', 5);
                        assigned = true;
                    } else if (name == "TA Frequency") {
                        value = QString::number(channel.txMHz, 'f', 5);
                        assigned = true;
                    } else if (name == "Personality #") {
                        value = QString::number(channel.channelId);
                        assigned = true;
                    } else if (name == "Receive Squelch Type") {
                        value = QString::number(channel.squelchRx);
                        assigned = true;
                    } else if (name == "Transmit Squelch Type") {
                        value = QString::number(channel.squelchTx);
                        assigned = true;
                    } else if (name == "Personality Type") {
                        value = channel.hasAstro ? "ASTRO" : "Conventional";
                        assigned = true;
                    }
                    const auto& column = table.columns[c];
                    if (!assigned && column.bindingCount > 0) {
                        const auto& bind = column.bindings[0];
                        const uint8_t* bytes = nullptr;
                        size_t length = 0;
                        const QString group = QString::fromUtf8(bind.group);
                        if (group == "0x40aa9d8" && channel.personalitySize > 0) {
                            bytes = channel.personality.data();
                            length = size_t(channel.personalitySize);
                        } else if (group == "0xf1a7ac0" || group == "0xf1a77f0") {
                            bytes = channel.entry.data();
                            length = channel.entry.size();
                        }
                        if (bytes && bind.bits > 0 &&
                            size_t(bind.off) + size_t((bind.bits + 7) / 8) <= length) {
                            const uint64_t raw = opendepot::ReadFieldBits(
                                bytes, length, bind.off, bind.bits, bind.msb);
                            if (name.contains("Frequency", Qt::CaseInsensitive) &&
                                bind.bits == 32) {
                                value = QString::number(
                                    opendepot::WordToMHz(static_cast<uint32_t>(raw)), 'f', 5);
                            } else if (bind.bits > 32) {
                                value = QString("0x%1").arg(
                                    static_cast<qulonglong>(raw), 0, 16);
                            } else {
                                value = EnumText(name, raw);
                                if (value.isEmpty())
                                    value = QString::number(static_cast<qulonglong>(raw));
                            }
                        }
                    }
                    cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
                }
                ++rows;
            }
        }
        info += QString(" - %1 channel row(s)").arg(rows);
    } else if (key == "trunking_message_alias" || key == "trunking_status_alias") {
        const uint16_t tag = key == "trunking_message_alias" ? 0x005B : 0x0059;
        const opendepot::AliasListModel* list = nullptr;
        if (model_) {
            for (const auto& candidate : model_->AliasLists()) {
                if (candidate.tag == tag) {
                    list = &candidate;
                    break;
                }
            }
        }
        if (list) {
            for (size_t i = 0; i < list->entries.size(); ++i) {
                const int row = cpsGrid_->rowCount();
                cpsGrid_->insertRow(row);
                for (size_t c = 0; c < table.columnCount; ++c) {
                    const QString name = QString::fromUtf8(table.columns[c].name).trimmed();
                    QString value;
                    if (name.contains("Number", Qt::CaseInsensitive)) value = QString::number(i + 1);
                    if (name.contains("Text", Qt::CaseInsensitive))
                        value = QString::fromStdString(list->entries[i]);
                    cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
                }
            }
            info += QString(" - %1 alias row(s)").arg(list->entries.size());
        }
    } else if (key == "radio_configuration_features") {
        if (have_) {
            cpsGrid_->insertRow(0);
            const QString flashcode = QString::fromStdString(opendepot::GetFlashcode(cp_));
            for (size_t c = 0; c < table.columnCount; ++c)
                cpsGrid_->setItem(0, int(c), new QTableWidgetItem(flashcode));
            info += " - current FLASHcode";
        }
    } else if (key == "narrow_band_list") {
        const auto* node = findNode(0x00290000u);
        if (node) {
            for (size_t base = 0; base + 8 <= node->payload.size(); base += 8) {
                const int row = cpsGrid_->rowCount();
                cpsGrid_->insertRow(row);
                for (size_t c = 0; c < table.columnCount; ++c) {
                    const auto& column = table.columns[c];
                    if (column.bindingCount == 0) continue;
                    const auto& bind = column.bindings[0];
                    const uint64_t raw = opendepot::ReadFieldBits(
                        node->payload.data(), node->payload.size(),
                        static_cast<uint16_t>(base + bind.off), bind.bits, bind.msb);
                    const QString name = QString::fromUtf8(column.name);
                    QString value;
                    if (name.contains("Frequency", Qt::CaseInsensitive) && bind.bits == 32)
                        value = QString::number(opendepot::WordToMHz(uint32_t(raw)), 'f', 5);
                    else {
                        value = EnumText(name, raw);
                        if (value.isEmpty())
                            value = QString::number(static_cast<qulonglong>(raw));
                    }
                    cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
                }
            }
            info += QString(" - %1 range(s)").arg(cpsGrid_->rowCount());
        }
    } else if (key == "nat_list") {
        const auto* node = findNode(0x00280000u);
        if (node) {
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                const QString name = QString::fromUtf8(column.name);
                if (name == "Static NAT IP Address" && node->payload.size() >= 8) {
                    cpsGrid_->setItem(
                        row, int(c),
                        new QTableWidgetItem(QString("%1.%2.%3.%4")
                                                 .arg(node->payload[4])
                                                 .arg(node->payload[5])
                                                 .arg(node->payload[6])
                                                 .arg(node->payload[7])));
                } else if (column.bindingCount > 0) {
                    const auto& bind = column.bindings[0];
                    const uint64_t raw = opendepot::ReadFieldBits(
                        node->payload.data(), node->payload.size(), bind.off, bind.bits, bind.msb);
                    QString value = EnumText(name, raw);
                    if (value.isEmpty())
                        value = QString::number(static_cast<qulonglong>(raw));
                    cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
                }
            }
            info += " - 1 NAT row";
        }
    } else if (key == "trunking_site_alias" || key == "trunking_astro_site_alias") {
        const auto* node = findNode(0x003F0000u);
        if (node) {
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                if (column.bindingCount == 0) continue;
                const auto& bind = column.bindings[0];
                const uint64_t raw = opendepot::ReadFieldBits(
                    node->payload.data(), node->payload.size(), bind.off, bind.bits, bind.msb);
                const QString name = QString::fromUtf8(column.name);
                QString value;
                if (name.contains("Text", Qt::CaseInsensitive) ||
                    name.contains("Txt", Qt::CaseInsensitive)) {
                    std::string text;
                    for (size_t i = bind.off; i < node->payload.size() && node->payload[i];
                         ++i)
                        text.push_back(char(node->payload[i]));
                    value = QString::fromStdString(text);
                } else {
                    value = EnumText(name, raw);
                    if (value.isEmpty())
                        value = QString::number(static_cast<qulonglong>(raw));
                }
                cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
            }
            info += " - 1 alias row";
        }
    } else if (key == "trunking_call_list" || key == "astro_call_list" ||
               key == "astro_talkgroup_list") {
        QList<uint32_t> ids;
        if (key == "trunking_call_list") ids = {0x0FD10000u, 0x0FD20000u, 0x0FD40000u};
        if (key == "astro_call_list") ids = {0x0FAF0000u, 0x0FB00000u};
        if (key == "astro_talkgroup_list") ids = {0x0FD70000u, 0x0FDA0000u};
        for (uint32_t id : ids) {
            const auto* node = findNode(id);
            if (!node) continue;
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                if (column.bindingCount == 0) continue;
                const auto& bind = column.bindings[0];
                if (size_t(bind.off) + size_t((bind.bits + 7) / 8) > node->payload.size())
                    continue;
                const uint64_t raw = opendepot::ReadFieldBits(
                    node->payload.data(), node->payload.size(), bind.off, bind.bits, bind.msb);
                const QString name = QString::fromUtf8(column.name);
                QString value;
                if (bind.bits > 32) {
                    value = QString("0x%1").arg(static_cast<qulonglong>(raw), 0, 16);
                } else {
                    value = EnumText(name, raw);
                    if (value.isEmpty())
                        value = QString::number(static_cast<qulonglong>(raw));
                }
                cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
            }
        }
        info += QString(" - %1 row(s)").arg(cpsGrid_->rowCount());
    } else if (key == "trunking_personality_talkgroups") {
        for (const auto& node : model_->RecordNodes()) {
            if (node.tag != 0x0FD5) continue;
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                if (column.bindingCount == 0) continue;
                const auto& bind = column.bindings[0];
                if (size_t(bind.off) + size_t((bind.bits + 7) / 8) > node.payload.size())
                    continue;
                const uint64_t raw = opendepot::ReadFieldBits(
                    node.payload.data(), node.payload.size(), bind.off, bind.bits, bind.msb);
                const QString name = QString::fromUtf8(column.name);
                QString value;
                if (name.contains("Frequency", Qt::CaseInsensitive) && bind.bits == 32)
                    value = QString::number(opendepot::WordToMHz(uint32_t(raw)), 'f', 5);
                else {
                    value = EnumText(name, raw);
                    if (value.isEmpty())
                        value = QString::number(static_cast<qulonglong>(raw));
                }
                cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
            }
        }
        info += QString(" - %1 talkgroup row(s)").arg(cpsGrid_->rowCount());
    } else if (key == "secure_multikey") {
        for (uint16_t tag : {uint16_t(0x0034), uint16_t(0x0057)}) {
            const auto* node = FindRecord(cp_, tag, 0);
            if (!node) continue;
            const auto payload = RecordPayload(*node);
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                if (column.bindingCount == 0) continue;
                const auto& bind = column.bindings[0];
                if (size_t(bind.off) + size_t((bind.bits + 7) / 8) > payload.size())
                    continue;
                const uint64_t raw = opendepot::ReadFieldBits(
                    payload.data(), payload.size(), bind.off, bind.bits, bind.msb);
                cpsGrid_->setItem(row, int(c),
                                  new QTableWidgetItem(QString::number(
                                      static_cast<qulonglong>(raw))));
            }
        }
        info += QString(" - %1 secure row(s)").arg(cpsGrid_->rowCount());
    } else if (model_ && table.coreSection) {
        const QString section = QString::fromUtf8(table.coreSection);
        int rows = 0;
        for (const auto& node : model_->RecordNodes()) {
            if (QString::fromStdString(node.section) != section) continue;
            const auto payload = node.payload;
            const int row = cpsGrid_->rowCount();
            cpsGrid_->insertRow(row);
            cpsGrid_->setVerticalHeaderItem(
                row, new QTableWidgetItem(QString("0x%1/%2")
                                              .arg(node.tag, 4, 16, QChar('0'))
                                              .arg(node.inst)
                                              .toUpper()));
            for (size_t c = 0; c < table.columnCount; ++c) {
                const auto& column = table.columns[c];
                if (column.bindingCount == 0) continue;
                const auto& bind = column.bindings[0];
                if (bind.bits == 0) continue;
                const size_t need = size_t(bind.off) + size_t((bind.bits + 7) / 8);
                if (need > payload.size()) continue;
                const uint64_t raw = opendepot::ReadFieldBits(
                    payload.data(), payload.size(), bind.off, bind.bits, bind.msb);
                const QString name = QString::fromUtf8(table.columns[c].name);
                QString value;
                if (name.contains("Frequency", Qt::CaseInsensitive) && bind.bits == 32) {
                    value = QString::number(
                        opendepot::WordToMHz(static_cast<uint32_t>(raw)), 'f', 5);
                } else if (bind.bits > 32) {
                    value = QString("0x%1").arg(static_cast<qulonglong>(raw), 0, 16);
                } else {
                    value = EnumText(name, raw);
                    if (value.isEmpty())
                        value = QString::number(static_cast<qulonglong>(raw));
                }
                cpsGrid_->setItem(row, int(c), new QTableWidgetItem(value));
            }
            ++rows;
            if (rows >= 100) break;
        }
        info += QString(" - %1 raw record row(s)").arg(rows);
    }
    cpsTableInfo_->setText(info);
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

    trunkGroup_ = new QGroupBox("Trunking System");
    auto* tl = new QGridLayout(trunkGroup_);
    tl->addWidget(new QLabel("Home WACN:"), 0, 0);
    trunkWacn_ = new QLineEdit;
    trunkWacn_->setMaximumWidth(150);
    trunkWacn_->setPlaceholderText("hex, e.g. BEEF0");
    tl->addWidget(trunkWacn_, 0, 1);
    tl->addWidget(new QLabel("Home System ID:"), 0, 2);
    trunkHomeId_ = new QLineEdit;
    trunkHomeId_->setMaximumWidth(110);
    trunkHomeId_->setPlaceholderText("hex, e.g. 123");
    tl->addWidget(trunkHomeId_, 0, 3);
    auto* trunkApply = new QPushButton("Apply");
    trunkApply->setObjectName("accent");
    connect(trunkApply, &QPushButton::clicked, this, [this] {
        if (!model_ || !model_->TrunkingSystem().present) return;
        bool ok = false;
        const uint wacn = trunkWacn_->text().trimmed().toUInt(&ok, 16);
        if (!ok || wacn > 0xFFFFFF) {
            QMessageBox::warning(this, "Trunking System", "Home WACN must be 24-bit hex.");
            return;
        }
        const uint home = trunkHomeId_->text().trimmed().toUInt(&ok, 16);
        if (!ok || home > 0xFFFF) {
            QMessageBox::warning(this, "Trunking System", "Home System ID must be 16-bit hex.");
            return;
        }
        if (!model_->SetWacn(wacn) || !model_->SetHomeSystemId(static_cast<uint16_t>(home))) {
            QMessageBox::warning(this, "Trunking System", "The record could not be updated.");
            return;
        }
        refreshAfterEdit();
        showSection(sectionIndex_);
        statusBar()->showMessage("Trunking System updated in memory - File > Save.", 8000);
    });
    tl->addWidget(trunkApply, 0, 4);
    tl->setColumnStretch(5, 1);
    trunkGroup_->setVisible(false);
    lay->addWidget(trunkGroup_);

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
    auto* tabs = new QTabWidget;
    sectionFields_ = new QTableWidget(0, 6);
    sectionFields_->setHorizontalHeaderLabels({"Offset", "Hex", "U8", "U16", "U32", "ASCII"});
    sectionFields_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    sectionFields_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    sectionFields_->verticalHeader()->setVisible(false);
    sectionFields_->setFont(QFont("Consolas", 10));
    sectionFields_->setEditTriggers(QAbstractItemView::DoubleClicked |
                                    QAbstractItemView::EditKeyPressed);
    connect(sectionFields_, &QTableWidget::itemChanged, this, &MainWindow::applyFieldEdit);
    tabs->addTab(sectionFields_, "Fields (plain text)");

    sectionNamed_ = new QTableWidget(0, 6);
    sectionNamed_->setHorizontalHeaderLabels({"Field", "Group", "Off", "Bits", "MSB", "Value"});
    sectionNamed_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    sectionNamed_->verticalHeader()->setVisible(false);
    sectionNamed_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sectionNamed_->setShowGrid(false);
    tabs->addTab(sectionNamed_, "Named fields");
    sectionForm_ = new QWidget;
    sectionFormLayout_ = new QVBoxLayout(sectionForm_);
    sectionFormLayout_->setContentsMargins(12, 10, 12, 10);
    sectionFormLayout_->setSpacing(10);
    auto* formScroll = new QScrollArea;
    formScroll->setWidgetResizable(true);
    formScroll->setFrameShape(QFrame::NoFrame);
    formScroll->setWidget(sectionForm_);
    tabs->addTab(formScroll, "CPS Panel");
    tabs->setCurrentWidget(formScroll);

    auto* hexWrap = new QWidget;
    auto* hexLay = new QVBoxLayout(hexWrap);
    hexLay->setContentsMargins(0, 0, 0, 0);
    hexLay->setSpacing(6);
    sectionHex_ = new QPlainTextEdit;
    sectionHex_->setPlaceholderText("Select a record to edit its payload.");
    sectionHex_->setFont(QFont("Consolas", 10));
    hexLay->addWidget(sectionHex_, 1);
    auto* applyPayload = new QPushButton("Apply payload");
    applyPayload->setObjectName("accent");
    connect(applyPayload, &QPushButton::clicked, this, &MainWindow::applySectionPayload);
    hexLay->addWidget(applyPayload, 0, Qt::AlignLeft);
    tabs->addTab(hexWrap, "Hex");
    payloadCol->addWidget(tabs, 1);
    auto* exportBtn = new QPushButton("Export plain text...");
    exportBtn->setObjectName("small");
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportPlainText);
    payloadCol->addWidget(exportBtn, 0, Qt::AlignLeft);
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
    port_ = new QComboBox;
    port_->setEditable(true);
    port_->setMinimumWidth(130);
    const QStringList ports = AvailableComPorts();
    port_->addItems(ports);
    const QString saved = QSettings().value("radio/port", "COM20").toString();
    if (ports.contains(saved))
        port_->setCurrentText(saved);
    else
        port_->setEditText(saved.isEmpty() ? "COM20" : saved);
    row->addWidget(port_);
    auto* refreshPorts = new QPushButton("Refresh");
    refreshPorts->setObjectName("small");
    connect(refreshPorts, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    row->addWidget(refreshPorts);
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

void MainWindow::createCodeplug() {
    QDialog dlg(this);
    dlg.setWindowTitle("Create Codeplug");
    dlg.setMinimumWidth(460);
    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(22, 20, 22, 16);
    layout->setSpacing(14);

    auto* title = new QLabel("Create a new codeplug");
    title->setObjectName("pageTitle");
    auto* hint = new QLabel(
        "Builds a complete codeplug from OpenDepotCore's built-in model schema, then applies "
        "the identity and FLASHcode.");
    hint->setObjectName("muted");
    hint->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(hint);

    auto* form = new QFormLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(10);

    auto* templateUrl = new QWidget;
    auto* templateRow = new QHBoxLayout(templateUrl);
    templateRow->setContentsMargins(0, 0, 0, 0);
    templateRow->setSpacing(8);
    auto* templateCombo = new QComboBox;
    templateCombo->addItem("Built-in model schema", QString());
    QStringList templateDirs = {
        QApplication::applicationDirPath() + "/templates",
        QApplication::applicationDirPath() + "/../../../radio-backups/templates",
    };
    QStringList seenTemplates;
    for (const auto& dirPath : templateDirs) {
        const QDir dir(dirPath);
        for (const auto& info : dir.entryInfoList({"*.cpg"}, QDir::Files, QDir::Name)) {
            if (seenTemplates.contains(info.absoluteFilePath())) continue;
            seenTemplates << info.absoluteFilePath();
            templateCombo->addItem(info.fileName(), info.absoluteFilePath());
        }
    }
    const QString backupsDir = QApplication::applicationDirPath() + "/../../../radio-backups";
    for (const char* name : {"XTS2500-conv.cpg", "xts2500_trunking_template.cpg",
                             "XTS2500_UHF.cpg", "XTS2500_VHF.cpg"}) {
        const QFileInfo info(QDir(backupsDir).filePath(QString::fromLatin1(name)));
        if (!info.exists() || seenTemplates.contains(info.absoluteFilePath())) continue;
        seenTemplates << info.absoluteFilePath();
        templateCombo->addItem(info.fileName(), info.absoluteFilePath());
    }
    auto* browseTemplate = new QPushButton("Browse...");
    browseTemplate->setObjectName("small");
    templateRow->addWidget(templateCombo, 1);
    templateRow->addWidget(browseTemplate);
    form->addRow("Template", templateUrl);

    QString profileDb;
    for (const char* relative : {"data/depot_profiles.bin", "../../../data/depot_profiles.bin",
                                 "../../data/depot_profiles.bin"}) {
        const QString candidate =
            QDir(QApplication::applicationDirPath()).filePath(QString::fromLatin1(relative));
        if (QFileInfo::exists(candidate)) {
            profileDb = candidate;
            break;
        }
    }
    std::vector<opendepot::DepotProfileInfo> depotProfiles;
    std::string profileErr;
    if (!profileDb.isEmpty())
        opendepot::ListDepotProfiles(profileDb.toStdString(), depotProfiles, profileErr);

    auto* bandEdit = new QComboBox;
    bandEdit->addItems({"800 MHz", "900 MHz", "UHF", "VHF", "700/800 MHz"});
    bandEdit->setCurrentText("800 MHz");
    auto* radioEdit = new QComboBox;
    radioEdit->addItems({"XTS5000", "XTS2500", "XTS1500", "XTS4000", "MT1500", "PR1500",
                         "SSE5000", "XTL5000"});
    radioEdit->setCurrentText("XTS2500");
    auto* typeEdit = new QComboBox;
    typeEdit->addItems({"Model III", "Model II", "Model I"});
    auto* modelEdit = new QComboBox;
    for (const char* model : {"H46UCF9PW6BN", "H46WCF9PW6BN", "H46QDD9PW5BN",
                              "H46KDD9PW5BN", "H18UCC9PW5AN",
                              "H66UCC9PW5AN", "H67UCC9PW5AN", "H10SDD9PW6AN",
                              "H79KDC9PW5AN", "H18KCN9PW9AN", "M20QSS9PW1AN"})
        modelEdit->addItem(QString::fromLatin1(model));
    for (const auto& profile : depotProfiles) {
        const QString model = QString::fromStdString(profile.model);
        if (modelEdit->findText(model) < 0) modelEdit->addItem(model);
    }
    const QString initialModel =
        have_ ? QString::fromStdString(cp_.header.model)
              : QString::fromStdString(opendepot::BuiltinFirstModel());
    if (modelEdit->findText(initialModel) < 0) modelEdit->addItem(initialModel);
    modelEdit->setCurrentText(initialModel);
    auto* serialEdit = new QLineEdit("123ABC4567");
    serialEdit->setMaxLength(16);
    auto* flashEdit = new QLineEdit(
        have_ ? QString::fromStdString(opendepot::GetFlashcode(cp_))
              : QStringLiteral("500018-000488-0"));
    QString baseFlash = flashEdit->text();
    auto syncHardwareModel = [&] {
        const QString band = bandEdit->currentText();
        const QString radio = radioEdit->currentText();
        const QString type = typeEdit->currentText();
        struct Choice {
            const char* band;
            const char* radio;
            const char* type;
            const char* model;
        };
        static const Choice choices[] = {
            {"800 MHz", "XTS2500", "Model III", "H46UCF9PW6BN"},
            {"900 MHz", "XTS2500", "Model III", "H46WCF9PW6BN"},
            {"UHF", "XTS2500", "Model III", "H46QDD9PW5BN"},
            {"VHF", "XTS2500", "Model III", "H46KDD9PW5BN"},
            {"800 MHz", "XTS2500", "Model II", "H46UCF9PW6AN"},
            {"900 MHz", "XTS2500", "Model II", "H46WCF9PW6AN"},
            {"UHF", "XTS2500", "Model II", "H46QDD9PW5AN"},
            {"VHF", "XTS2500", "Model II", "H46KDD9PW5AN"},
            {"800 MHz", "XTS5000", "Model III", "H18UCC9PW5AN"},
            {"UHF", "XTS4000", "Model III", "H18KCN9PW9AN"},
            {"800 MHz", "XTS1500", "Model III", "H66UCC9PW5AN"},
            {"800 MHz", "MT1500", "Model III", "H67UCC9PW5AN"},
            {"800 MHz", "SSE5000", "Model III", "H10SDD9PW6AN"},
            {"800 MHz", "PR1500", "Model III", "H79KDC9PW5AN"},
            {"UHF", "XTL5000", "Model III", "M20QSS9PW1AN"},
        };
        QString model;
        if (radio == "XTS2500") {
            if (band == "900 MHz")
                model = "H46WCF9PW6";
            else if (band == "UHF")
                model = "H46QDD9PW5";
            else if (band == "VHF")
                model = "H46KDD9PW5";
            else
                model = "H46UCF9PW6";
            model += (type == "Model II") ? "AN" : "BN";
        } else if (radio == "XTS5000") {
            if (band == "UHF")
                model = "H18QDH9PW7AN";
            else if (band == "VHF")
                model = "H18KCN9PW9AN";
            else
                model = "H18UCC9PW5AN";
        } else if (radio == "XTS1500") {
            model = "H66UCC9PW5AN";
        } else if (radio == "XTS4000") {
            model = "H18KCN9PW9AN";
        } else if (radio == "MT1500") {
            model = "H67UCC9PW5AN";
        } else if (radio == "PR1500") {
            model = "H79KDC9PW5AN";
        } else if (radio == "SSE5000") {
            model = "H10SDD9PW6AN";
        } else if (radio == "XTL5000") {
            model = "M20QSS9PW1AN";
        }
        {
            const QSignalBlocker block(modelEdit);
            modelEdit->clear();
            for (const auto& choice : choices) {
                if (band == choice.band && radio == choice.radio && type == choice.type &&
                    modelEdit->findText(QString::fromLatin1(choice.model)) < 0)
                    modelEdit->addItem(QString::fromLatin1(choice.model));
            }
            if (!model.isEmpty() && modelEdit->findText(model) >= 0)
                modelEdit->setCurrentText(model);
        }
    };
    auto joinList = [](const std::vector<std::string>& values) {
        QStringList list;
        for (const auto& value : values) list << QString::fromStdString(value);
        return list;
    };
    auto* packagesCombo = new QComboBox;
    packagesCombo->addItem("Use template FLASHcode", QString());
    const std::pair<const char*, const char*> packageCodes[] = {
        {"3600 Analog Trunk Data", "Q443"},
        {"3600 Digital Trunk Data", "Q575"},
        {"9600 Digital Trunk Data", "Q574"},
        {"P25 MDC Conv Data", "Q811"},
        {"Analog Cnv Data", "Q811"},
    };
    for (const auto& package : opendepot::PackagesForModel(
             modelEdit->currentText().toStdString())) {
        packagesCombo->addItem(QString::fromStdString(package));
        for (const auto& mapping : packageCodes)
            if (package == mapping.first)
                packagesCombo->setItemData(packagesCombo->count() - 1,
                                           QString::fromLatin1(mapping.second),
                                           Qt::UserRole);
    }
    for (const auto& profile : depotProfiles) {
        const QString package = QString::fromStdString(profile.package);
        if (packagesCombo->findText(package) < 0) packagesCombo->addItem(package);
    }
    form->addRow("Band", bandEdit);
    form->addRow("Radio", radioEdit);
    form->addRow("Model type", typeEdit);
    form->addRow("Model", modelEdit);
    form->addRow("Serial", serialEdit);
    form->addRow("FLASHcode", flashEdit);
    form->addRow("Packages", packagesCombo);
    layout->addLayout(form);

    connect(browseTemplate, &QPushButton::clicked, &dlg, [&] {
        const QString file = QFileDialog::getOpenFileName(
            &dlg, "Choose template codeplug",
            QApplication::applicationDirPath() + "/../../../radio-backups/templates",
            "Codeplug (*.cpg);;All files (*)");
        if (file.isEmpty()) return;
        int index = templateCombo->findData(file);
        if (index < 0) {
            templateCombo->addItem(QFileInfo(file).fileName(), file);
            index = templateCombo->count() - 1;
        }
        templateCombo->setCurrentIndex(index);
    });

    auto* addonsBox = new QGroupBox("Add-on options");
    auto* addonsLayout = new QVBoxLayout(addonsBox);
    addonsLayout->setContentsMargins(10, 14, 10, 10);
    auto* addonsList = new QListWidget;
    addonsList->setMaximumHeight(150);

    auto checkedAddons = [addonsList] {
        QStringList codes;
        for (int i = 0; i < addonsList->count(); ++i) {
            const auto* item = addonsList->item(i);
            if (item->checkState() == Qt::Checked)
                codes << item->data(Qt::UserRole).toString();
        }
        return codes;
    };
    auto refreshFlashcode = [&] {
        const auto* platform =
            opendepot::FindFlashcodePlatform(opendepot::PlatformForRadio(
                radioEdit->currentText().toStdString()));
        if (!platform) return;
        QString hex = QString::fromStdString(opendepot::FlashcodeToNibbles(
            baseFlash.toStdString()));
        if (hex.size() != 12) return;
        const QString digits = "0123456789ABCDEF";
        auto clearBit = [&](int nibble, int bit) {
            if (nibble < 0 || nibble >= hex.size() || bit < 0 || bit > 3) return;
            const int value = digits.indexOf(hex[nibble]);
            if (value >= 0) hex[nibble] = digits[value & ~(1 << bit)];
        };
        auto setBit = [&](int nibble, int bit) {
            if (nibble < 0 || nibble >= hex.size() || bit < 0 || bit > 3) return;
            const int value = digits.indexOf(hex[nibble]);
            if (value >= 0) hex[nibble] = digits[value | (1 << bit)];
        };
        const auto addonList = opendepot::AddonsForModel(modelEdit->currentText().toStdString());
        for (const auto& option : platform->options) {
            if (std::find(addonList.begin(), addonList.end(), option.code) != addonList.end())
                clearBit(option.nibble, option.bit);
        }
        const QString packageKey = packagesCombo->currentData().toString();
        if (!packageKey.isEmpty()) {
            for (const auto& option : platform->options) {
                for (const auto& package : platform->packages) {
                    if (std::find(package.codes.begin(), package.codes.end(), option.code) !=
                        package.codes.end())
                        clearBit(option.nibble, option.bit);
                }
            }
            for (const auto& package : platform->packages) {
                if (QString::fromStdString(package.key) != packageKey) continue;
                for (const auto& code : package.codes) {
                    for (const auto& option : platform->options) {
                        if (option.code == code) setBit(option.nibble, option.bit);
                    }
                }
            }
        }
        const QStringList addonCodes = checkedAddons();
        for (const auto& option : platform->options) {
            if (addonCodes.contains(QString::fromStdString(option.code)))
                setBit(option.nibble, option.bit);
        }
        const QString canonical = QString::fromStdString(
            opendepot::CanonicalFlashcode(hex.toStdString()));
        if (!canonical.isEmpty()) {
            const QSignalBlocker block(flashEdit);
            flashEdit->setText(canonical);
        }
    };

    auto populateAddons = [&] {
        const QStringList selected = checkedAddons();
        const QSignalBlocker block(addonsList);
        addonsList->clear();
        const auto* platform =
            opendepot::FindFlashcodePlatform(opendepot::PlatformForRadio(
                radioEdit->currentText().toStdString()));
        const auto codes = opendepot::AddonsForModel(modelEdit->currentText().toStdString());
        for (const auto& code : codes) {
            QString label = QString::fromStdString(opendepot::OptionLabel(code));
            if (platform) {
                for (const auto& option : platform->options)
                    if (option.code == code) label = QString::fromStdString(option.label);
            }
            auto* item = new QListWidgetItem(QString("%1 - %2")
                                                 .arg(QString::fromStdString(code), label));
            item->setData(Qt::UserRole, QString::fromStdString(code));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(selected.contains(QString::fromStdString(code))
                                    ? Qt::Checked
                                    : Qt::Unchecked);
            addonsList->addItem(item);
        }
        refreshFlashcode();
    };

    auto syncAddonsFromFlashcode = [&] {
        const auto* platform =
            opendepot::FindFlashcodePlatform(opendepot::PlatformForRadio(
                radioEdit->currentText().toStdString()));
        if (!platform) return;
        QStringList enabled;
        for (const auto& item : opendepot::DecodeFlashcode(
                 platform->name, baseFlash.toStdString())) {
            enabled << QString::fromStdString(item.substr(0, item.find(' ')));
        }
        int packageIndex = 0;
        for (const auto& package : platform->packages) {
            bool all = !package.codes.empty();
            for (const auto& code : package.codes)
                if (!enabled.contains(QString::fromStdString(code))) all = false;
            if (!all) continue;
            const int index = packagesCombo->findData(QString::fromStdString(package.key));
            if (index >= 0) {
                packageIndex = index;
                break;
            }
        }
        {
            const QSignalBlocker blockPackages(packagesCombo);
            packagesCombo->setCurrentIndex(packageIndex);
        }
        const QSignalBlocker block(addonsList);
        for (int i = 0; i < addonsList->count(); ++i) {
            auto* item = addonsList->item(i);
            item->setCheckState(enabled.contains(item->data(Qt::UserRole).toString())
                                    ? Qt::Checked
                                    : Qt::Unchecked);
        }
        refreshFlashcode();
    };

    auto applyProfile = [&] {
        if (profileDb.isEmpty() || packagesCombo->currentText() == "Use template FLASHcode")
            return;
        opendepot::Codeplug profile;
        std::string err;
        if (!opendepot::LoadDepotProfile(profileDb.toStdString(),
                                         modelEdit->currentText().toStdString(),
                                         packagesCombo->currentText().toStdString(), profile,
                                         err))
            return;
        const QSignalBlocker blockModel(modelEdit);
        const QSignalBlocker blockSerial(serialEdit);
        const QSignalBlocker blockFlash(flashEdit);
        modelEdit->setCurrentText(QString::fromStdString(profile.header.model));
        if (serialEdit->text().trimmed().isEmpty() &&
            !profile.header.serial.empty()) {
            serialEdit->setText(QString::fromStdString(profile.header.serial));
        }
        const QString fc = QString::fromStdString(opendepot::GetFlashcode(profile));
        baseFlash = fc;
        flashEdit->setText(fc);
        addonsList->clear();
        populateAddons();
        syncAddonsFromFlashcode();
    };

    connect(addonsList, &QListWidget::itemChanged, &dlg,
            [&](QListWidgetItem*) { refreshFlashcode(); });
    connect(radioEdit, &QComboBox::currentTextChanged, &dlg,
            [&](const QString&) { syncHardwareModel(); populateAddons(); });
    connect(packagesCombo, &QComboBox::currentTextChanged, &dlg,
            [&](const QString&) { refreshFlashcode(); });
    connect(packagesCombo, &QComboBox::currentTextChanged, &dlg,
            [&](const QString&) { applyProfile(); });
    connect(templateCombo, &QComboBox::currentIndexChanged, &dlg, [&](int) {
        const QString path = templateCombo->currentData().toString();
        if (path.isEmpty()) return;
        opendepot::Codeplug t;
        if (!opendepot::LoadCodeplug(std::filesystem::path(path.toStdWString()), t)) return;

        const QSignalBlocker blockModel(modelEdit);
        const QSignalBlocker blockRadio(radioEdit);
        const QSignalBlocker blockBand(bandEdit);
        const QSignalBlocker blockType(typeEdit);
        const QSignalBlocker blockSerial(serialEdit);
        const QSignalBlocker blockFlash(flashEdit);
        const QSignalBlocker blockPackages(packagesCombo);

        const QString model = QString::fromStdString(t.header.model);
        QString radio = radioEdit->currentText();
        QString band = "800 MHz";
        if (model.startsWith("H46")) {
            radio = "XTS2500";
            if (model.contains("WCF")) band = "900 MHz";
            else if (model.contains("QDD")) band = "UHF";
            else if (model.contains("KDD")) band = "VHF";
        } else if (model.startsWith("H18K")) {
            radio = "XTS4000";
        } else if (model.startsWith("H18")) {
            radio = "XTS5000";
        } else if (model.startsWith("H66")) {
            radio = "XTS1500";
        } else if (model.startsWith("H67")) {
            radio = "MT1500";
        } else if (model.startsWith("H10")) {
            radio = "SSE5000";
        } else if (model.startsWith("H79")) {
            radio = "PR1500";
        } else if (model.startsWith("M20")) {
            radio = "XTL5000";
        }
        std::vector<opendepot::ZoneView> zones;
        bool bandFound = false;
        if (opendepot::ParseZones(t, zones)) {
            for (const auto& zone : zones) {
                for (const auto& channel : zone.channels) {
                    const double mhz = opendepot::WordToMHz(channel.rxWord);
                    if (mhz >= 896.0 && mhz < 1000.0) {
                        band = "900 MHz";
                        bandFound = true;
                    } else if (mhz >= 700.0 && mhz < 896.0) {
                        band = "800 MHz";
                        bandFound = true;
                    } else if (mhz >= 300.0 && mhz < 700.0) {
                        band = "UHF";
                        bandFound = true;
                    } else if (mhz >= 100.0 && mhz < 300.0) {
                        band = "VHF";
                        bandFound = true;
                    }
                    if (bandFound) break;
                }
                if (bandFound) break;
            }
        }
        if (modelEdit->findText(model) < 0) modelEdit->addItem(model);
        modelEdit->setCurrentText(model);
        serialEdit->setText(QString::fromStdString(t.header.serial));
        radioEdit->setCurrentText(radio);
        bandEdit->setCurrentText(band);
        typeEdit->setCurrentText("Model III");
        packagesCombo->setCurrentIndex(0);

        const QString fc = QString::fromStdString(opendepot::GetFlashcode(t));
        baseFlash = fc;
        flashEdit->setText(fc);
        addonsList->clear();
        populateAddons();
        syncAddonsFromFlashcode();
    });
    connect(bandEdit, &QComboBox::currentTextChanged, &dlg,
            [&](const QString&) { syncHardwareModel(); });
    connect(typeEdit, &QComboBox::currentTextChanged, &dlg,
            [&](const QString&) { syncHardwareModel(); });
    connect(flashEdit, &QLineEdit::textEdited, &dlg, [&](const QString& text) {
        baseFlash = text;
        refreshFlashcode();
    });
    populateAddons();
    addonsLayout->addWidget(addonsList);
    layout->addWidget(addonsBox);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
    auto* createButton = buttons->addButton("Create...", QDialogButtonBox::AcceptRole);
    createButton->setObjectName("accent");
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(createButton, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted) return;

    const std::string model = modelEdit->currentText().trimmed().toStdString();
    const std::string serial = serialEdit->text().trimmed().toStdString();
    if (model.empty() || serial.empty()) {
        QMessageBox::warning(this, "Create codeplug", "Model and serial are required.");
        return;
    }

    opendepot::Codeplug templ;
    std::string err;
    const QString templatePath = templateCombo->currentData().toString();
    if (!templatePath.isEmpty()) {
        if (!opendepot::LoadCodeplug(std::filesystem::path(templatePath.toStdWString()),
                                     templ)) {
            QMessageBox::critical(this, "Create codeplug",
                                  QString::fromStdString("Cannot load template: " + templ.error));
            return;
        }
    } else if (!profileDb.isEmpty() &&
               packagesCombo->currentText() != "Use template FLASHcode") {
        if (!opendepot::LoadDepotProfile(profileDb.toStdString(), model,
                                         packagesCombo->currentText().toStdString(), templ,
                                         err)) {
            QMessageBox::critical(this, "Create codeplug",
                                  QString::fromStdString("No native profile: " + err));
            return;
        }
    } else if (!opendepot::BuiltinTemplate(model, templ, err)) {
        QMessageBox::critical(this, "Create codeplug",
                              QString::fromStdString("No built-in schema for this model: " + err));
        return;
    }

    opendepot::CreateOptions opt;
    opt.band = bandEdit->currentText().trimmed().toStdString();
    opt.radio = radioEdit->currentText().trimmed().toStdString();
    opt.type = typeEdit->currentText().trimmed().toStdString();
    opt.model = model;
    opt.serial = serial;
    opt.flashcode = flashEdit->text().trimmed().toStdString();
    const QString packageKey = packagesCombo->currentData().toString();
    if (!packageKey.isEmpty())
        opt.packages.push_back(packageKey.toStdString());
    for (int i = 0; i < addonsList->count(); ++i) {
        const auto* item = addonsList->item(i);
        if (item->checkState() == Qt::Checked)
            opt.addons.push_back(item->data(Qt::UserRole).toString().toStdString());
    }
    opendepot::Codeplug created;
    if (!opendepot::CreateCodeplug(templ, opt, created, err)) {
        QMessageBox::critical(this, "Create codeplug",
                              QString::fromStdString("Create failed: " + err));
        return;
    }

    setCodeplug(std::move(created), QString());
    dirty_ = true;
    setWindowTitle("OpenDepot - New Codeplug");
    statusBar()->showMessage("New codeplug created in memory. Use File > Save to write it to disk.",
                             10000);
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
    refreshRadioWidePage();
    if (cpsTableList_) fillCpsTable(cpsTableList_->currentRow());
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
    if (trunkGroup_) {
        const bool show = s.id == "trunking_system" && model_->TrunkingSystem().present;
        trunkGroup_->setVisible(show);
        if (show) {
            trunkWacn_->setText(
                QString("%1")
                    .arg(static_cast<qulonglong>(model_->TrunkingSystem().wacn), 5, 16, QChar('0'))
                    .toUpper());
            trunkHomeId_->setText(
                QString("%1")
                    .arg(static_cast<int>(model_->TrunkingSystem().homeSystemId), 4, 16, QChar('0'))
                    .toUpper());
        }
    }
    refreshSectionList();
}

void MainWindow::refreshSectionList() {
    if (!model_ || !sectionRecords_ || sectionIndex_ < 0 ||
        sectionIndex_ >= static_cast<int>(model_->Sections().size()))
        return;
    {
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
    {
        const QSignalBlocker block(sectionFields_);
        sectionFields_->setRowCount(0);
        const auto& p = n->payload;
        for (size_t off = 0; off < p.size(); ++off) {
            const int rowIdx = sectionFields_->rowCount();
            sectionFields_->insertRow(rowIdx);
            const uint16_t v16 =
                off + 1 < p.size() ? uint16_t((p[off] << 8) | p[off + 1]) : 0;
            const uint32_t v32 = off + 3 < p.size()
                                     ? (uint32_t(p[off]) << 24) | (uint32_t(p[off + 1]) << 16) |
                                           (uint32_t(p[off + 2]) << 8) | p[off + 3]
                                     : 0;
            auto* offItem = new QTableWidgetItem(
                QString("0x%1").arg(static_cast<qulonglong>(off), 4, 16, QChar('0')).toUpper());
            offItem->setData(Qt::UserRole, int(off));
            offItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sectionFields_->setItem(rowIdx, 0, offItem);
            auto* hexItem = new QTableWidgetItem(
                QString("%1").arg(p[off], 2, 16, QChar('0')).toUpper());
            hexItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sectionFields_->setItem(rowIdx, 1, hexItem);
            auto* u8Item = new QTableWidgetItem(QString::number(p[off]));
            u8Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            sectionFields_->setItem(rowIdx, 2, u8Item);
            auto* u16Item = new QTableWidgetItem(off + 1 < p.size() ? QString::number(v16) : "");
            u16Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sectionFields_->setItem(rowIdx, 3, u16Item);
            auto* u32Item = new QTableWidgetItem(off + 3 < p.size() ? QString::number(v32) : "");
            u32Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sectionFields_->setItem(rowIdx, 4, u32Item);
            auto* ascItem = new QTableWidgetItem(
                p[off] >= 0x20 && p[off] < 0x7F ? QString(QChar(p[off])) : QString("."));
            ascItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sectionFields_->setItem(rowIdx, 5, ascItem);
        }
    }
    if (sectionNamed_) {
        sectionNamed_->setRowCount(0);
        const QString recordTag =
            QString("0x%1").arg(static_cast<int>(n->tag), 4, 16, QChar('0')).toUpper();
        size_t fieldCount = 0;
        const auto* fields = opendepot::FieldCatalog(fieldCount);
        auto addField = [&](const opendepot::FieldDef& field, const QString& group) {
            const int rowIdx = sectionNamed_->rowCount();
            sectionNamed_->insertRow(rowIdx);
            sectionNamed_->setItem(
                rowIdx, 0,
                new QTableWidgetItem(QString::fromUtf8(field.name ? field.name : "")));
            sectionNamed_->setItem(rowIdx, 1, new QTableWidgetItem(group));
            sectionNamed_->setItem(rowIdx, 2, new QTableWidgetItem(QString::number(field.off)));
            sectionNamed_->setItem(rowIdx, 3, new QTableWidgetItem(QString::number(field.bits)));
            sectionNamed_->setItem(rowIdx, 4, new QTableWidgetItem(QString::number(field.msb)));
            QString valueText = "-";
            if (field.bits > 0) {
                const uint64_t value =
                    opendepot::ReadFieldBits(n->payload.data(), n->payload.size(), field.off,
                                             field.bits, field.msb);
                const QString label = EnumText(field.name ? field.name : "", value);
                valueText = !label.isEmpty()
                                ? label
                                : field.bits > 32
                                      ? QString("0x%1").arg(static_cast<qulonglong>(value), 0, 16)
                                      : QString::number(static_cast<qulonglong>(value));
            }
            sectionNamed_->setItem(rowIdx, 5, new QTableWidgetItem(valueText));
        };
        for (size_t i = 0; i < fieldCount; ++i) {
            const auto& field = fields[i];
            if (!field.recordTag ||
                QString::fromLatin1(field.recordTag).toUpper() != recordTag)
                continue;
            addField(field, QString::fromUtf8(field.group ? field.group : ""));
        }
        if (sectionNamed_->rowCount() == 0) {
            QHash<QString, int> groupSize;
            QHash<QString, QList<const opendepot::FieldDef*>> candidates;
            for (size_t i = 0; i < fieldCount; ++i) {
                const auto& field = fields[i];
                const QString group = QString::fromUtf8(field.group ? field.group : "");
                const int end = int(field.off) * 8 + int(field.bits);
                if (end > groupSize.value(group)) groupSize[group] = end;
                candidates[group].append(&field);
            }
            for (auto it = groupSize.cbegin(); it != groupSize.cend(); ++it) {
                if ((it.value() + 7) / 8 != int(n->payload.size())) continue;
                for (const auto* field : candidates.value(it.key()))
                    addField(*field, it.key());
            }
        }
        if (sectionNamed_->rowCount() == 0) {
            sectionNamed_->insertRow(0);
            sectionNamed_->setItem(
                0, 0, new QTableWidgetItem("No named field group decoded for this record."));
            sectionNamed_->setSpan(0, 0, 1, 6);
        }
    }

    if (sectionFormLayout_) {
        while (auto* item = sectionFormLayout_->takeAt(0)) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        const QString recordTag =
            QString("0x%1").arg(static_cast<int>(n->tag), 4, 16, QChar('0')).toUpper();
        size_t fieldCount = 0;
        const auto* fields = opendepot::FieldCatalog(fieldCount);
        QHash<QString, QList<const opendepot::FieldDef*>> groups;
        for (size_t i = 0; i < fieldCount; ++i) {
            const auto& field = fields[i];
            if (!field.name || !field.recordTag || field.bits == 0 || field.bits > 32) continue;
            if (QString::fromLatin1(field.recordTag).toUpper() != recordTag) continue;
            groups[recordTag].append(&field);
        }
        if (groups.isEmpty()) {
            QHash<QString, int> groupSize;
            QHash<QString, QList<const opendepot::FieldDef*>> candidates;
            for (size_t i = 0; i < fieldCount; ++i) {
                const auto& field = fields[i];
                if (!field.name || field.bits == 0 || field.bits > 32) continue;
                const QString group = QString::fromUtf8(field.group ? field.group : "");
                const int end = int(field.off) * 8 + int(field.bits);
                if (end > groupSize.value(group)) groupSize[group] = end;
                candidates[group].append(&field);
            }
            for (auto it = groupSize.cbegin(); it != groupSize.cend(); ++it) {
                if ((it.value() + 7) / 8 != int(n->payload.size())) continue;
                groups[recordTag] = candidates.value(it.key());
                break;
            }
        }
        for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
            auto* box = new QGroupBox("Record " + it.key() + " decoded fields");
            auto* grid = new QGridLayout(box);
            grid->setHorizontalSpacing(18);
            grid->setVerticalSpacing(7);
            int row = 0;
            for (const auto* field : it.value()) {
                const QString name = QString::fromUtf8(field->name);
                grid->addWidget(new QLabel(name + ":"), row, 0);
                const uint64_t value =
                    opendepot::ReadFieldBits(n->payload.data(), n->payload.size(), field->off,
                                             field->bits, field->msb);
                QWidget* editor = nullptr;
                if (field->bits == 1) {
                    auto* check = new QCheckBox;
                    check->setChecked(value != 0);
                    editor = check;
                    const auto fd = *field;
                    connect(check, &QCheckBox::toggled, this, [this, fd, editor, row](bool on) {
                        const int recordRow = sectionRecords_->currentRow();
                        auto* id = sectionRecords_->item(recordRow, 0);
                        if (!id) return;
                        const auto* node = findNode(id->data(kRoleRecordId).toUInt());
                        if (!node) return;
                        std::vector<uint8_t> payload = node->payload;
                        WriteFieldBits(payload, fd.off, fd.bits, fd.msb, on ? 1 : 0);
                        if (!model_->SetRecordPayload(node->tag, node->inst, payload)) return;
                        refreshAfterEdit();
                        refreshSectionRecord(recordRow);
                    });
                } else {
                    auto* spin = new QSpinBox;
                    const int maxValue = field->bits >= 31 ? INT_MAX
                                                           : ((1 << field->bits) - 1);
                    spin->setRange(0, maxValue);
                    spin->setValue(static_cast<int>(value));
                    editor = spin;
                    const auto fd = *field;
                    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                            [this, fd](int next) {
                                const int recordRow = sectionRecords_->currentRow();
                                auto* id = sectionRecords_->item(recordRow, 0);
                                if (!id) return;
                                const auto* node = findNode(id->data(kRoleRecordId).toUInt());
                                if (!node) return;
                                std::vector<uint8_t> payload = node->payload;
                                WriteFieldBits(payload, fd.off, fd.bits, fd.msb, uint64_t(next));
                                if (!model_->SetRecordPayload(node->tag, node->inst, payload)) return;
                                refreshAfterEdit();
                                refreshSectionRecord(recordRow);
                            });
                }
                grid->addWidget(editor, row, 1);
                grid->setColumnStretch(1, 1);
                ++row;
            }
            sectionFormLayout_->addWidget(box);
        }
        if (groups.isEmpty()) {
            auto* none = new QLabel("No decoded controls for this record; use Named fields or Hex.");
            none->setObjectName("muted");
            sectionFormLayout_->addWidget(none);
        }
        sectionFormLayout_->addStretch(1);
    }

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

void MainWindow::applyFieldEdit(QTableWidgetItem* item) {
    if (!model_ || !sectionRecords_ || !sectionFields_ || !item) return;
    if (item->column() != 2) return;
    const int recordRow = sectionRecords_->currentRow();
    if (recordRow < 0) return;
    auto* idItem = sectionRecords_->item(recordRow, 0);
    if (!idItem) return;
    const uint32_t id = idItem->data(kRoleRecordId).toUInt();
    const auto* n = findNode(id);
    if (!n) return;
    auto* offItem = sectionFields_->item(item->row(), 0);
    if (!offItem) return;
    const int off = offItem->data(Qt::UserRole).toInt();
    const QString text = item->text().trimmed();
    bool ok = false;
    const int value = text.startsWith("0x", Qt::CaseInsensitive) ? text.mid(2).toInt(&ok, 16)
                                                                 : text.toInt(&ok);
    if (!ok || value < 0 || value > 255 || off < 0 || off >= int(n->payload.size())) {
        QMessageBox::warning(this, "Field", "Byte value must be 0-255 (decimal or 0x..).");
        refreshSectionRecord(recordRow);
        return;
    }
    std::vector<uint8_t> payload = n->payload;
    payload[size_t(off)] = uint8_t(value);
    const uint16_t tag = n->tag;
    const uint16_t inst = n->inst;
    if (!model_->SetRecordPayload(tag, inst, payload)) {
        QMessageBox::warning(this, "Field", "The record could not be updated.");
        return;
    }
    refreshAfterEdit();
    for (int row = 0; row < sectionRecords_->rowCount(); ++row) {
        auto* it = sectionRecords_->item(row, 0);
        if (it && it->data(kRoleRecordId).toUInt() == id) {
            sectionRecords_->selectRow(row);
            break;
        }
    }
    statusBar()->showMessage("Byte updated in memory - File > Save to keep it.", 8000);
}

void MainWindow::exportPlainText() {
    if (!have_ || !model_) {
        QMessageBox::information(this, "Export plain text", "Open a codeplug first.");
        return;
    }
    const QString file = QFileDialog::getSaveFileName(this, "Export plain text", QString(),
                                                      "Text (*.txt)");
    if (file.isEmpty()) return;
    QString out;
    out += "OpenDepot plain-text codeplug dump\n";
    out += QString("Model: %1\nSerial: %2\nFLASHcode: %3\nRecords: %4\n\n")
               .arg(QString::fromStdString(cp_.header.model),
                    QString::fromStdString(cp_.header.serial),
                    QString::fromStdString(opendepot::GetFlashcode(cp_)))
               .arg(model_->RecordCount());
    for (const auto& s : model_->Sections()) {
        out += QString("=== %1 / %2  [%3] ===\n")
                   .arg(QString::fromStdString(s.group), QString::fromStdString(s.name),
                        QString::fromUtf8(opendepot::ConfidenceName(s.confidence)));
        for (uint32_t id : s.records) {
            const auto* n = findNode(id);
            if (!n) continue;
            out += QString("\n0x%1  tag 0x%2  inst %3  len %4  family %5\n")
                       .arg(static_cast<qulonglong>(id), 8, 16, QChar('0'))
                       .arg(static_cast<int>(n->tag), 4, 16, QChar('0'))
                       .arg(static_cast<int>(n->inst))
                       .arg(n->length)
                       .arg(QString::fromStdString(n->family));
            if (!n->name.empty())
                out += "  name: " + QString::fromStdString(n->name) + "\n";
            if (!n->names.empty()) {
                QStringList names;
                for (const auto& x : n->names) names << QString::fromStdString(x);
                out += "  names: " + names.join(" | ") + "\n";
            }
            if (!n->strings.empty()) {
                QStringList ss;
                for (const auto& x : n->strings) ss << QString::fromStdString(x);
                out += "  strings: " + ss.join(" | ") + "\n";
            }
            if (!n->references.empty()) {
                QStringList rs;
                for (const auto& r : n->references)
                    rs << QString("0x%1/%2@%3")
                              .arg(static_cast<int>(r.tag), 4, 16, QChar('0'))
                              .arg(static_cast<int>(r.inst))
                              .arg(r.offset);
                out += "  refs: " + rs.join(", ") + "\n";
            }
            const auto& p = n->payload;
            for (size_t off = 0; off < p.size(); off += 16) {
                out += QString("  %1  ")
                           .arg(static_cast<qulonglong>(off), 4, 16, QChar('0'))
                           .toUpper();
                const size_t count = std::min<size_t>(16, p.size() - off);
                for (size_t i = 0; i < 16; ++i) {
                    if (i < count)
                        out += QString("%1 ").arg(p[off + i], 2, 16, QChar('0')).toUpper();
                    else
                        out += "   ";
                }
                out += " |";
                for (size_t i = 0; i < count; ++i) {
                    const uint8_t c = p[off + i];
                    out += (c >= 0x20 && c < 0x7F) ? QChar(c) : QChar('.');
                }
                out += "|\n";
            }
        }
        out += "\n";
    }
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export plain text", "Cannot write the selected file.");
        return;
    }
    f.write(out.toUtf8());
    f.close();
    statusBar()->showMessage("Exported plain text to " + file, 8000);
}

void MainWindow::refreshPorts() {
    if (!port_) return;
    const QString current = port_->currentText().trimmed();
    const QStringList ports = AvailableComPorts();
    const QSignalBlocker block(port_);
    port_->clear();
    port_->addItems(ports);
    if (!current.isEmpty())
        port_->setEditText(current);
    else if (!ports.isEmpty())
        port_->setCurrentIndex(0);
    else
        port_->setEditText("COM20");
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
    const std::string port = port_ ? port_->currentText().trimmed().toStdString() : "COM20";
    QSettings().setValue("radio/port", QString::fromStdString(port));
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
    const std::string port = port_ ? port_->currentText().trimmed().toStdString() : "COM20";
    QSettings().setValue("radio/port", QString::fromStdString(port));
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
