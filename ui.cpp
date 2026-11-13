//
// ui.cpp – Week 9 edition
//
// Changes vs Week 8:
//   • All service calls are now wrapped in try/catch; errors display user-friendly messages.
//   • At startup the user chooses CSV / HTML / SQLite for the watch list file (Req 4 + bonus).
//   • User menu has a new option to open the watch list file with the OS default app (Req 5).
//   • InputException is thrown and caught for invalid console input.
//

#include "ui.h"
#include "repository.h"
#include "domain.h"
#include "services.h"
#include "exceptions.h"
#include "validator.h"
#include "file_repo.h"

#include <limits>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <memory>

using namespace std;

static const string DB_FILE = "tutorials.txt";


static int readInteger(const string& prompt) {
    int value;
    while (true) {
        if (!prompt.empty()) cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input! Please enter a whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}


static void displayTutorial(const Tutorial& t) {
    cout << "Title: "        << t.get_title()
         << " | Presenter: " << t.get_presenter()
         << " | Link: "      << t.get_link()
         << " | Likes: "     << t.get_likes()
         << " | Duration: "  << t.get_minutes() << ":";
    if (t.get_seconds() < 10) cout << "0";
    cout << t.get_seconds()
         << " | Comments: "  << t.get_comments() << "\n";
}

static void displayTutorials(const DynamicVector<Tutorial>& vec) {
    if (vec.empty()) { cout << "\nThere are no tutorials!\n"; return; }
    for (const Tutorial& t : vec) displayTutorial(t);
}

static void openWithOS(const string& path) {
#ifdef _WIN32
    string cmd = "start \"\" \"" + path + "\"";
#elif defined(__APPLE__)
    string cmd = "open \"" + path + "\"";
#else
    string cmd = "xdg-open \"" + path + "\"";
#endif
    cout << "Opening: " << path << "\n";
    system(cmd.c_str());
}

static void openInBrowser(const char* link) {
    openWithOS(link);
}


/**
 * Asks the user to pick a watch-list file type.
 * Returns a heap-allocated WatchListRepository (ownership via unique_ptr).
 * Uses inheritance + polymorphism (Req 4 + Bonus).
 */
static unique_ptr<WatchListRepository> chooseWatchListRepo() {
    cout << "\nChoose watch list storage type:\n"
         << "  1. CSV\n"
         << "  2. HTML\n"
         << "  3. SQLite (database)\n";
    int choice = readInteger("Your choice: ");
    switch (choice) {
        case 1:
            return make_unique<CsvWatchListRepository>("watchlist.csv");
        case 2:
            return make_unique<HtmlWatchListRepository>("watchlist.html");
        case 3:
            return make_unique<SqliteWatchListRepository>("watchlist.db");
        default:
            cout << "Invalid choice, defaulting to CSV.\n";
            return make_unique<CsvWatchListRepository>("watchlist.csv");
    }
}

// ── Admin mode ────────────────────────────────────────────────────────────────

static void displayMenuAdm() {
    cout << "\n===ADMIN MENU===\n"
         << "1. Add a new tutorial\n"
         << "2. Remove a tutorial\n"
         << "3. Update a tutorial\n"
         << "4. Display all tutorials\n"
         << "5. Exit\n"
         << "6. Filter by maximum comments\n\n";
}

static void runAdmin(DynamicVector<Tutorial>& db) {
    bool running = true;
    while (running) {
        displayMenuAdm();
        int opt = readInteger("Choose your option: ");
        try {
            switch (opt) {
                case 1: {
                    char title[25], presenter[50], link[50];
                    int likes, duration;
                    cout << "Enter the title (max 24 chars): "; cin >> title;
                    cout << "Enter the presenter: ";            cin >> presenter;
                    cout << "Enter a unique link: ";            cin >> link;
                    likes    = readInteger("Enter the number of likes (>= 0): ");
                    duration = readInteger("Enter the duration in seconds (> 0): ");

                    Tutorial t;
                    t.createTutorial(title, presenter, link, likes, duration);
                    // ValidationException or RepositoryException may be thrown here
                    addTutorial(db, t);
                    saveToFile(db, DB_FILE);
                    cout << "\nTutorial added successfully!\n";
                    break;
                }
                case 2: {
                    char link[50];
                    cout << "Enter the link of the tutorial to remove: "; cin >> link;
                    removeTutorial(db, link);
                    saveToFile(db, DB_FILE);
                    cout << "\nTutorial removed successfully!\n";
                    break;
                }
                case 3: {
                    char link[50];
                    cout << "Enter the link of the tutorial to update: "; cin >> link;
                    bool found = any_of(db.begin(), db.end(), [&link](const Tutorial& t) {
                        return strcmp(t.get_link(), link) == 0;
                    });
                    if (!found) throw RepositoryException("No tutorial with that link exists.");

                    cout << "What to update:\n1. Title\n2. Presenter\n3. Link\n4. Likes\n5. Duration\n";
                    int field = readInteger("Choice (1-5): ");
                    if (field < 1 || field > 5) throw InputException("Field must be between 1 and 5.");

                    if (field <= 3) {
                        char newVal[50];
                        if (field == 1)      cout << "New title: ";
                        else if (field == 2) cout << "New presenter: ";
                        else                 cout << "New link: ";
                        cin >> newVal;
                        updateTutorial(db, link, field, newVal);
                    } else {
                        int newVal = readInteger(field == 4 ? "New likes (>= 0): " : "New duration in seconds (> 0): ");
                        updateTutorial(db, link, field, newVal);
                    }
                    saveToFile(db, DB_FILE);
                    cout << "\nTutorial updated successfully!\n";
                    break;
                }
                case 4:
                    displayTutorials(db);
                    break;
                case 5:
                    running = false;
                    break;
                case 6: {
                    int maxComments = readInteger("Enter the maximum number of comments: ");
                    DynamicVector<Tutorial> filtered = filterByComments(db, maxComments);
                    if (filtered.empty())
                        cout << "\nNo tutorials found with fewer than " << maxComments << " comments.\n";
                    else {
                        cout << "\n=== Filtered Tutorials ===\n";
                        displayTutorials(filtered);
                    }
                    break;
                }
                default:
                    cout << "Invalid option, please try again.\n";
            }
        } catch (const ValidationException& e) {
            cout << "\n[Validation Error] " << e.what() << "\n";
        } catch (const RepositoryException& e) {
            cout << "\n[Repository Error] " << e.what() << "\n";
        } catch (const InputException& e) {
            cout << "\n[Input Error] " << e.what() << "\n";
        } catch (const AppException& e) {
            cout << "\n[Error] " << e.what() << "\n";
        }
    }
}

// ── User mode ─────────────────────────────────────────────────────────────────

static void displayMenuUser() {
    cout << "\n===USER MENU===\n"
         << "1. Browse tutorials by presenter\n"
         << "2. Remove a watched tutorial from list (optionally like it)\n"
         << "3. View watch list\n"
         << "4. Open watch list file\n"
         << "5. Exit\n\n";
}

static void runUser(DynamicVector<Tutorial>& db, WatchListRepository& wlRepo) {
    DynamicVector<Tutorial> watchList;
    bool running = true;
    while (running) {
        displayMenuUser();
        int opt = readInteger("Choose your option: ");
        try {
            switch (opt) {
                case 1: {
                    char presenter[50];
                    cout << "Enter presenter name (leave empty for all): ";
                    cin.getline(presenter, 50);

                    int cur = nextMatchingTutorial(db, presenter, 0);
                    if (cur == -1) {
                        cout << "\nNo tutorials found for that presenter.\n";
                        break;
                    }

                    bool browsing = true;
                    while (browsing) {
                        cout << "\n--- Tutorial ---\n";
                        displayTutorial(db[cur]);
                        openInBrowser(db[cur].get_link());
                        cout << "\nOptions:\n1. Add to watch list\n2. Next tutorial\n3. Stop browsing\n";
                        int bOpt = readInteger("Choice: ");
                        switch (bOpt) {
                            case 1:
                                try {
                                    addToWatchList(watchList, db[cur]);
                                    wlRepo.save(watchList);
                                    cout << "Added to watch list!\n";
                                } catch (const RepositoryException& e) {
                                    cout << "[Watch List] " << e.what() << "\n";
                                }
                                {
                                    int next = nextMatchingTutorial(db, presenter, (cur + 1) % db.getSize());
                                    if (next == -1 || next == cur) { cout << "No more tutorials.\n"; browsing = false; }
                                    else cur = next;
                                }
                                break;
                            case 2: {
                                int next = nextMatchingTutorial(db, presenter, (cur + 1) % db.getSize());
                                if (next == -1 || next == cur) {
                                    cout << "Looping back to start.\n";
                                    cur = nextMatchingTutorial(db, presenter, 0);
                                    if (cur == -1) browsing = false;
                                } else { cur = next; }
                                break;
                            }
                            case 3: browsing = false; break;
                            default: cout << "Invalid choice.\n";
                        }
                    }
                    break;
                }
                case 2: {
                    if (watchList.empty()) { cout << "\nYour watch list is empty!\n"; break; }
                    displayTutorials(watchList);
                    char link[50];
                    cout << "Enter the link of the tutorial you watched: "; cin >> link;
                    int like = readInteger("Did you like it? (1=Yes, 0=No): ");
                    while (like != 0 && like != 1) like = readInteger("Please enter 1 or 0: ");

                    removeFromWatchList(watchList, db, link, like == 1);
                    saveToFile(db, DB_FILE);
                    wlRepo.save(watchList);
                    cout << (like == 1 ? "\nTutorial removed and liked!\n" : "\nTutorial removed.\n");
                    break;
                }
                case 3:
                    if (watchList.empty()) cout << "\nYour watch list is empty!\n";
                    else { cout << "\n=== Your Watch List ===\n"; displayTutorials(watchList); }
                    break;

                case 4: {
                    if (watchList.empty()) {
                        cout << "\nWatch list is empty – nothing to open.\n";
                    } else {
                        wlRepo.save(watchList);
                        if (wlRepo.typeName() == "SQLite") {
                            cout << "\nSQLite database saved to: " << wlRepo.getFilePath() << "\n";
                            cout << "(Open with DB Browser for SQLite or similar tool)\n";
                        } else {
                            cout << "\nOpening " << wlRepo.typeName() << " watch list...\n";
                            openWithOS(wlRepo.getFilePath());
                        }
                    }
                    break;
                }
                case 5: running = false; break;
                default: cout << "Invalid option, please try again.\n";
            }
        } catch (const RepositoryException& e) {
            cout << "\n[Repository Error] " << e.what() << "\n";
        } catch (const ValidationException& e) {
            cout << "\n[Validation Error] " << e.what() << "\n";
        } catch (const AppException& e) {
            cout << "\n[Error] " << e.what() << "\n";
        }
    }
}

// ── Entry point ───────────────────────────────────────────────────────────────

void run() {
    DynamicVector<Tutorial> db;
    loadFromFile(db, DB_FILE);
    if (db.empty()) {
        initRepo(db);
        saveToFile(db, DB_FILE);
    }

    unique_ptr<WatchListRepository> wlRepo = chooseWatchListRepo();
    cout << "\nWatch list will be saved as " << wlRepo->typeName()
         << " (" << wlRepo->getFilePath() << ")\n";

    cout << "\n=== Tutorial Manager ===\n"
         << "1. Administrator\n"
         << "2. User\n";
    int mode = readInteger("Enter your choice: ");

    if (mode == 1)      runAdmin(db);
    else if (mode == 2) runUser(db, *wlRepo);
    else                cout << "Invalid choice. Exiting.\n";
}