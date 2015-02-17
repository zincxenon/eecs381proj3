#include <fstream>
#include <limits>
#include "Utility.h"
#include "Record.h"
#include "Collection.h"
#include <string>
#include <vector>

using namespace std

/* Construct a Collection from an input file stream in save format, using the record list,
    restoring all the Record information.
    Record list is needed to resolve references to record members.
    No check made for whether the Collection already exists or not.
    Throw Error exception if invalid data discovered in file.
    string data input is read directly into the member variable. */
Collection::Collection(ifstream& is, const vector<Record*, Less_than_ptr<Record*>>& library)
{
    int num;
    if (!(is >> name >> num))
    {
        throw_file_error();
    }
    is.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; i < num; i++)
    {
        string title;
        getline(is, title);
        Record temp_record(title);
        auto record_it = library.find(&temp_record);
        if (record_it == library.end())
        {
            throw_file_error();
        }
        elements.insert(*record_it);
    }
}

// Add the Record, throw exception if there is already a Record with the same title.
void Collection::add_member(Record* record_ptr)
{
    if (is_member_present(record_ptr))
    {
        throw Error("Record is already a member in the collection!");
    }
    elements.insert(record_ptr);
}
// Return true if the record is present, false if not.
bool Collection::is_member_present(Record* record_ptr) const
{
    return elements.find(record_ptr) != elements.end();
}
// Remove the specified Record, throw exception if the record was not found.
void Collection::remove_member(Record* record_ptr)
{
    auto it = elements.find(record_ptr);
    if (it == elements.end())
    {
        throw Error("Record is not a member in the collection!");
    }
    elements.erase(it);
}

// Write a Collection's data to a stream in save format, with endl as specified.
void Collection::save(ostream& os) const
{
    os << name << " " << elements.size();
    for (auto it = elements.begin(); it != elements.end(); ++it)
    {
        os << "\n" << (*it)->get_title();
    }
    os << "\n";
}

// Print the Collection data
ostream& operator<< (ostream& os, const Collection& collection)
{
    os << "Collection " << collection.name << " contains:";
    if (collection.empty())
    {
        os << " None";
    }
    else
    {
        for (auto it = collection.elements.begin(); it != collection.elements.end(); ++it)
        {
            os << "\n" << *(*it);
        }
    }
    return os;
}