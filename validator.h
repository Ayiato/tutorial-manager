//
// validator.h – Validator class for Tutorial entities
//
#pragma once
#ifndef UNTITLED_VALIDATOR_H
#define UNTITLED_VALIDATOR_H

#include "domain.h"
#include "exceptions.h"
#include <cstring>
#include <string>
using namespace std;
class TutorialValidator {
public:
    /**
     * Validates all fields of a Tutorial.
     * Throws ValidationException with a descriptive message on failure.
     */
    static void validate(const Tutorial& t) {
        string errors;
        if (strlen(t.get_title()) > 24)
            errors += "Title must be at most 24 characters. ";
        if (t.get_minutes() > 60)
            errors += "Duration cannot exceed 60 minutes. ";
        if (t.get_seconds() < 0)
            errors += "Seconds must be above 0. ";
        if (strlen(t.get_title()) == 0)
            errors += "Title must not be empty. ";
        if (strlen(t.get_presenter()) == 0)
            errors += "Presenter must not be empty. ";
        if (strlen(t.get_link()) == 0)
            errors += "Link must not be empty. ";
        if (t.get_likes() < 0)
            errors += "Likes must be >= 0. ";
        if (t.get_minutes() == 0 && t.get_seconds() == 0)
            errors += "Duration must be greater than 0. ";
        if (t.get_comments() < 0)
            errors += "Comments must be >= 0. ";

        if (!errors.empty())
            throw ValidationException(errors);
    }
};

#endif // UNTITLED_VALIDATOR_H
