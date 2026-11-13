#pragma once
#ifndef TUTORIALTABLEMODEL_H
#define TUTORIALTABLEMODEL_H

#include <QAbstractTableModel>
#include "domain.h"
#include "repository.h"

class TutorialTableModel : public QAbstractTableModel {
    Q_OBJECT
    const DynamicVector<Tutorial>& data_;

public:
    explicit TutorialTableModel(const DynamicVector<Tutorial>& data, QObject* parent = nullptr)
        : QAbstractTableModel(parent), data_(data) {}

    int rowCount(const QModelIndex& parent = {}) const override {
        return parent.isValid() ? 0 : data_.getSize();
    }

    int columnCount(const QModelIndex& parent = {}) const override {
        return parent.isValid() ? 0 : 5;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || role != Qt::DisplayRole) return {};
        const Tutorial& t = data_[index.row()];
        switch (index.column()) {
            case 0: return QString(t.get_title());
            case 1: return QString(t.get_presenter());
            case 2: return QString(t.get_link());
            case 3: return t.get_likes();
            case 4: return QString("%1:%2")
                        .arg(t.get_minutes())
                        .arg(t.get_seconds(), 2, 10, QChar('0'));
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
        static const char* cols[] = {"Title", "Presenter", "Link", "Likes", "Duration"};
        if (section >= 0 && section < 5) return QString(cols[section]);
        return {};
    }

    void refresh() {
        beginResetModel();
        endResetModel();
    }
};

#endif // TUTORIALTABLEMODEL_H
