#pragma once
#ifndef COMMANDS_H
#define COMMANDS_H

#include "domain.h"
#include "repository.h"
#include "services.h"
#include <algorithm>
#include <cstring>

// Abstract base – all concrete commands inherit from this
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
};

// ── Add ──────────────────────────────────────────────────────────────────────
class AddCommand : public Command {
    DynamicVector<Tutorial>& db_;
    Tutorial tutorial_;
public:
    AddCommand(DynamicVector<Tutorial>& db, const Tutorial& t)
        : db_(db), tutorial_(t) {}

    void execute() override { addTutorial(db_, tutorial_); }
    void undo()    override { removeTutorial(db_, tutorial_.get_link()); }
};

// ── Remove ───────────────────────────────────────────────────────────────────
class RemoveCommand : public Command {
    DynamicVector<Tutorial>& db_;
    Tutorial tutorial_;
public:
    RemoveCommand(DynamicVector<Tutorial>& db, const Tutorial& t)
        : db_(db), tutorial_(t) {}

    void execute() override { removeTutorial(db_, tutorial_.get_link()); }
    void undo()    override { addTutorial(db_, tutorial_); }
};

// ── Update ───────────────────────────────────────────────────────────────────
class UpdateCommand : public Command {
    DynamicVector<Tutorial>& db_;
    Tutorial oldTutorial_;
    Tutorial newTutorial_;

    void replaceByLink(const char* link, const Tutorial& replacement) {
        auto it = std::find_if(db_.begin(), db_.end(), [link](const Tutorial& t) {
            return std::strcmp(t.get_link(), link) == 0;
        });
        if (it != db_.end()) *it = replacement;
    }

public:
    UpdateCommand(DynamicVector<Tutorial>& db, const Tutorial& oldT, const Tutorial& newT)
        : db_(db), oldTutorial_(oldT), newTutorial_(newT) {}

    void execute() override { replaceByLink(oldTutorial_.get_link(), newTutorial_); }
    void undo()    override { replaceByLink(newTutorial_.get_link(), oldTutorial_); }
};

#endif // COMMANDS_H
