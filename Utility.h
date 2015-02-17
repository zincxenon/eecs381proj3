#ifndef UTILITY_H
#define UTILITY_H

#include <istream>

/* Utility functions, constants, and classes used by more than one other modules */

// a simple class for error exceptions - msg points to a C-string error message
struct Error {
	Error(const char* msg_ = "") :
		msg(msg_)
		{}
	const char* msg;
};

// Compare two objects (passed by const&) using T's operator<
template<typename T>
struct Less_than_ref {
    bool operator() (const T& t1, const T& t2) const {return t1 < t2;}
};

// Compare two pointers (T is a pointer type) using *T's operator<
template<typename T>
struct Less_than_ptr {
    bool operator()(const T p1, const T p2) const {return *p1 < *p2;}
};

void throw_file_error();

int integer_read();

#endif