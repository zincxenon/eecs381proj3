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

void throw_file_error();

int integer_read();

#endif