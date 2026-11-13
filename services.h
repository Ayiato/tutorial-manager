//
// services.h
//
#pragma once
#ifndef UNTITLED_SERVICES_H
#define UNTITLED_SERVICES_H

#include "repository.h"
#include "domain.h"
#include "exceptions.h"
#include "validator.h"
#include <variant>
#include <string>

using namespace std;

int find_minutes(int duration);
int find_seconds(int duration);

void loadFromFile(DynamicVector<Tutorial>& tutorials, const string& filename);
void saveToFile(const DynamicVector<Tutorial>& tutorials, const string& filename);

// These now throw RepositoryException / ValidationException instead of returning -1
void addTutorial   (DynamicVector<Tutorial>& tutorials, const Tutorial& tutorial);
void removeTutorial(DynamicVector<Tutorial>& tutorials, const char* link);
void updateTutorial(DynamicVector<Tutorial>& tutorials, const char* link,
                    int what_to_update, variant<int, char*> with_what);

void initRepo(DynamicVector<Tutorial>& tutorials);

int nextMatchingTutorial(DynamicVector<Tutorial>& all, const char* presenter, int fromIndex);

void addToWatchList    (DynamicVector<Tutorial>& watchList, const Tutorial& tutorial);
void removeFromWatchList(DynamicVector<Tutorial>& watchList,
                         DynamicVector<Tutorial>& all,
                         const char* link,
                         bool like);

DynamicVector<Tutorial> filterByComments(DynamicVector<Tutorial>& all, int max_comments);

#endif //UNTITLED_SERVICES_H