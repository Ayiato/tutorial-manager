//
// tests.h – Week 9 edition
// All service calls now use exception-based API.
// Helper wrappers restore the old int-return convention for test assertions.
//
#pragma once
#ifndef UNTITLED_TESTS_H
#define UNTITLED_TESTS_H

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "domain.h"
#include "repository.h"
#include "services.h"
#include "exceptions.h"
#include "validator.h"
#include "file_repo.h"

using namespace std;

// ── Convenience wrappers returning 1/−1 so old assertions compile ────────────

static int addTut(DynamicVector<Tutorial>& db, const Tutorial& t) {
    try { addTutorial(db, t); return 1; }
    catch (...) { return -1; }
}
static int remTut(DynamicVector<Tutorial>& db, const char* link) {
    try { removeTutorial(db, link); return 1; }
    catch (...) { return -1; }
}
static int updTut(DynamicVector<Tutorial>& db, const char* link,
                  int field, variant<int,char*> val) {
    try { updateTutorial(db, link, field, val); return 1; }
    catch (...) { return -1; }
}
static int addWL(DynamicVector<Tutorial>& wl, const Tutorial& t) {
    try { addToWatchList(wl, t); return 1; }
    catch (...) { return -1; }
}
static int remWL(DynamicVector<Tutorial>& wl, DynamicVector<Tutorial>& all,
                 const char* link, bool like) {
    try { removeFromWatchList(wl, all, link, like); return 1; }
    catch (...) { return -1; }
}

static Tutorial makeTutorial(const char* title, const char* presenter,
                              const char* link, int likes, int durationSec) {
    Tutorial t;
    t.createTutorial(const_cast<char*>(title), const_cast<char*>(presenter),
                     const_cast<char*>(link), likes, durationSec);
    return t;
}

// ─────────────────────────────────────────────────────────────────────────────
// DynamicVector tests
// ─────────────────────────────────────────────────────────────────────────────

static void test_DV_initialEmpty() {
    DynamicVector<int> v;
    assert(v.size() == 0);
    assert(v.empty());
}
static void test_DV_add() {
    DynamicVector<int> v; v.add(42);
    assert(v.size() == 1); assert(v[0] == 42);
}
static void test_DV_autoResize() {
    DynamicVector<int> v(2);
    for (int i = 0; i < 10; i++) v.add(i);
    assert(v.size() == 10);
    for (int i = 0; i < 10; i++) assert(v[i] == i);
}
static void test_DV_remove() {
    DynamicVector<int> v; v.add(1); v.add(2); v.add(3);
    v.remove(0);
    assert(v.size() == 2);
    assert(v[0] == 3); assert(v[1] == 2);
}
static void test_DV_removeOutOfRange() {
    DynamicVector<int> v; v.add(10);
    bool threw = false;
    try { v.remove(5); } catch (const out_of_range&) { threw = true; }
    assert(threw);
}
static void test_DV_getOutOfRange() {
    DynamicVector<int> v;
    bool threw = false;
    try { v[0]; } catch (const out_of_range&) { threw = true; }
    assert(threw);
}
static void test_DV_copyConstructor() {
    DynamicVector<int> v; v.add(1); v.add(2);
    DynamicVector<int> copy(v); copy[0] = 99;
    assert(v[0] == 1); assert(copy[0] == 99);
}
static void test_DV_assignmentOperator() {
    DynamicVector<int> v; v.add(5);
    DynamicVector<int> w; w = v; w[0] = 100;
    assert(v[0] == 5); assert(w[0] == 100);
}
static void test_DV_selfAssignment() {
    DynamicVector<int> v; v.add(7); v = v;
    assert(v[0] == 7); assert(v.size() == 1);
}
static void test_DV_operatorPlusEq() {
    DynamicVector<int> v; v += 10; v += 20;
    assert(v.size() == 2); assert(v[0] == 10); assert(v[1] == 20);
}
static void test_DV_operatorPlus() {
    DynamicVector<int> v; v.add(1);
    DynamicVector<int> w = v + 2;
    assert(v.size() == 1); assert(w.size() == 2); assert(w[1] == 2);
}
static void test_DV_withTutorial() {
    DynamicVector<Tutorial> v;
    Tutorial t = makeTutorial("Test","Tester","link_t",5,120);
    v.add(t);
    assert(v.size() == 1);
    assert(strcmp(v[0].get_title(),"Test") == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tutorial domain tests
// ─────────────────────────────────────────────────────────────────────────────

static void test_Tutorial_create() {
    Tutorial t = makeTutorial("Basics","Alice","http://a.com",10,125);
    assert(strcmp(t.get_title(),"Basics") == 0);
    assert(strcmp(t.get_presenter(),"Alice") == 0);
    assert(strcmp(t.get_link(),"http://a.com") == 0);
    assert(t.get_likes() == 10);
    assert(t.get_minutes() == 2); assert(t.get_seconds() == 5);
}
static void test_Tutorial_setters() {
    Tutorial t = makeTutorial("Old","Bob","oldlink",0,60);
    t.set_title(const_cast<char*>("New")); assert(strcmp(t.get_title(),"New") == 0);
    t.set_presenter(const_cast<char*>("Carol")); assert(strcmp(t.get_presenter(),"Carol") == 0);
    t.set_link(const_cast<char*>("newlink")); assert(strcmp(t.get_link(),"newlink") == 0);
    t.set_likes(99); assert(t.get_likes() == 99);
    t.set_minutes(5); assert(t.get_minutes() == 5);
    t.set_seconds(30); assert(t.get_seconds() == 30);
}

// ─────────────────────────────────────────────────────────────────────────────
// find_minutes / find_seconds
// ─────────────────────────────────────────────────────────────────────────────

static void test_findMinutesSeconds_zero()   { assert(find_minutes(0)==0); assert(find_seconds(0)==0); }
static void test_findMinutesSeconds_normal() { assert(find_minutes(90)==1); assert(find_seconds(90)==30); }
static void test_findMinutesSeconds_hour()   { assert(find_minutes(3600)==60); assert(find_seconds(3600)==0); }

// ─────────────────────────────────────────────────────────────────────────────
// addTutorial
// ─────────────────────────────────────────────────────────────────────────────

static void test_addTutorial_success() {
    DynamicVector<Tutorial> db;
    Tutorial t = makeTutorial("T1","P1","link_a",0,60);
    assert(addTut(db,t) == 1); assert(db.size() == 1);
}
static void test_addTutorial_duplicate() {
    DynamicVector<Tutorial> db;
    Tutorial t1 = makeTutorial("T1","P1","same_link",0,60);
    Tutorial t2 = makeTutorial("T2","P2","same_link",5,90);
    addTut(db,t1);
    assert(addTut(db,t2) == -1); assert(db.size() == 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// removeTutorial
// ─────────────────────────────────────────────────────────────────────────────

static void test_removeTutorial_success() {
    DynamicVector<Tutorial> db;
    addTut(db, makeTutorial("T1","P1","link_rm",0,60));
    assert(remTut(db,"link_rm") == 1); assert(db.size() == 0);
}
static void test_removeTutorial_notFound() {
    DynamicVector<Tutorial> db;
    assert(remTut(db,"ghost") == -1);
}
static void test_removeTutorial_fail() { test_removeTutorial_notFound(); }

// ─────────────────────────────────────────────────────────────────────────────
// updateTutorial
// ─────────────────────────────────────────────────────────────────────────────

static void test_updateTutorial_notFound() {
    DynamicVector<Tutorial> db;
    db.add(makeTutorial("T","P","real",0,60));
    assert(updTut(db,"fake",1,const_cast<char*>("X")) == -1);
}
static void test_updateTutorial_title() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",1,const_cast<char*>("New")) == 1);
    assert(strcmp(db[0].get_title(),"New") == 0);
}
static void test_updateTutorial_presenter() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",2,const_cast<char*>("NewP")) == 1);
    assert(strcmp(db[0].get_presenter(),"NewP") == 0);
}
static void test_updateTutorial_link() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",3,const_cast<char*>("NL")) == 1);
    assert(strcmp(db[0].get_link(),"NL") == 0);
}
static void test_updateTutorial_likes() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",4,42) == 1);
    assert(db[0].get_likes() == 42);
}
static void test_updateTutorial_duration() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",5,125) == 1);
    assert(db[0].get_minutes()==2); assert(db[0].get_seconds()==5);
}
static void test_updateTutorial_invalidField() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",99,0) == -1);
}
static void test_updateTutorial_invalidField_full() { test_updateTutorial_invalidField(); }
static void test_updateTutorial_zeroDuration_fail() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(updTut(db,"L",5,0) == -1);
}
static void test_updateTutorial_fail_not_found() { test_updateTutorial_notFound(); }
static void test_updateTutorial_link_not_found()  { test_updateTutorial_notFound(); }
static void test_updateTutorial_invalid_field()    { test_updateTutorial_invalidField(); }
// Service now rejects negative likes (throws ValidationException → returns -1):
static void test_updateTutorial_negativeLikes() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","link",0,60));
    assert(updTut(db,"link",4,-5) == -1);
}

// ─────────────────────────────────────────────────────────────────────────────
// initRepo
// ─────────────────────────────────────────────────────────────────────────────

static void test_initRepo_count() {
    DynamicVector<Tutorial> db; initRepo(db);
    assert(db.size() == 10);
}
static void test_initRepo_uniqueLinks() {
    DynamicVector<Tutorial> db; initRepo(db);
    for (int i = 0; i < (int)db.size(); i++)
        for (int j = i+1; j < (int)db.size(); j++)
            assert(strcmp(db[i].get_link(), db[j].get_link()) != 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// nextMatchingTutorial
// ─────────────────────────────────────────────────────────────────────────────

static void test_nextMatching_empty() {
    DynamicVector<Tutorial> db;
    assert(nextMatchingTutorial(db,"Alice",0) == -1);
}
static void test_nextMatching_emptyPresenter() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(nextMatchingTutorial(db,"",0) == 0);
}
static void test_nextMatching_byPresenter() {
    DynamicVector<Tutorial> db;
    db.add(makeTutorial("T1","Alice","L1",0,60));
    db.add(makeTutorial("T2","Bob","L2",0,60));
    assert(nextMatchingTutorial(db,"Bob",0) == 1);
}
static void test_nextMatching_wrapsAround() {
    DynamicVector<Tutorial> db;
    db.add(makeTutorial("T1","Alice","L1",0,60));
    db.add(makeTutorial("T2","Bob","L2",0,60));
    db.add(makeTutorial("T3","Alice","L3",0,60));
    assert(nextMatchingTutorial(db,"Alice",2) == 2);
    assert(nextMatchingTutorial(db,"Alice",1) == 2);
}
static void test_nextMatching_notFound() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","Alice","L",0,60));
    assert(nextMatchingTutorial(db,"Bob",0) == -1);
}
static void test_nextMatching_noMatchAtAll()      { test_nextMatching_notFound(); }
static void test_nextMatching_outOfBoundsIndex() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(nextMatchingTutorial(db,"",999) == 0);
}
static void test_nextMatching_nullptr() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    assert(nextMatchingTutorial(db,nullptr,0) == 0);
}
static void test_nextMatching_null_presenter()  { test_nextMatching_nullptr(); }
static void test_nextMatching_empty_repo()      { test_nextMatching_empty(); }
static void test_nextMatching_full_loop_no_match() {
    DynamicVector<Tutorial> db;
    db.add(makeTutorial("T1","Alice","l1",0,60));
    db.add(makeTutorial("T2","Bob","l2",0,60));
    assert(nextMatchingTutorial(db,"Charlie",0) == -1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Watch list
// ─────────────────────────────────────────────────────────────────────────────

static void test_addToWatchList_success() {
    DynamicVector<Tutorial> wl;
    assert(addWL(wl, makeTutorial("T","P","L",0,60)) == 1);
    assert(wl.size() == 1);
}
static void test_addToWatchList_duplicate() {
    DynamicVector<Tutorial> wl;
    addWL(wl, makeTutorial("T","P","L",0,60));
    assert(addWL(wl, makeTutorial("T2","P2","L",0,60)) == -1);
}
static void test_removeFromWatchList_notFound() {
    DynamicVector<Tutorial> db, wl;
    assert(remWL(wl,db,"ghost",false) == -1);
}
static void test_removeFromWatchList_success() {
    DynamicVector<Tutorial> db, wl;
    Tutorial t = makeTutorial("T","P","L",0,60);
    addTut(db,t); addWL(wl,t);
    assert(remWL(wl,db,"L",false) == 1); assert(wl.size() == 0);
}
static void test_removeFromWatchList_withLike() {
    DynamicVector<Tutorial> db, wl;
    Tutorial t = makeTutorial("T","P","L",5,60);
    addTut(db,t); addWL(wl,t);
    remWL(wl,db,"L",true);
    assert(db[0].get_likes() == 6);
}
static void test_removeFromWatchList_likeNotInDb() {
    DynamicVector<Tutorial> db, wl;
    Tutorial t = makeTutorial("T","P","L",0,60);
    addWL(wl,t);
    assert(remWL(wl,db,"L",true) == 1);
}
static void test_removeFromWatchList_noLike() {
    DynamicVector<Tutorial> db, wl;
    Tutorial t = makeTutorial("T","P","L",10,60);
    addTut(db,t); addWL(wl,t);
    remWL(wl,db,"L",false);
    assert(db[0].get_likes() == 10);
}
static void test_removeFromWatchList_missingFromMain()  { test_removeFromWatchList_likeNotInDb(); }
static void test_removeFromWatchList_missing_from_all() { test_removeFromWatchList_likeNotInDb(); }
static void test_removeFromWatchList_missing_from_main(){ test_removeFromWatchList_likeNotInDb(); }
static void test_removeFromWatchList_explicit_no_like() {
    DynamicVector<Tutorial> db, wl;
    Tutorial t = makeTutorial("T","P","l_skip",10,60);
    addTut(db,t); addWL(wl,t);
    assert(remWL(wl,db,"l_skip",false) == 1);
    assert(db[0].get_likes() == 10);
}
static void test_removeFromWatchList_emptyLike() {
    DynamicVector<Tutorial> db, wl;
    assert(remWL(wl,db,"any",true) == -1);
}

// ─────────────────────────────────────────────────────────────────────────────
// filterByComments
// ─────────────────────────────────────────────────────────────────────────────

static void test_filterByComments_noMatch() {
    DynamicVector<Tutorial> db; db.add(makeTutorial("T","P","L",0,60));
    db[0].set_comments(100);
    assert(filterByComments(db,10).size() == 0);
}
static void test_filterByComments_empty()   { test_filterByComments_noMatch(); }
static void test_filterByComments_no_match(){ test_filterByComments_noMatch(); }
static void test_filterByComments_multipleMatch() {
    DynamicVector<Tutorial> db;
    Tutorial t1 = makeTutorial("T1","P1","L1",0,60); t1.set_comments(5);
    Tutorial t2 = makeTutorial("T2","P2","L2",0,60); t2.set_comments(10);
    db.add(t1); db.add(t2);
    assert(filterByComments(db,11).size() == 2);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tutorial extra / edge cases
// ─────────────────────────────────────────────────────────────────────────────

static void test_Tutorial_Extra() {
    Tutorial t; t.set_comments(50);
    assert(t.get_comments() == 50);
    assert(!(t < 20));
}
static void test_Tutorial_operatorLess_False() {
    Tutorial t; t.set_comments(50);
    assert(!(t < 20));
}
static void test_Tutorial_operatorLess_equal() {
    Tutorial t; t.set_comments(50);
    assert(!(t < 50));
}

// ─────────────────────────────────────────────────────────────────────────────
// DynamicVector extra
// ─────────────────────────────────────────────────────────────────────────────

static void test_DV_assignment_overwrite() {
    DynamicVector<int> v1; v1.add(1);
    DynamicVector<int> v2; v2.add(2);
    v2 = v1; assert(v2[0] == 1);
}
static void test_DV_isEmptyAfterOps() {
    DynamicVector<int> v;
    assert(v.empty()); v.add(1); assert(!v.empty()); v.remove(0); assert(v.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// NEW: Exception / Validator tests
// ─────────────────────────────────────────────────────────────────────────────

static void test_ValidationException_emptyTitle() {
    Tutorial t = makeTutorial("","Alice","link",5,60);
    bool threw = false;
    try { TutorialValidator::validate(t); } catch (const ValidationException&) { threw = true; }
    assert(threw);
}
static void test_ValidationException_emptyPresenter() {
    Tutorial t = makeTutorial("Title","","link",5,60);
    bool threw = false;
    try { TutorialValidator::validate(t); } catch (const ValidationException&) { threw = true; }
    assert(threw);
}
static void test_ValidationException_emptyLink() {
    Tutorial t = makeTutorial("Title","Alice","",5,60);
    bool threw = false;
    try { TutorialValidator::validate(t); } catch (const ValidationException&) { threw = true; }
    assert(threw);
}
static void test_ValidationException_negativeLikes() {
    Tutorial t; t.set_title(const_cast<char*>("T")); t.set_presenter(const_cast<char*>("P"));
    t.set_link(const_cast<char*>("L")); t.set_likes(-1); t.set_minutes(1); t.set_seconds(0); t.set_comments(0);
    bool threw = false;
    try { TutorialValidator::validate(t); } catch (const ValidationException&) { threw = true; }
    assert(threw);
}
static void test_RepositoryException_duplicate() {
    DynamicVector<Tutorial> db;
    Tutorial t = makeTutorial("T","P","L",0,60);
    addTutorial(db, t);
    bool threw = false;
    try { addTutorial(db, t); } catch (const RepositoryException&) { threw = true; }
    assert(threw);
}
static void test_RepositoryException_removeNotFound() {
    DynamicVector<Tutorial> db;
    bool threw = false;
    try { removeTutorial(db, "ghost"); } catch (const RepositoryException&) { threw = true; }
    assert(threw);
}

// ─────────────────────────────────────────────────────────────────────────────
// NEW: WatchList file repo tests
// ─────────────────────────────────────────────────────────────────────────────

static void test_CsvWatchListRepository() {
    DynamicVector<Tutorial> wl;
    wl.add(makeTutorial("T1","P1","http://a.com",10,125));
    CsvWatchListRepository repo("test_watchlist.csv");
    repo.save(wl);
    ifstream f("test_watchlist.csv");
    assert(f.is_open());
    string line; getline(f,line); // no header - first line is data
    assert(line.find("T1") != string::npos);
    assert(line.find("P1") != string::npos);
    assert(line.find("http://a.com") != string::npos);
}
static void test_HtmlWatchListRepository() {
    DynamicVector<Tutorial> wl;
    wl.add(makeTutorial("T2","P2","http://b.com",5,60));
    HtmlWatchListRepository repo("test_watchlist.html");
    repo.save(wl);
    ifstream f("test_watchlist.html");
    assert(f.is_open());
    string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    assert(content.find("<table") != string::npos);
    assert(content.find("T2") != string::npos);
}
static void test_SqliteWatchListRepository() {
    DynamicVector<Tutorial> wl;
    wl.add(makeTutorial("T3","P3","http://c.com",3,90));
    SqliteWatchListRepository repo("test_watchlist.db");
    repo.save(wl);
    // verify row count via sqlite3 API
    sqlite3* db = nullptr;
    sqlite3_open("test_watchlist.db", &db);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM watchlist;",-1,&stmt,nullptr);
    sqlite3_step(stmt);
    int cnt = sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    assert(cnt == 1);
}

// ─────────────────────────────────────────────────────────────────────────────

static void testAll() {
    test_DV_initialEmpty();
    test_DV_add();
    test_DV_autoResize();
    test_DV_remove();
    test_DV_removeOutOfRange();
    test_DV_getOutOfRange();
    test_DV_copyConstructor();
    test_DV_assignmentOperator();
    test_DV_selfAssignment();
    test_DV_operatorPlusEq();
    test_DV_operatorPlus();
    test_DV_withTutorial();

    test_Tutorial_create();
    test_Tutorial_setters();

    test_findMinutesSeconds_zero();
    test_findMinutesSeconds_normal();
    test_findMinutesSeconds_hour();

    test_addTutorial_success();
    test_addTutorial_duplicate();

    test_removeTutorial_success();
    test_removeTutorial_notFound();

    test_updateTutorial_notFound();
    test_updateTutorial_title();
    test_updateTutorial_presenter();
    test_updateTutorial_link();
    test_updateTutorial_likes();
    test_updateTutorial_duration();
    test_updateTutorial_invalidField();
    test_updateTutorial_zeroDuration_fail();
    test_updateTutorial_fail_not_found();
    test_updateTutorial_link_not_found();
    test_updateTutorial_invalid_field();
    test_updateTutorial_invalidField_full();
    test_updateTutorial_negativeLikes();

    test_initRepo_count();
    test_initRepo_uniqueLinks();

    test_nextMatching_empty();
    test_nextMatching_emptyPresenter();
    test_nextMatching_byPresenter();
    test_nextMatching_wrapsAround();
    test_nextMatching_notFound();
    test_nextMatching_noMatchAtAll();
    test_nextMatching_outOfBoundsIndex();
    test_nextMatching_nullptr();
    test_nextMatching_null_presenter();
    test_nextMatching_empty_repo();
    test_nextMatching_full_loop_no_match();

    test_addToWatchList_success();
    test_addToWatchList_duplicate();
    test_removeFromWatchList_notFound();
    test_removeFromWatchList_success();
    test_removeFromWatchList_withLike();
    test_removeFromWatchList_likeNotInDb();
    test_removeFromWatchList_noLike();
    test_removeFromWatchList_missingFromMain();
    test_removeFromWatchList_missing_from_all();
    test_removeFromWatchList_missing_from_main();
    test_removeFromWatchList_explicit_no_like();
    test_removeFromWatchList_emptyLike();

    test_filterByComments_noMatch();
    test_filterByComments_empty();
    test_filterByComments_no_match();
    test_filterByComments_multipleMatch();

    test_Tutorial_Extra();
    test_Tutorial_operatorLess_False();
    test_Tutorial_operatorLess_equal();

    test_DV_assignment_overwrite();
    test_DV_isEmptyAfterOps();

    // New Week 9 tests
    test_ValidationException_emptyTitle();
    test_ValidationException_emptyPresenter();
    test_ValidationException_emptyLink();
    test_ValidationException_negativeLikes();
    test_RepositoryException_duplicate();
    test_RepositoryException_removeNotFound();
    test_CsvWatchListRepository();
    test_HtmlWatchListRepository();
    test_SqliteWatchListRepository();

    cout << "\n[TESTS] All tests passed!\n\n";
}

#endif //UNTITLED_TESTS_H