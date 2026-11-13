//
// exceptions.h – custom exception classes for the Tutorial Manager
//
#pragma once
#ifndef UNTITLED_EXCEPTIONS_H
#define UNTITLED_EXCEPTIONS_H

#include <stdexcept>
#include <string>
using namespace std;
/**
 * Base class for all application-specific exceptions.
 */
class AppException : public runtime_error {
public:
    explicit AppException(const string& msg) : runtime_error(msg) {}
};

/**
 * Thrown by the repository when an entity constraint is violated
 * (e.g. duplicate link, or element not found).
 */
class RepositoryException : public AppException {
public:
    explicit RepositoryException(const string& msg) : AppException(msg) {}
};

/**
 * Thrown by Validator classes when a Tutorial field fails validation.
 */
class ValidationException : public AppException {
public:
    explicit ValidationException(const string& msg) : AppException(msg) {}
};

/**
 * Thrown when the user provides invalid console input (bad type, out of range, etc.).
 */
class InputException : public AppException {
public:
    explicit InputException(const string& msg) : AppException(msg) {}
};

#endif // UNTITLED_EXCEPTIONS_H
