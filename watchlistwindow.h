#pragma once
#ifndef WATCHLISTWINDOW_H
#define WATCHLISTWINDOW_H

#include <QWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include "domain.h"
#include "repository.h"
#include "tutorialtablemodel.h"

class WatchListWindow : public QWidget {
    Q_OBJECT
    TutorialTableModel* model_;
    QTableView*         tableView_;

public:
    explicit WatchListWindow(const DynamicVector<Tutorial>& watchList, QWidget* parent = nullptr)
        : QWidget(parent, Qt::Window)
    {
        setWindowTitle("Watch List – Table View");
        setMinimumSize(900, 400);

        model_     = new TutorialTableModel(watchList, this);
        tableView_ = new QTableView(this);
        tableView_->setModel(model_);
        tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView_->verticalHeader()->setVisible(false);

        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Tutorial watch list:"));
        layout->addWidget(tableView_);
    }

    void refresh() { model_->refresh(); }
};

#endif // WATCHLISTWINDOW_H
