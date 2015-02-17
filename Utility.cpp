#include "Utility.h"
#include <iostream>

using namespace std;

void throw_file_error()
{
    throw Error("Invalid data found in file!");
}

int integer_read()
{
    int integer;
    if (!(cin >> integer))
    {
        throw Error("Could not read an integer value!");
    }
    return integer;
}