//
// file_repo.h – polymorphic watch-list file repositories
//
// Hierarchy:
//   WatchListRepository  (abstract)
//     ├─ CsvWatchListRepository
//     ├─ HtmlWatchListRepository
//     └─ SqliteWatchListRepository  (bonus)
//
#pragma once
#ifndef UNTITLED_FILE_REPO_H
#define UNTITLED_FILE_REPO_H

#include "domain.h"
#include "repository.h"
#include "exceptions.h"

#include <fstream>
#include <sstream>
#include <string>
#include "sqlite3.h"

using namespace std;
class WatchListRepository {
public:
    virtual ~WatchListRepository() = default;

    /** Persist the entire watch list to the backing store. */
    virtual void save(const DynamicVector<Tutorial>& wl) = 0;

    /** Return the filename / path that can be opened by the OS. */
    virtual string getFilePath() const = 0;

    /** Human-readable type name (for UI messages). */
    virtual string typeName() const = 0;
};

class CsvWatchListRepository : public WatchListRepository {
    string path_;
public:
    explicit CsvWatchListRepository(const string& path = "watchlist.csv")
        : path_(path) {}

    void save(const DynamicVector<Tutorial>& wl) override {
        ofstream f(path_, ios::trunc);
        if (!f.is_open())
            throw RepositoryException("Cannot open CSV file for writing: " + path_);

        for (const Tutorial& t : wl) {
            ostringstream dur;
            dur << t.get_minutes() << ":";
            if (t.get_seconds() < 10) dur << "0";
            dur << t.get_seconds();

            f << t.get_title()     << ","
              << t.get_presenter() << ","
              << dur.str()         << ","
              << t.get_link()      << "\n";
        }
    }

    string getFilePath() const override { return path_; }
    string typeName()    const override { return "CSV"; }
};

class HtmlWatchListRepository : public WatchListRepository {
    string path_;
public:
    explicit HtmlWatchListRepository(const string& path = "watchlist.html")
        : path_(path) {}

    void save(const DynamicVector<Tutorial>& wl) override {
        ofstream f(path_, ios::trunc);
        if (!f.is_open())
            throw RepositoryException("Cannot open HTML file for writing: " + path_);

        f << "<!DOCTYPE html>\n"
          << "<html>\n"
          << "<head>\n"
          << "    <title>Watch List</title>\n"
          << "</head>\n"
          << "<body>\n"
          << "<table border=\"1\">\n"
          << "    <tr>\n"
          << "        <td>Title</td>\n"
          << "        <td>Presenter</td>\n"
          << "        <td>Duration</td>\n"
          << "        <td>Link</td>\n"
          << "    </tr>\n";

        for (const Tutorial& t : wl) {
            ostringstream dur;
            dur << t.get_minutes() << ":";
            if (t.get_seconds() < 10) dur << "0";
            dur << t.get_seconds();

            f << "    <tr>\n"
              << "        <td>" << t.get_title()     << "</td>\n"
              << "        <td>" << t.get_presenter() << "</td>\n"
              << "        <td>" << dur.str()          << "</td>\n"
              << "        <td><a href=\"" << t.get_link() << "\">Link</a></td>\n"
              << "    </tr>\n";
        }

        f << "</table>\n"
          << "</body>\n"
          << "</html>\n";
    }

    string getFilePath() const override { return path_; }
    string typeName()    const override { return "HTML"; }
};

class SqliteWatchListRepository : public WatchListRepository {
    string dbPath_;

    void exec(sqlite3* db, const string& sql) {
        char* err = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
            string msg = err ? err : "unknown sqlite3 error";
            sqlite3_free(err);
            throw RepositoryException("SQLite error: " + msg);
        }
    }

public:
    explicit SqliteWatchListRepository(const string& dbPath = "watchlist.db")
        : dbPath_(dbPath) {}

    void save(const DynamicVector<Tutorial>& wl) override {
        sqlite3* db = nullptr;
        if (sqlite3_open(dbPath_.c_str(), &db) != SQLITE_OK)
            throw RepositoryException("Cannot open SQLite database: " + dbPath_);

        exec(db, "CREATE TABLE IF NOT EXISTS watchlist ("
                 "  title     TEXT,"
                 "  presenter TEXT,"
                 "  link      TEXT PRIMARY KEY,"
                 "  likes     INTEGER,"
                 "  minutes   INTEGER,"
                 "  seconds   INTEGER,"
                 "  comments  INTEGER"
                 ");");
        exec(db, "DELETE FROM watchlist;");

        const string insert =
            "INSERT INTO watchlist (title,presenter,link,likes,minutes,seconds,comments) "
            "VALUES (?,?,?,?,?,?,?);";

        sqlite3_stmt* stmt =  nullptr;
        if (sqlite3_prepare_v2(db, insert.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            throw RepositoryException("SQLite prepare failed");
        }

        for (const Tutorial& t : wl) {
            sqlite3_bind_text(stmt, 1, t.get_title(),     -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, t.get_presenter(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, t.get_link(),      -1, SQLITE_TRANSIENT);
            sqlite3_bind_int (stmt, 4, t.get_likes());
            sqlite3_bind_int (stmt, 5, t.get_minutes());
            sqlite3_bind_int (stmt, 6, t.get_seconds());
            sqlite3_bind_int (stmt, 7, t.get_comments());

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                throw RepositoryException("SQLite insert failed");
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    string getFilePath() const override { return dbPath_; }
    string typeName()    const override { return "SQLite"; }
};

#endif // UNTITLED_FILE_REPO_H