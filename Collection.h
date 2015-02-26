#ifndef COLLECTION_H
#define COLLECTION_H

#include <fstream>
#include <ostream>
#include "Utility.h"
#include "Record.h"
#include <string>
#include <set>

/* Collections contain a name and a container of members,
represented as pointers to Records.
Collection objects manage their own Record container. 
The container of Records is not available to clients.
*/

class Collection {

public:
	// Construct a collection with the specified name and no members
	Collection(const std::string& name_) : name{name_} {}

	// Construct a collection with the given name and the same elements as those in original
	Collection(const std::string& name_, const Collection& original) : name{name_}, elements(original.elements) {}
	
	/* Construct a Collection from an input file stream in save format, using the record list,
	restoring all the Record information.
	Record list is needed to resolve references to record members.
	No check made for whether the Collection already exists or not.
	Throw Error exception if invalid data discovered in file.
	std::string data input is read directly into the member variable. */
    Collection(std::ifstream& is, const std::vector<Record*>& library);

	// Accessors
	std::string get_name() const
		{return name;}

	const std::set<Record*, Less_than_ptr<Record*>> get_elements() const
		{return elements;}
		
	// Add the Record, throw exception if there is already a Record with the same title.
	void add_member(Record* record_ptr);
	// Return true if there are no members; false otherwise
	bool empty() const
		{ return elements.empty(); }
	// Return true if the record is present, false if not.
	bool is_member_present(Record* record_ptr) const;
	// Remove the specified Record, throw exception if the record was not found.
	void remove_member(Record* record_ptr);
	// discard all members
	void clear()
		{ elements.clear(); }

	// Write a Collections's data to a stream in save format, with endl as specified.
	void save(std::ostream& os) const;

	// Set union of the records in rhs and this
	Collection& operator+=(const Collection &rhs);

	// This operator defines the order relation between Collections, based just on the name
	bool operator< (const Collection& rhs) const
		{ return name < rhs.get_name(); }

	bool operator==(const Collection &rhs) const {return name == rhs.get_name(); }

	bool operator!=(const Collection &rhs) const {return !(*this == rhs); }
	
	friend std::ostream& operator<< (std::ostream& os, const Collection& collection);
		
private:
	std::string name;
    std::set<Record*, Less_than_ptr<Record*>> elements;

    void print_record_title(Record* record, std::ostream& os);
};

// Print the Collection data
std::ostream& operator<< (std::ostream& os, const Collection& collection);

#endif