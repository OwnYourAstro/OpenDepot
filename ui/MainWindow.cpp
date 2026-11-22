// SPDX-FileCopyrightText: 2026 Sarah Rose
// SPDX-License-Identifier: MIT

#include "MainWindow.h"

#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include <filesystem>
#include <vector>

#include "opendepot/core/radio.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("OpenDepot");
    resize(1180, 760);

    setupMenu();

    auto* split = new QSplitter(this);
    nav_ = new QListWidget;
    nav_->addItems({"Dashboard", "Service Report", "Doctor", "Diff", "Radio"});
    nav_->setFixedWidth(190);
    nav_->setObjectName("nav");

    pages_ = new QStackedWidget;
    pages_->addWidget(buildDashboard());
    pages_->addWidget(
        buildTextPage(&report_, "Generate report", &MainWindow::generateReport));
    pages_->addWidget(buildTextPage(&doctor_, "Run doctor", &MainWindow::runDoctor));
    pages_->addWidget(
        buildTextPage(&diff_, "Compare with codeplug...", &MainWindow::compareCodeplug));
    pages_->addWidget(buildRadioPage());

    connect(nav_, &QListWidget::currentRowChanged, pages_, &QStackedWidget::setCurrentIndex);
    split->addWidget(nav_);
    split->addWidget(pages_);
    split->setStretchFactor(1, 1);
    setCentralWidget(split);

    statusBar()->showMessage("Open a codeplug or read a radio.");

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

    auto* tools = menuBar()->addMenu("&Tools");
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
    bar->addAction(open);
    bar->addAction(saveAct_);
    bar->addSeparator();
    bar->addAction(tools->actions().first());
}

QWidget* MainWindow::buildDashboard() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);

    auto* title = new QLabel("Codeplug");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    auto* idBox = new QGroupBox("Identity");
    auto* grid = new QGridLayout(idBox);
    identity_ = new QLabel("No codeplug loaded");
    flashcode_ = new QLabel("-");
    counts_ = new QLabel("-");
    grid->addWidget(new QLabel("Radio"), 0, 0);
    grid->addWidget(identity_, 0, 1);
    grid->addWidget(new QLabel("FLASHcode"), 1, 0);
    grid->addWidget(flashcode_, 1, 1);
    grid->addWidget(new QLabel("Records"), 2, 0);
    grid->addWidget(counts_, 2, 1);
    grid->setColumnStretch(1, 1);
    lay->addWidget(idBox);

    auto* zoneBox = new QGroupBox("Zones and channels");
    auto* zlay = new QVBoxLayout(zoneBox);
    zones_ = new QTableWidget(0, 4);
    zones_->setHorizontalHeaderLabels({"Zone", "Channel", "Rx MHz", "Tx MHz"});
    zones_->horizontalHeader()->setStretchLastSection(true);
    zones_->verticalHeader()->setVisible(false);
    zones_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    zones_->setSelectionBehavior(QAbstractItemView::SelectRows);
    zlay->addWidget(zones_);
    lay->addWidget(zoneBox, 1);
    return page;
}

QWidget* MainWindow::buildTextPage(QPlainTextEdit** out, const QString& button,
                                   void (MainWindow::*handler)()) {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);
    auto* btn = new QPushButton(button);
    connect(btn, &QPushButton::clicked, this, handler);
    *out = new QPlainTextEdit;
    (*out)->setReadOnly(true);
    (*out)->setPlaceholderText("Nothing to show yet.");
    lay->addWidget(btn, 0, Qt::AlignLeft);
    lay->addWidget(*out, 1);
    return page;
}

QWidget* MainWindow::buildRadioPage() {
    auto* page = new QWidget;
    auto* lay = new QVBoxLayout(page);

    auto* title = new QLabel("Radio");
    title->setObjectName("pageTitle");
    lay->addWidget(title);

    auto* box = new QGroupBox("Native transport (clean room)");
    auto* blay = new QVBoxLayout(box);
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Port"));
    port_ = new QLineEdit("COM20");
    port_->setMaximumWidth(140);
    row->addWidget(port_);
    auto* read = new QPushButton("Read Radio");
    auto* write = new QPushButton("Write Radio...");
    read->setObjectName("accent");
    row->addWidget(read);
    row->addWidget(write);
    row->addStretch(1);
    blay->addLayout(row);
    auto* note = new QLabel(
        "Reads and writes use OpenDepot's own serial/PPP/RCMP transport. "
        "A write can brick a radio; keep a verified backup.");
    note->setWordWrap(true);
    note->setObjectName("muted");
    blay->addWidget(note);
    lay->addWidget(box);

    radioLog_ = new QPlainTextEdit;
    radioLog_->setReadOnly(true);
    lay->addWidget(radioLog_, 1);

    connect(read, &QPushButton::clicked, this, &MainWindow::readRadio);
    connect(write, &QPushButton::clicked, this, &MainWindow::writeRadio);
    return page;
}

void MainWindow::openCodeplug() {
    const QString start = path_.isEmpty() ? QString() : path_;
    const QString file = QFileDialog::getOpenFileName(this, "Open codeplug", start,
                                                      "Codeplug (*.cpg);;All files (*)");
    if (file.isEmpty()) return;
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
    statusBar()->showMessage("Saved " + file, 8000);
}

void MainWindow::setCodeplug(opendepot::Codeplug cp, const QString& path) {
    cp_ = std::move(cp);
    have_ = true;
    path_ = path;
    saveAct_->setEnabled(true);

    identity_->setText(QString("%1 / %2")
                           .arg(QString::fromStdString(cp_.header.model),
                                QString::fromStdString(cp_.header.serial)));
    flashcode_->setText(QString::fromStdString(opendepot::GetFlashcode(cp_)));
    size_t good = 0;
    for (const auto& r : cp_.records)
        if (r.verified) ++good;
    counts_->setText(QString("%1 records, %2 CRC-ok")
                         .arg(static_cast<qulonglong>(cp_.records.size()))
                         .arg(static_cast<qulonglong>(good)));

    std::vector<opendepot::ZoneView> zones;
    opendepot::ParseZones(cp_, zones);
    zones_->setRowCount(0);
    for (const auto& z : zones) {
        if (z.channels.empty()) {
            const int row = zones_->rowCount();
            zones_->insertRow(row);
            zones_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(z.name)));
            zones_->setItem(row, 1, new QTableWidgetItem("-"));
            zones_->setItem(row, 2, new QTableWidgetItem(""));
            zones_->setItem(row, 3, new QTableWidgetItem(""));
            continue;
        }
        for (const auto& ch : z.channels) {
            const int row = zones_->rowCount();
            zones_->insertRow(row);
            zones_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(z.name)));
            zones_->setItem(row, 1,
                            new QTableWidgetItem(QString::fromStdString(ch.name)));
            zones_->setItem(row, 2,
                            new QTableWidgetItem(QString::number(
                                opendepot::WordToMHz(ch.rxWord), 'f', 5)));
            zones_->setItem(row, 3,
                            new QTableWidgetItem(QString::number(
                                opendepot::WordToMHz(ch.txWord), 'f', 5)));
        }
    }
    zones_->resizeColumnsToContents();
    statusBar()->showMessage("Loaded " + path, 8000);
}

void MainWindow::refreshViews() {
    if (!have_) return;
    setCodeplug(cp_, path_);
}

void MainWindow::generateReport() {
    if (!have_) {
        QMessageBox::information(this, "Service report", "Open a codeplug first.");
        return;
    }
    report_->setPlainText(
        QString::fromStdString(opendepot::ServiceReportText(cp_, std::string())));
    nav_->setCurrentRow(1);
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
    nav_->setCurrentRow(2);
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
    diff_->setPlainText(
        QString::fromStdString(opendepot::DiffCodeplugsText(cp_, other)));
    nav_->setCurrentRow(3);
}

void MainWindow::readRadio() {
    if (job_) return;
    auto job = std::make_unique<RadioJob>();
    job->isWrite = false;
    RadioJob* j = job.get();
    opendepot::Codeplug base = cp_;
    const std::string port = port_->text().trimmed().toStdString();
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
    const auto answer = QMessageBox::warning(
        this, "Write to radio",
        QString("Write %1 records to %2?\n\nThis can brick the radio. Keep a verified backup.")
            .arg(static_cast<qulonglong>(cp_.records.size()))
            .arg(port_->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    auto job = std::make_unique<RadioJob>();
    job->isWrite = true;
    RadioJob* j = job.get();
    opendepot::Codeplug base = cp_;
    const std::string port = port_->text().trimmed().toStdString();
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
        nav_->setCurrentRow(0);
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
