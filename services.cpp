//
// services.cpp
//
#include "services.h"
#include "repository.h"
#include "domain.h"
#include "exceptions.h"
#include "validator.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <cstring>
#include <variant>

using namespace std;

// ── Helpers ──────────────────────────────────────────────────────────────────

int find_minutes(int duration) { return duration / 60; }
int find_seconds(int duration) { return duration % 60; }

static auto findByLink(DynamicVector<Tutorial>& tutorials, const char* link) {
    return find_if(tutorials.begin(), tutorials.end(), [link](const Tutorial& t) {
        return strcmp(t.get_link(), link) == 0;
    });
}

// ── File I/O ─────────────────────────────────────────────────────────────────

/**
 * Reads tutorials from a text file using the >> extraction operator.
 * Each line holds one tutorial in pipe-separated format.
 * Existing contents of `tutorials` are replaced.
 * If the file does not exist yet (first run), the function returns silently.
 */
void loadFromFile(DynamicVector<Tutorial>& tutorials, const string& filename) {
    tutorials.clear();
    ifstream fin(filename);
    if (!fin.is_open()) return;
    Tutorial t;
    while (fin >> t)
        tutorials.push_back(t);
}

/**
 * Writes all tutorials to a text file using the << insertion operator.
 * Overwrites the file completely.
 * Throws RepositoryException if the file cannot be opened for writing.
 */
void saveToFile(const DynamicVector<Tutorial>& tutorials, const string& filename) {
    ofstream fout(filename, ios::trunc);
    if (!fout.is_open())
        throw RepositoryException("Cannot open file for writing: " + filename);
    for (const Tutorial& t : tutorials)
        fout << t << "\n";
}

// ── Seed data ─────────────────────────────────────────────────────────────────

/**
 * Populates `tutorials` with 10 hard-coded tutorials for first-run seeding.
 * Each tutorial has a unique link. Called only when the persistent file is empty.
 */
void initRepo(DynamicVector<Tutorial>& tutorials) {
    const char* titles[]     = {"C++ Basics","Pointers 101","Memory Mgmt","STL Vectors",
                                 "OOP Principles","Inheritance","Polymorphism",
                                 "Templates","Exceptions","Lambda Funcs"};
    const char* presenters[] = {"Bjarne","Alice","Bob","Charlie","David",
                                 "Eve","Frank","Grace","Heidi","Ivan"};
    const char* links[] = {
        "https://www.youtube.com/watch?v=vLnPwxZdW4Y",
        "https://www.youtube.com/watch?v=DTxHyVnPNRo",
        "https://www.youtube.com/watch?v=yEEkG1aM-zE",
        "https://www.youtube.com/watch?v=PocJ5jP-3JM",
        "https://www.youtube.com/watch?v=wN0x9eielBg",
        "https://www.youtube.com/watch?v=x_q2A_y2q1I",
        "https://www.youtube.com/watch?v=wKdbN9D6eE4",
        "https://www.youtube.com/watch?v=I-hZkUa9mIs",
        "https://www.youtube.com/watch?v=mZpB-y2pU10",
        "https://www.youtube.com/watch?v=mWgmBBz0y8c"
    };
    int comments[] = {45,12,89,34,150,22,5,76,11,99};

    for (int i = 0; i < 10; i++) {
        Tutorial t;
        t.createTutorial(const_cast<char*>(titles[i]),
                         const_cast<char*>(presenters[i]),
                         const_cast<char*>(links[i]),
                         100 + i*9 + 30*i,
                         300 + (i*13 + 30));
        t.set_comments(comments[i]);
        addTutorial(tutorials, t);
    }
}

// ── CRUD – now throw on error ─────────────────────────────────────────────────

/**
 * Validates and adds a tutorial to the repository.
 * Throws ValidationException if any field is invalid (via TutorialValidator).
 * Throws RepositoryException if a tutorial with the same link already exists.
 */
void addTutorial(DynamicVector<Tutorial>& tutorials, const Tutorial& tutorial) {
    TutorialValidator::validate(tutorial);
    if (findByLink(tutorials, tutorial.get_link()) != tutorials.end())
        throw RepositoryException(
            string("A tutorial with link '") + tutorial.get_link() + "' already exists.");
    tutorials.push_back(tutorial);
}

/**
 * Removes a tutorial from the repository by its link.
 * Throws RepositoryException if no tutorial with the given link is found.
 */
void removeTutorial(DynamicVector<Tutorial>& tutorials, const char* link) {
    auto it = findByLink(tutorials, link);
    if (it == tutorials.end())
        throw RepositoryException(string("No tutorial with link '") + link + "' found.");
    tutorials.erase(it);
}

/**
 * Updates one field of an existing tutorial identified by `link`.
 * what_to_update: 1=Title, 2=Presenter, 3=Link, 4=Likes, 5=Duration (seconds).
 * Throws RepositoryException if no tutorial with that link is found.
 * Throws ValidationException if the new value is invalid (e.g. negative likes, zero duration).
 * Throws ValidationException if `what_to_update` is not in the range 1–5.
 */
void updateTutorial(DynamicVector<Tutorial>& tutorials, const char* link,
                    int what_to_update, variant<int, char*> with_what) {
    auto it = findByLink(tutorials, link);
    if (it == tutorials.end())
        throw RepositoryException(string("No tutorial with link '") + link + "' found.");

    Tutorial& t = *it;
    switch (what_to_update) {
        case 1: t.set_title(get<char*>(with_what));     break;
        case 2: t.set_presenter(get<char*>(with_what)); break;
        case 3: t.set_link(get<char*>(with_what));      break;
        case 4: {
            int v = get<int>(with_what);
            if (v < 0) throw ValidationException("Likes must be >= 0.");
            t.set_likes(v);
            break;
        }
        case 5: {
            int secs = get<int>(with_what);
            if (secs <= 0) throw ValidationException("Duration must be > 0 seconds.");
            t.set_minutes(find_minutes(secs));
            t.set_seconds(find_seconds(secs));
            break;
        }
        default: throw ValidationException("Invalid field index for update.");
    }
}



/**
 * Returns the index of the next tutorial in `all` whose presenter matches
 * `presenter`, starting the search from `fromIndex` (wraps around).
 * An empty or null presenter matches every tutorial.
 * Returns -1 if the repository is empty or no matching tutorial exists.
 */
int nextMatchingTutorial(DynamicVector<Tutorial>& all, const char* presenter, int fromIndex) {
    int n = static_cast<int>(all.size());
    if (n == 0) return -1;
    if (fromIndex < 0 || fromIndex >= n) fromIndex = 0;
    bool matchAll = (presenter == nullptr || presenter[0] == '\0');
    for (int offset = 0; offset < n; offset++) {
        int idx = (fromIndex + offset) % n;
        if (matchAll || strcmp(all[idx].get_presenter(), presenter) == 0)
            return idx;
    }
    return -1;
}

/**
 * Adds a tutorial to the watch list.
 * Throws RepositoryException if a tutorial with the same link is already present.
 */
void addToWatchList(DynamicVector<Tutorial>& watchList, const Tutorial& tutorial) {
    bool already = any_of(watchList.begin(), watchList.end(), [&](const Tutorial& t) {
        return strcmp(t.get_link(), tutorial.get_link()) == 0;
    });
    if (already)
        throw RepositoryException("Tutorial is already in the watch list.");
    watchList.push_back(tutorial);
}

/**
 * Removes a tutorial from the watch list by link.
 * If `like` is true, also increments the likes counter of the matching tutorial in `all`
 * (if it still exists there).
 * Throws RepositoryException if the link is not found in the watch list.
 */
void removeFromWatchList(DynamicVector<Tutorial>& watchList,
                         DynamicVector<Tutorial>& all,
                         const char* link,
                         bool like) {
    auto wIt = find_if(watchList.begin(), watchList.end(), [link](const Tutorial& t) {
        return strcmp(t.get_link(), link) == 0;
    });
    if (wIt == watchList.end())
        throw RepositoryException(string("No tutorial with link '") + link + "' in the watch list.");

    watchList.erase(wIt);

    if (like) {
        auto aIt = findByLink(all, link);
        if (aIt != all.end())
            aIt->set_likes(aIt->get_likes() + 1);
    }
}

/**
 * Returns a new DynamicVector containing only the tutorials whose comment count
 * is strictly less than `max_comments`, using std::copy_if.
 * The original `all` vector is not modified.
 */
DynamicVector<Tutorial> filterByComments(DynamicVector<Tutorial>& all, int max_comments) {
    DynamicVector<Tutorial> filtered;
    copy_if(all.begin(), all.end(), back_inserter(filtered), [max_comments](const Tutorial& t) {
        return t < max_comments;
    });
    return filtered;
}