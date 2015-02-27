#ifndef UTILITY_H
#define UTILITY_H

/* Utility functions, constants, and classes used by more than one other modules */

extern const char * FILE_ERROR_MSG;

// a simple class for error exceptions - msg points to a C-string error message
struct Error {
	Error(const char* msg_ = "") :
		msg(msg_)
		{}
	const char* msg;
};

// an error class for error exceptions that do not require the rest of the line to be cleared
struct ErrorNoClear {
	ErrorNoClear(const char* msg_ = "") :
			msg(msg_)
	{}
	const char* msg;
};

// Compare two pointers (T is a pointer type) using *T's operator<
template<typename T>
struct Less_than_ptr {
    bool operator()(const T p1, const T p2) const {return *p1 < *p2;}
};

#endif