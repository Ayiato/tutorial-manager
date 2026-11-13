#include "mainwindow.h"
#include "services.h"
#include "exceptions.h"
#include "validator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <algorithm>
#include <cstring>

static const std::string DB_FILE = "tutorials.txt";

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Tutorial Manager");
    setMinimumSize(900, 600);

    loadFromFile(db, DB_FILE);
    if (db.empty()) {
        initRepo(db);
        saveToFile(db, DB_FILE);
    }

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    buildModePage();
    buildAdminPage();
    buildUserPage();

    stack->addWidget(modePage);
    stack->addWidget(adminPage);
    stack->addWidget(userPage);
    stack->setCurrentIndex(0);

    // Watch-list table window (stays hidden until requested)
    wlTableWindow = new WatchListWindow(watchList, this);
}


// ── Formatting ───────────────────────────────────────────────────────────────

QString MainWindow::formatTutorial(const Tutorial& t) const {
    if (displayMode == DisplayMode::Short)
        return QString("%1  —  %2").arg(t.get_title()).arg(t.get_presenter());

    return QString("[%1] %2 | %3 | %4:%5 | Likes:%6 | Comments:%7")
        .arg(t.get_presenter())
        .arg(t.get_title())
        .arg(t.get_link())
        .arg(t.get_minutes())
        .arg(t.get_seconds(), 2, 10, QChar('0'))
        .arg(t.get_likes())
        .arg(t.get_comments());
}


// ── Build pages ───────────────────────────────────────────────────────────────

void MainWindow::buildModePage() {
    modePage = new QWidget;
    auto* outer = new QVBoxLayout(modePage);
    outer->setAlignment(Qt::AlignCenter);
    outer->setSpacing(20);

    auto* title = new QLabel("Tutorial Manager");
    title->setAlignment(Qt::AlignCenter);
    QFont f = title->font();
    f.setPointSize(22);
    f.setBold(true);
    title->setFont(f);
    outer->addWidget(title);

    auto* wlGroup = new QGroupBox("Watch-list storage");
    auto* wlLayout = new QHBoxLayout(wlGroup);
    wlLayout->addWidget(new QLabel("Type:"));
    comboWL = new QComboBox;
    comboWL->addItem("CSV",    0);
    comboWL->addItem("HTML",   1);
    comboWL->addItem("SQLite", 2);
    wlLayout->addWidget(comboWL);
    outer->addWidget(wlGroup);

    auto* dispGroup = new QGroupBox("Display form");
    auto* dispLayout = new QHBoxLayout(dispGroup);
    dispLayout->addWidget(new QLabel("Show items as:"));
    comboDisplay = new QComboBox;
    comboDisplay->addItem("Long form (all details)",        static_cast<int>(DisplayMode::Long));
    comboDisplay->addItem("Short form (title + presenter)", static_cast<int>(DisplayMode::Short));
    dispLayout->addWidget(comboDisplay);
    outer->addWidget(dispGroup);

    auto* btnRow = new QHBoxLayout;
    btnAdmin = new QPushButton("Administrator");
    btnUser  = new QPushButton("User");
    btnAdmin->setFixedHeight(50);
    btnUser->setFixedHeight(50);
    btnRow->addWidget(btnAdmin);
    btnRow->addWidget(btnUser);
    outer->addLayout(btnRow);

    connect(btnAdmin,     &QPushButton::clicked, this, &MainWindow::onEnterAdmin);
    connect(btnUser,      &QPushButton::clicked, this, &MainWindow::onEnterUser);
    connect(comboDisplay, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDisplayModeChanged);
}

void MainWindow::buildAdminPage() {
    adminPage = new QWidget;
    auto* mainLayout = new QVBoxLayout(adminPage);

    auto* header = new QLabel("Administrator Mode");
    QFont hf = header->font(); hf.setBold(true); hf.setPointSize(14);
    header->setFont(hf);
    header->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(header);

    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* listGroup = new QGroupBox("Tutorials");
    auto* listLayout = new QVBoxLayout(listGroup);
    adminList = new QListWidget;
    listLayout->addWidget(adminList);

    auto* filterRow = new QHBoxLayout;
    leMaxComments = new QLineEdit;
    leMaxComments->setPlaceholderText("Max comments…");
    btnFilter = new QPushButton("Filter");
    auto* btnClearFilter = new QPushButton("Show All");
    filterRow->addWidget(leMaxComments);
    filterRow->addWidget(btnFilter);
    filterRow->addWidget(btnClearFilter);
    listLayout->addLayout(filterRow);
    splitter->addWidget(listGroup);

    auto* formGroup = new QGroupBox("Tutorial details");
    auto* formLayout = new QFormLayout(formGroup);
    leTitle     = new QLineEdit; formLayout->addRow("Title:",        leTitle);
    lePresenter = new QLineEdit; formLayout->addRow("Presenter:",    lePresenter);
    leLink      = new QLineEdit; formLayout->addRow("Link:",         leLink);
    leLikes     = new QLineEdit; formLayout->addRow("Likes:",        leLikes);
    leDuration  = new QLineEdit; formLayout->addRow("Duration (s):", leDuration);
    splitter->addWidget(formGroup);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    auto* btnRow = new QHBoxLayout;
    btnAdd       = new QPushButton("Add");
    btnRemove    = new QPushButton("Remove");
    btnUpdate    = new QPushButton("Update");
    btnUndo      = new QPushButton("Undo  (Ctrl+Z)");
    btnRedo      = new QPushButton("Redo  (Ctrl+Y)");
    btnAdminBack = new QPushButton("Back to menu");

    btnUndo->setEnabled(false);
    btnRedo->setEnabled(false);

    btnRow->addWidget(btnAdd);
    btnRow->addWidget(btnRemove);
    btnRow->addWidget(btnUpdate);
    btnRow->addWidget(btnUndo);
    btnRow->addWidget(btnRedo);
    btnRow->addStretch();
    btnRow->addWidget(btnAdminBack);
    mainLayout->addLayout(btnRow);

    refreshAdminList();

    connect(adminList,       &QListWidget::itemSelectionChanged, this, &MainWindow::onAdminListSelectionChanged);
    connect(btnAdd,          &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnRemove,       &QPushButton::clicked, this, &MainWindow::onRemove);
    connect(btnUpdate,       &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(btnUndo,         &QPushButton::clicked, this, &MainWindow::onUndo);
    connect(btnRedo,         &QPushButton::clicked, this, &MainWindow::onRedo);
    connect(btnFilter,       &QPushButton::clicked, this, &MainWindow::onFilter);
    connect(btnClearFilter,  &QPushButton::clicked, this, [this]{ refreshAdminList(); });
    connect(btnAdminBack,    &QPushButton::clicked, this, &MainWindow::onAdminBack);

    // Keyboard shortcuts (active for the whole window)
    auto* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    auto* redoShortcut = new QShortcut(QKeySequence("Ctrl+Y"), this);
    connect(undoShortcut, &QShortcut::activated, this, &MainWindow::onUndo);
    connect(redoShortcut, &QShortcut::activated, this, &MainWindow::onRedo);
}

void MainWindow::buildUserPage() {
    userPage = new QWidget;
    auto* mainLayout = new QVBoxLayout(userPage);

    auto* header = new QLabel("User Mode");
    QFont hf = header->font(); hf.setBold(true); hf.setPointSize(14);
    header->setFont(hf);
    header->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(header);

    auto* filterGroup = new QGroupBox("Browse tutorials");
    auto* fg = new QVBoxLayout(filterGroup);
    auto* presRow = new QHBoxLayout;
    lePresenterFilter  = new QLineEdit;
    lePresenterFilter->setPlaceholderText("Filter by presenter (leave empty = all)");
    btnFilterPresenter = new QPushButton("Apply");
    presRow->addWidget(lePresenterFilter);
    presRow->addWidget(btnFilterPresenter);
    fg->addLayout(presRow);

    userBrowseList = new QListWidget;
    fg->addWidget(userBrowseList);

    btnAddToWatch = new QPushButton("Add selected to Watch List");
    fg->addWidget(btnAddToWatch);
    mainLayout->addWidget(filterGroup);

    auto* wlGroup = new QGroupBox("Watch list");
    auto* wg = new QVBoxLayout(wlGroup);
    userWatchList = new QListWidget;
    wg->addWidget(userWatchList);

    auto* wlBtnRow = new QHBoxLayout;
    btnRemoveWatched = new QPushButton("Remove (mark watched)");
    btnOpenWL        = new QPushButton("Open watch-list file");
    btnViewTable     = new QPushButton("View as Table");
    wlBtnRow->addWidget(btnRemoveWatched);
    wlBtnRow->addWidget(btnOpenWL);
    wlBtnRow->addWidget(btnViewTable);
    wg->addLayout(wlBtnRow);
    mainLayout->addWidget(wlGroup);

    btnUserBack = new QPushButton("Back to menu");
    mainLayout->addWidget(btnUserBack, 0, Qt::AlignLeft);

    refreshUserBrowseList(db);

    connect(btnFilterPresenter, &QPushButton::clicked, this, &MainWindow::onFilterByPresenter);
    connect(btnAddToWatch,      &QPushButton::clicked, this, &MainWindow::onAddToWatchList);
    connect(btnRemoveWatched,   &QPushButton::clicked, this, &MainWindow::onRemoveWatched);
    connect(btnOpenWL,          &QPushButton::clicked, this, &MainWindow::onOpenWatchList);
    connect(btnViewTable,       &QPushButton::clicked, this, &MainWindow::onViewWatchListTable);
    connect(btnUserBack,        &QPushButton::clicked, this, &MainWindow::onUserBack);
}


// ── List helpers ──────────────────────────────────────────────────────────────

void MainWindow::refreshAdminList() {
    adminList->clear();
    for (const Tutorial& t : db)
        adminList->addItem(formatTutorial(t));
}

void MainWindow::refreshAdminList(const DynamicVector<Tutorial>& subset) {
    adminList->clear();
    for (const Tutorial& t : subset)
        adminList->addItem(formatTutorial(t));
}

void MainWindow::refreshUserBrowseList(const DynamicVector<Tutorial>& subset) {
    userBrowseList->clear();
    for (const Tutorial& t : subset)
        userBrowseList->addItem(formatTutorial(t));
}

void MainWindow::refreshWatchList() {
    userWatchList->clear();
    for (const Tutorial& t : watchList)
        userWatchList->addItem(formatTutorial(t));
    if (wlTableWindow) wlTableWindow->refresh();
}

void MainWindow::populateFormFromSelected() {
    int row = adminList->currentRow();
    if (row < 0 || row >= (int)db.getSize()) return;
    const Tutorial& t = db[row];
    leTitle->setText(t.get_title());
    lePresenter->setText(t.get_presenter());
    leLink->setText(t.get_link());
    leLikes->setText(QString::number(t.get_likes()));
    leDuration->setText(QString::number(t.get_minutes() * 60 + t.get_seconds()));
}

void MainWindow::clearForm() {
    leTitle->clear(); lePresenter->clear(); leLink->clear();
    leLikes->clear(); leDuration->clear();
}

Tutorial MainWindow::tutorialFromForm() const {
    Tutorial t;
    char title[25], presenter[50], link[50];
    strncpy(title,     leTitle->text().toStdString().c_str(),     24); title[24] = '\0';
    strncpy(presenter, lePresenter->text().toStdString().c_str(), 49); presenter[49] = '\0';
    strncpy(link,      leLink->text().toStdString().c_str(),      49); link[49] = '\0';
    int likes    = leLikes->text().toInt();
    int duration = leDuration->text().toInt();
    t.createTutorial(title, presenter, link, likes, duration);
    return t;
}

void MainWindow::showError(const QString& msg) { QMessageBox::critical(this, "Error", msg); }
void MainWindow::showInfo(const QString& msg)  { QMessageBox::information(this, "Info", msg); }


// ── Undo / Redo ───────────────────────────────────────────────────────────────

void MainWindow::executeCommand(std::unique_ptr<Command> cmd) {
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
    redoStack_.clear();
    updateUndoRedoButtons();
}

void MainWindow::updateUndoRedoButtons() {
    btnUndo->setEnabled(!undoStack_.empty());
    btnRedo->setEnabled(!redoStack_.empty());
}

void MainWindow::onUndo() {
    if (undoStack_.empty()) return;
    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();
    try {
        cmd->undo();
        saveToFile(db, DB_FILE);
        refreshAdminList();
        redoStack_.push_back(std::move(cmd));
    } catch (const AppException& e) { showError(e.what()); }
    updateUndoRedoButtons();
}

void MainWindow::onRedo() {
    if (redoStack_.empty()) return;
    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();
    try {
        cmd->execute();
        saveToFile(db, DB_FILE);
        refreshAdminList();
        undoStack_.push_back(std::move(cmd));
    } catch (const AppException& e) { showError(e.what()); }
    updateUndoRedoButtons();
}


// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onDisplayModeChanged(int index) {
    displayMode = (index == 0) ? DisplayMode::Long : DisplayMode::Short;
    refreshAdminList();
    onFilterByPresenter();
    refreshWatchList();
}

void MainWindow::onEnterAdmin() {
    refreshAdminList();
    stack->setCurrentIndex(1);
}

void MainWindow::onEnterUser() {
    int type = comboWL->currentData().toInt();
    switch (type) {
        case 0: wlRepo = std::make_unique<CsvWatchListRepository>("watchlist.csv");   break;
        case 1: wlRepo = std::make_unique<HtmlWatchListRepository>("watchlist.html"); break;
        case 2: wlRepo = std::make_unique<SqliteWatchListRepository>("watchlist.db"); break;
    }
    watchList = DynamicVector<Tutorial>();
    refreshUserBrowseList(db);
    refreshWatchList();
    stack->setCurrentIndex(2);
}

void MainWindow::onAdminListSelectionChanged() { populateFormFromSelected(); }

void MainWindow::onAdd() {
    if (leTitle->text().trimmed().isEmpty() || leLink->text().trimmed().isEmpty()) {
        showError("Title and Link are required."); return;
    }
    try {
        Tutorial t = tutorialFromForm();
        executeCommand(std::make_unique<AddCommand>(db, t));
        saveToFile(db, DB_FILE);
        refreshAdminList();
        clearForm();
    } catch (const AppException& e) { showError(e.what()); }
}

void MainWindow::onRemove() {
    int row = adminList->currentRow();
    if (row < 0) { showError("Select a tutorial to remove."); return; }
    try {
        Tutorial removed = db[row];
        executeCommand(std::make_unique<RemoveCommand>(db, removed));
        saveToFile(db, DB_FILE);
        refreshAdminList();
        clearForm();
    } catch (const AppException& e) { showError(e.what()); }
}

void MainWindow::onUpdate() {
    int row = adminList->currentRow();
    if (row < 0) { showError("Select a tutorial to update."); return; }
    if (leTitle->text().trimmed().isEmpty() || leLink->text().trimmed().isEmpty()) {
        showError("Title and Link are required."); return;
    }
    try {
        Tutorial oldT = db[row];
        Tutorial newT = tutorialFromForm();
        newT.set_comments(oldT.get_comments()); // preserve comment count
        TutorialValidator::validate(newT);
        executeCommand(std::make_unique<UpdateCommand>(db, oldT, newT));
        saveToFile(db, DB_FILE);
        refreshAdminList();
    } catch (const AppException& e) { showError(e.what()); }
}

void MainWindow::onFilter() {
    bool ok;
    int maxC = leMaxComments->text().toInt(&ok);
    if (!ok) { showError("Enter a valid integer for max comments."); return; }
    DynamicVector<Tutorial> filtered = filterByComments(db, maxC);
    refreshAdminList(filtered);
}

void MainWindow::onAdminBack() { clearForm(); stack->setCurrentIndex(0); }

void MainWindow::onFilterByPresenter() {
    std::string pres = lePresenterFilter->text().toStdString();
    if (pres.empty()) { refreshUserBrowseList(db); return; }
    DynamicVector<Tutorial> filtered;
    for (const Tutorial& t : db)
        if (std::string(t.get_presenter()).find(pres) != std::string::npos)
            filtered.add(t);
    refreshUserBrowseList(filtered);
}

void MainWindow::onAddToWatchList() {
    int row = userBrowseList->currentRow();
    if (row < 0) { showError("Select a tutorial to add."); return; }
    QString sel = userBrowseList->currentItem()->text();
    for (const Tutorial& t : db) {
        if (formatTutorial(t) == sel) {
            try {
                addToWatchList(watchList, t);
                if (wlRepo) wlRepo->save(watchList);
                refreshWatchList();
                showInfo("Added to watch list!");
            } catch (const AppException& e) { showError(e.what()); }
            return;
        }
    }
}

void MainWindow::onRemoveWatched() {
    int row = userWatchList->currentRow();
    if (row < 0) { showError("Select a tutorial from your watch list."); return; }
    QMessageBox::StandardButton like = QMessageBox::question(
        this, "Like?", "Did you like this tutorial?",
        QMessageBox::Yes | QMessageBox::No);
    try {
        QString sel = userWatchList->currentItem()->text();
        for (const Tutorial& t : watchList) {
            if (formatTutorial(t) == sel) {
                removeFromWatchList(watchList, db, t.get_link(), like == QMessageBox::Yes);
                saveToFile(db, DB_FILE);
                if (wlRepo) wlRepo->save(watchList);
                refreshWatchList();
                return;
            }
        }
    } catch (const AppException& e) { showError(e.what()); }
}

void MainWindow::onOpenWatchList() {
    if (!wlRepo) { showError("No watch-list repository selected."); return; }
    if (watchList.empty()) { showInfo("Your watch list is empty."); return; }
    wlRepo->save(watchList);
    if (wlRepo->typeName() == "SQLite") {
        showInfo("SQLite watch list saved to: " + QString::fromStdString(wlRepo->getFilePath()) +
                 "\nOpen with DB Browser for SQLite.");
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(
            QString::fromStdString(wlRepo->getFilePath())));
    }
}

void MainWindow::onViewWatchListTable() {
    wlTableWindow->refresh();
    wlTableWindow->show();
    wlTableWindow->raise();
    wlTableWindow->activateWindow();
}

void MainWindow::onUserBack() { stack->setCurrentIndex(0); }
