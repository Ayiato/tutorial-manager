//
// Created by pandr on 23/03/2026.
// Week 8: DynamicVector is now a thin wrapper around vector.
//

#ifndef UNTITLED_REPOSITORY_H
#define UNTITLED_REPOSITORY_H

#include <vector>
#include <stdexcept>
#include <algorithm>
using namespace std;

/**
 * A thin wrapper around vector that preserves the old DynamicVector API
 * (getSize, isEmpty, get, remove(index), operator+=, operator+) while
 * delegating all storage to vector.
 *
 * All new service code uses vector iterators directly
 * (begin, end, push_back, erase, size, empty, clear).
 */
template <typename T>
class DynamicVector : public vector<T> {
    using Base = vector<T>;
public:
    DynamicVector() : Base() {}
    explicit DynamicVector(int cap) : Base() { Base::reserve(cap > 0 ? cap : 2); }

    /** Legacy: size as int. */
    int getSize() const { return static_cast<int>(Base::size()); }

    /** Legacy: isEmpty. */
    bool isEmpty() const { return Base::empty(); }

    /** Legacy: bounds-checked access. Uses Base::operator[] to avoid recursion. */
    T& get(int index) {
        if (index < 0 || index >= getSize())
            throw out_of_range("Index out of range");
        return Base::operator[](static_cast<size_t>(index));
    }
    const T& get(int index) const {
        if (index < 0 || index >= getSize())
            throw out_of_range("Index out of range");
        return Base::operator[](static_cast<size_t>(index));
    }

    /** Legacy: push_back alias. */
    void add(const T& element) { Base::push_back(element); }

    /** Bounds-checked operator[] (calls get which calls Base::operator[]). */
    T& operator[](int index) { return get(index); }
    const T& operator[](int index) const { return get(index); }
    T& operator[](size_t index) { return get(static_cast<int>(index)); }
    const T& operator[](size_t index) const { return get(static_cast<int>(index)); }

    /**
     * Legacy: swap-with-last removal (order NOT preserved).
     * Throws out_of_range for invalid indices.
     */
    void remove(int index) {
        if (index < 0 || index >= getSize())
            throw out_of_range("Index out of range");
        Base::operator[](static_cast<size_t>(index)) = Base::back();
        Base::pop_back();
    }

    DynamicVector& operator+=(const T& e) { Base::push_back(e); return *this; }

    friend DynamicVector operator+(DynamicVector v, const T& e) {
        v.push_back(e); return v;
    }
    friend DynamicVector operator+(const T& e, DynamicVector v) {
        return v + e;
    }
};

#endif //UNTITLED_REPOSITORY_H
