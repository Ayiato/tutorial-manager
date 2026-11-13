#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include "repository.h"
#include "domain.h"
#include "file_repo.h"
#include "commands.h"
#include "watchlistwindow.h"
#include <memory>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    enum class DisplayMode { Long, Short };
    DisplayMode displayMode = DisplayMode::Long;

    DynamicVector<Tutorial> db;
    DynamicVector<Tutorial> watchList;
    std::unique_ptr<WatchListRepository> wlRepo;

    // ── Undo / Redo ───────────────────────────────────────────────────────────
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;

    void executeCommand(std::unique_ptr<Command> cmd);
    void updateUndoRedoButtons();

    WatchListWindow* wlTableWindow = nullptr;

    QStackedWidget* stack;
    QWidget*     modePage;
    QPushButton* btnAdmin;
    QPushButton* btnUser;
    QComboBox*   comboWL;
    QComboBox*   comboDisplay;

    QWidget*     adminPage;
    QListWidget* adminList;
    QLineEdit*   leTitle;
    QLineEdit*   lePresenter;
    QLineEdit*   leLink;
    QLineEdit*   leLikes;
    QLineEdit*   leDuration;
    QPushButton* btnAdd;
    QPushButton* btnRemove;
    QPushButton* btnUpdate;
    QPushButton* btnUndo;
    QPushButton* btnRedo;
    QPushButton* btnAdminBack;

    QLineEdit*   leMaxComments;
    QPushButton* btnFilter;

    // ── User page ────────────────────────────────────────────────────────────
    QWidget*     userPage;
    QListWidget* userBrowseList;
    QListWidget* userWatchList;
    QLineEdit*   lePresenterFilter;
    QPushButton* btnFilterPresenter;
    QPushButton* btnAddToWatch;
    QPushButton* btnRemoveWatched;
    QPushButton* btnOpenWL;
    QPushButton* btnViewTable;
    QPushButton* btnUserBack;

    void buildModePage();
    void buildAdminPage();
    void buildUserPage();

    void refreshAdminList();
    void refreshAdminList(const DynamicVector<Tutorial>& subset);
    void refreshUserBrowseList(const DynamicVector<Tutorial>& subset);
    void refreshWatchList();
    void populateFormFromSelected();
    void clearForm();
    Tutorial tutorialFromForm() const;
    void showError(const QString& msg);
    void showInfo(const QString& msg);
    QString formatTutorial(const Tutorial& t) const;

private slots:
    void onEnterAdmin();
    void onEnterUser();

    void onAdd();
    void onRemove();
    void onUpdate();
    void onUndo();
    void onRedo();
    void onFilter();
    void onAdminBack();
    void onAdminListSelectionChanged();

    void onFilterByPresenter();
    void onAddToWatchList();
    void onRemoveWatched();
    void onOpenWatchList();
    void onViewWatchListTable();
    void onUserBack();

    void onDisplayModeChanged(int index);
};

#endif // MAINWINDOW_H
