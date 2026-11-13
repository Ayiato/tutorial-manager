
#ifndef UNTITLED_DOMAIN_H
#define UNTITLED_DOMAIN_H

#include <iostream>
#include <string.h>
#include <string>
using namespace std;
class Tutorial {
private:
    char title[25];
    char presenter[50];
    int minutes;
    int seconds;
    int likes;
    char link[50];
    int comments;

public:
    Tutorial() : minutes(0), seconds(0), likes(0), comments(10) {
        title[0]     = '\0';
        presenter[0] = '\0';
        link[0]      = '\0';
    }

    void set_title(char* t) {
        strcpy(this->title, t);
    }
    void set_presenter(char* p) {
        strcpy(this->presenter, p);
    }
    void set_minutes(int m) {
        this->minutes = m;
    }
    void set_seconds(int s) {
        this->seconds = s;
    }
    void set_likes(int l) {
        this->likes = l;
    }
    void set_link(char* l) {
        strcpy(this->link, l);
    }
    void set_comments(int c) {
        this->comments = c;
    }

    const char* get_title()     const {
        return title;
    }
    const char* get_presenter() const {
        return presenter;
    }
    int get_minutes()   const {
        return minutes;
    }
    int get_seconds()   const {
        return seconds;
    }
    int get_likes()     const {
        return likes;
    }
    const char* get_link()      const {
        return link;
    }
    int get_comments()  const {
        return comments;
    }

    bool operator<(int limit) const { return comments < limit; }

    void createTutorial(char* titlee, char* presenterr, char* linkk, int likess, int duration) {
        set_title(titlee);
        set_presenter(presenterr);
        set_link(linkk);
        set_likes(likess);
        set_minutes(duration / 60);
        set_seconds(duration % 60);
    }

    /**
     * Insertion operator: writes one tutorial per line as pipe-separated fields.
     * Format: title|presenter|link|likes|minutes|seconds|comments
     */
    friend ostream& operator<<(ostream& os, const Tutorial& t) {
        os << t.title     << "|"
           << t.presenter << "|"
           << t.link      << "|"
           << t.likes     << "|"
           << t.minutes   << "|"
           << t.seconds   << "|"
           << t.comments;
        return os;
    }

    /**
     * Extraction operator: reads one tutorial from a pipe-separated line.
     * Format: title|presenter|link|likes|minutes|seconds|comments
     */
    friend istream& operator>>(istream& is, Tutorial& t) {
        string title, presenter, link;
        int likes, minutes, seconds, comments;

        if (!getline(is, title,     '|'))
            return is;
        if (!getline(is, presenter, '|'))
            return is;
        if (!getline(is, link,      '|'))
            return is;

        char buf[8];
        is.getline(buf, sizeof(buf), '|');
        likes    = stoi(buf);
        is.getline(buf, sizeof(buf), '|');
        minutes  = stoi(buf);
        is.getline(buf, sizeof(buf), '|');
        seconds  = stoi(buf);
        // last field ends at newline
        string cstr;
        if (!getline(is, cstr)) return is;
        comments = stoi(cstr);

        strncpy(t.title,     title.c_str(),     sizeof(t.title)     - 1); t.title[sizeof(t.title)-1]     = '\0';
        strncpy(t.presenter, presenter.c_str(), sizeof(t.presenter) - 1); t.presenter[sizeof(t.presenter)-1] = '\0';
        strncpy(t.link,      link.c_str(),      sizeof(t.link)      - 1); t.link[sizeof(t.link)-1]       = '\0';
        t.likes    = likes;
        t.minutes  = minutes;
        t.seconds  = seconds;
        t.comments = comments;
        return is;
    }
};

#endif //UNTITLED_DOMAIN_H
