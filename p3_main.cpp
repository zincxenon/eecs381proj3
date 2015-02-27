#include <iostream>
#include <fstream>
#include <limits>
#include <istream>
#include <cctype>
#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>

#include <string>
#include <vector>
#include <map>
#include <list>

#include "Record.h"
#include "Collection.h"
#include "Utility.h"

using namespace std;

const char * UNRECOGNIZED_MSG = "Unrecognized command!";
const char * FILE_OPEN_FAIL_MSG = "Could not open file!";
const char * LIBRARY_EMPTY_MSG = "Library is empty\n";

/* data types */

// Collections in the catalog are held using the following container
typedef vector<Collection> Catalog_container;

// Record IDs are compared using this functor
struct ID_compare {
    bool operator() (const Record *lhs, const Record *rhs) const { return lhs->get_ID() < rhs->get_ID(); }
};

// Struct holding the library and catalog information
struct data_container {
    Catalog_container catalog;
    Record_container library_title;
    Record_container library_id;
};

/* Function pointer used in command map
 * Returns true if the user is finished, false otherwise
 */
typedef bool (*data_container_func)(data_container&);

/* lib cat helper functions dec */

// Performs a title-based lower_bound on the library for a given record
Record_container::iterator lib_title_lower_bound(data_container& lib_cat, Record* record);
// Performs an id-based lower_bound on the library for a given record
Record_container::iterator lib_id_lower_bound(data_container& lib_cat, Record* record);
// Performs a lower_bound on the catalog for a collection
Catalog_container::iterator catalog_lower_bound(data_container& lib_cat, Collection& collection);

// Read a title from stdin and then return an iterator to a record in the library with that title
Record_container::iterator read_title_get_iter(data_container& lib_cat);
// Read an id from stdin and then return an iterator to a record in the library with that id
Record_container::iterator read_id_get_iter(data_container& lib_cat);
// Read a name from stdin and then return an iterator to a collection in the catalog with that name
Catalog_container::iterator read_name_get_iter(data_container& lib_cat);

// Checks if the provided title is already in the library
void check_title_in_library(data_container& lib_cat, string title);

// Inserts a record into the library and returns a pointer to the inserted record
Record* insert_record(data_container& lib_cat, Record* record);
// Inserts a collection into the catalog
void insert_collection(data_container& lib_cat, Collection&& collection);

// Clears the library and its data
void clear_library_data(data_container& lib_cat);

/* other functions dec */

// Reads a title from stdin
string title_read(istream &is);
// Processes a string and removes excess whitespace
string parse_title(string& title_string);
// Reads an integer from stdin and throws an error if it fails
int integer_read();

/* main lib cat functions dec */

bool find_record(data_container& lib_cat);
bool find_string(data_container& lib_cat);

bool list_ratings(data_container& lib_cat);

bool print_record(data_container& lib_cat);
bool print_collection(data_container& lib_cat);
bool print_library(data_container& lib_cat);
bool print_catalog(data_container& lib_cat);
bool print_allocation(data_container& lib_cat);

bool collection_statistics(data_container& lib_cat);
bool combine_collections(data_container& lib_cat);

bool modify_rating(data_container& lib_cat);
bool modify_title(data_container& lib_cat);

bool add_record(data_container& lib_cat);
bool add_collection(data_container& lib_cat);
bool add_member(data_container& lib_cat);

bool delete_record(data_container& lib_cat);
bool delete_collection(data_container& lib_cat);
bool delete_member(data_container& lib_cat);

bool clear_library(data_container& lib_cat);
bool clear_catalog(data_container& lib_cat);
bool clear_all(data_container& lib_cat);

bool save_all(data_container& lib_cat);

bool restore_all(data_container& lib_cat);

bool quit(data_container& lib_cat);

/* main */

int main()
{

    data_container lib_cat;
    map<string, data_container_func> function_map {
            {"fr", find_record},
            {"fs", find_string},

            {"lr", list_ratings},

            {"pr", print_record},
            {"pc", print_collection},
            {"pL", print_library},
            {"pC", print_catalog},
            {"pa", print_allocation},

            {"cs", collection_statistics},
            {"cc", combine_collections},

            {"mr", modify_rating},
            {"mt", modify_title},

            {"ar", add_record},
            {"ac", add_collection},
            {"am", add_member},

            {"dr", delete_record},
            {"dc", delete_collection},
            {"dm", delete_member},

            {"cL", clear_library},
            {"cC", clear_catalog},
            {"cA", clear_all},

            {"sA", save_all},

            {"rA", restore_all},

            {"qq", quit}
    };
    while (true)
    {
        try
        {
            char action, object;
            cout << "\nEnter command: ";
            if (!(cin >> action >> object))
            {
                throw Error(UNRECOGNIZED_MSG);
            }
            string command;
            command += action;
            command += object;
            if (function_map.find(command) == function_map.end())
            {
                throw Error(UNRECOGNIZED_MSG);
            }
            if (function_map[command](lib_cat))
            {
                return 0;
            }
        } catch (Error& e) {
            cout << e.msg << "\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } catch (ErrorNoClear& e)
        {
            cout << e.msg << "\n";
        } catch (...)
        {
            // print error message
            return 0;
        }
    }
}

/* lib cat helper functions impl */

// Performs a title-based lower_bound on the library for a given record
Record_container::iterator lib_title_lower_bound(data_container& lib_cat, Record* record)
{
    return lower_bound(lib_cat.library_title.begin(), lib_cat.library_title.end(), record, Title_compare());
}
// Performs an id-based lower_bound on the library for a given record
Record_container::iterator lib_id_lower_bound(data_container& lib_cat, Record* record)
{
    return lower_bound(lib_cat.library_id.begin(), lib_cat.library_id.end(), record, ID_compare());
}
// Performs a lower_bound on the catalog for a collection
Catalog_container::iterator catalog_lower_bound(data_container& lib_cat, Collection& collection)
{
    return lower_bound(lib_cat.catalog.begin(), lib_cat.catalog.end(), collection);
}

// Read a title from stdin and then return an iterator to a record in the library with that title
Record_container::iterator read_title_get_iter(data_container& lib_cat)
{
    string title = title_read(cin);
    Record temp_record(title);
    auto record_iter = lib_title_lower_bound(lib_cat, &temp_record);
    if (record_iter == lib_cat.library_title.end() || **record_iter != temp_record)
    {
        throw ErrorNoClear("No record with that title!");
    }
    return record_iter;
}
// Read an id from stdin and then return an iterator to a record in the library with that id
Record_container::iterator read_id_get_iter(data_container& lib_cat)
{
    int id = integer_read();
    Record temp_record(id);
    auto record_iter = lib_id_lower_bound(lib_cat, &temp_record);
    if (record_iter == lib_cat.library_id.end() || **record_iter != temp_record)
    {
        throw Error("No record with that ID!");
    }
    return record_iter;
}
// Read a name from stdin and then return an iterator to a collection in the catalog with that name
Catalog_container::iterator read_name_get_iter(data_container& lib_cat)
{
    string name;
    cin >> name;
    Collection temp_collection(name);
    auto collection_iter = catalog_lower_bound(lib_cat, temp_collection);
    if (collection_iter == lib_cat.catalog.end() || *collection_iter != temp_collection)
    {
        throw Error("No collection with that name!");
    }
    return collection_iter;
}

// Checks if the provided title is already in the library
void check_title_in_library(data_container& lib_cat, string title)
{
    Record temp_record(title);
    auto title_check = lib_title_lower_bound(lib_cat, &temp_record);
    if (title_check != lib_cat.library_title.end() && **title_check == temp_record)
    {
        throw ErrorNoClear("Library already has a record with this title!");
    }
}

// Inserts a record into the library and returns a pointer to the inserted record
Record* insert_record(data_container& lib_cat, Record* record)
{
    auto title_lower_bound = lib_title_lower_bound(lib_cat, record);
    try
    {
        lib_cat.library_title.insert(title_lower_bound, record);
    } catch (...)
    {
        delete record;
        throw;
    }
    try
    {
        lib_cat.library_id.insert(lib_id_lower_bound(lib_cat, record), record);
    } catch (...)
    {
        // if this insertion fails, we need to remove the record from the other container!
        lib_cat.library_title.erase(title_lower_bound);
        delete record;
        throw;
    }
    return record;
}

// Inserts a collection into the catalog
void insert_collection(data_container& lib_cat, Collection&& collection)
{
    auto collection_iter = catalog_lower_bound(lib_cat, collection);
    if (collection_iter != lib_cat.catalog.end() && *collection_iter == collection)
    {
        throw Error("Catalog already has a collection with this name!");
    }
    lib_cat.catalog.insert(collection_iter, collection);
}

// Clears the library and its data
void clear_library_data(data_container& lib_cat)
{
    for_each(lib_cat.library_title.begin(), lib_cat.library_title.end(), [](Record* record) { delete record; });
    lib_cat.library_title.clear();
    lib_cat.library_id.clear();
}

/* other functions impl */

// Reads a title from stdin
string title_read(istream &is)
{
    string raw_title;
    getline(is, raw_title);
    string title = parse_title(raw_title);
    // valid titles must be at 1 character long
    if (title.size() == 0)
    {
        throw ErrorNoClear("Could not read a title!");
    }
    return title;
}
// functor used to parse a string and remove excess whitespace
struct title_parser
{
    void operator()(char c)
    {
        if (!isspace(c))
        {
            title.push_back(c);
            remove_whitespace = false;
        } else
        {
            /* if remove_whitespace is false, the last character read must not have been whitespace and we need
             * a space, otherwise do not add this character to the string
             */
            if (!remove_whitespace)
            {
                title.push_back(' ');
            }
            remove_whitespace = true;
        }
    }
    // removes terminating whitespace from processed string
    void finalize()
    {
        if (isspace(title.back()))
        {
            title.pop_back();
        }
    }
    string get_title() { return title; }
private:
    string title;
    bool remove_whitespace = true;
};
// Processes a string and removes excess whitespace
string parse_title(string& original)
{
    title_parser title_helper;
    title_helper = for_each(original.begin(), original.end(), title_helper);
    title_helper.finalize();
    return title_helper.get_title();
}
// Reads an integer from stdin and throws an error if it fails
int integer_read()
{
    int integer;
    if (!(cin >> integer))
    {
        throw Error("Could not read an integer value!");
    }
    return integer;
}

/* main lib cat functions impl */

bool find_record(data_container& lib_cat)
{
    auto record_ptr = *read_title_get_iter(lib_cat);
    cout << *record_ptr << "\n";
    return false;
}
// functor used to find records with titles containing a string
struct string_finder
{
    string_finder(string key_) : key(string_to_lower(key_)) {}
    void operator()(Record* record)
    {
        string temp_title = string_to_lower(record->get_title());
        if (temp_title.find(key) != string::npos)
        {
            matching_records.push_back(record);
        }
    }
    list<Record*> get_matches() { return matching_records; }
private:
    list<Record*> matching_records;
    string key;

    // converts a string to all lowercase
    static string string_to_lower(string original)
    {
        transform(original.begin(), original.end(), original.begin(), ::tolower);
        return original;
    }
};
bool find_string(data_container& lib_cat)
{
    string key;
    cin >> key;
    string_finder string_helper(key);
    string_helper = for_each(lib_cat.library_title.begin(), lib_cat.library_title.end(), string_helper);
    list<Record*> matching_records = string_helper.get_matches();
    if (matching_records.size() == 0)
    {
        throw Error("No records contain that string!");
    }
    ostream_iterator<Record*> out_it(cout, "\n");
    copy(matching_records.begin(), matching_records.end(), out_it);
    return false;
}

bool list_ratings(data_container& lib_cat)
{
    if (lib_cat.library_title.empty())
    {
        cout << LIBRARY_EMPTY_MSG;
        return false;
    }
    Record_container sorted_by_rating = lib_cat.library_title;
    // sort the new container by rating first, then by title
    sort(sorted_by_rating.begin(), sorted_by_rating.end(), [](const Record* a, const Record* b)
        { return a->get_rating() == b->get_rating() ? *a < *b : a->get_rating() > b->get_rating(); });
    ostream_iterator<Record*> out_it(cout, "\n");
    copy(sorted_by_rating.begin(), sorted_by_rating.end(), out_it);
    return false;
}

bool print_record(data_container& lib_cat)
{
    Record *record_ptr = *read_id_get_iter(lib_cat);
    cout << *record_ptr << "\n";
    return false;
}
bool print_collection(data_container& lib_cat)
{
    Collection& collection = *read_name_get_iter(lib_cat);
    cout << collection;
    return false;
}
bool print_library(data_container& lib_cat)
{
    if (lib_cat.library_title.empty())
    {
        cout << LIBRARY_EMPTY_MSG;
    }
    else
    {
        cout << "Library contains " << lib_cat.library_title.size() << " records:\n";
        ostream_iterator<Record*> out_it(cout, "\n");
        copy(lib_cat.library_title.begin(), lib_cat.library_title.end(), out_it);
    }
    return false;
}
bool print_catalog(data_container& lib_cat)
{
    if (lib_cat.catalog.empty())
    {
        cout << "Catalog is empty\n";
    }
    else
    {
        cout << "Catalog contains " << lib_cat.catalog.size() << " collections:\n";
        ostream_iterator<Collection> out_it(cout);
        copy(lib_cat.catalog.begin(), lib_cat.catalog.end(), out_it);
    }
    return false;
}
bool print_allocation(data_container& lib_cat)
{
    cout << "Memory allocations:\n";
    cout << "Records: " << lib_cat.library_title.size() << "\n";
    cout << "Collections: " << lib_cat.catalog.size() << "\n";
    return false;
}

// functor used to gather stats about the collections
struct Collection_stats {
public:
    void operator()(Collection& collection)
    {
        // the one range for
        for (auto& record : collection.get_elements())
        {
            process_record(record);
        }
    }
    void process_record(Record* const record)
    {
        // add key to map if it has not yet been added
        if (record_count.find(record->get_ID()) == record_count.end())
        {
            record_count[record->get_ID()] = 0;
        }
        // find the current value of the record's ID in the map
        int& current_count = record_count[record->get_ID()];
        if (current_count == 0)
        {
            // if the current value is 0, it must be in at least one collection
            ++at_least_one;
        } else if (current_count == 1)
        {
            // if the current value is 1, it must be in more than one collection
            ++many;
        }
        ++all;
        ++current_count;
    }
    int get_one() { return at_least_one; }
    int get_many() { return many; }
    int get_all() { return all; }
private:
    // maps record ID's to the number of collections in which they occur
    map<int, int> record_count;
    int at_least_one = 0, many = 0, all = 0;
};
bool collection_statistics(data_container& lib_cat)
{
    Collection_stats stats_helper;
    stats_helper = for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), stats_helper);

    int lib_size = lib_cat.library_title.size();
    cout << stats_helper.get_one() << " out of " << lib_size << " Records appear in at least one Collection\n";
    cout << stats_helper.get_many() << " out of " << lib_size << " Records appear in more than one Collection\n";
    cout << "Collections contain a total of " << stats_helper.get_all() << " Records\n";
    return false;
}
bool combine_collections(data_container& lib_cat)
{
    Collection first = *read_name_get_iter(lib_cat);
    Collection second = *read_name_get_iter(lib_cat);
    string new_name;
    cin >> new_name;
    Collection result(new_name, first);
    result += second;
    insert_collection(lib_cat, move(result));
    cout << "Collections " << first.get_name() << " and " << second.get_name() << " combined into new collection " << new_name << "\n";
    return false;
}

bool modify_rating(data_container& lib_cat)
{
    Record *record_ptr = *read_id_get_iter(lib_cat);
    int rating = integer_read();
    record_ptr->set_rating(rating);
    cout << "Rating for record " << record_ptr->get_ID() << " changed to " << rating << "\n";
    return false;
}
bool modify_title(data_container& lib_cat)
{
    auto record_iter = read_id_get_iter(lib_cat);
    Record *record_ptr = *record_iter;

    // make sure the new title is not already in the library
    string title = title_read(cin);
    check_title_in_library(lib_cat, title);

    // remove the record from all collections and remember what collections it is in
    list<Collection*> collections_with_record;
    for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), [&collections_with_record, record_ptr](Collection& collection)
        { if (collection.is_member_present(record_ptr)) { collection.remove_member(record_ptr); collections_with_record.push_back(&collection); }});

    // remove the record from the library
    lib_cat.library_id.erase(record_iter);
    assert(binary_search(lib_cat.library_title.begin(), lib_cat.library_title.end(), record_ptr, Title_compare()));
    lib_cat.library_title.erase(lib_title_lower_bound(lib_cat, record_ptr));

    // change the record's title and add it back into the library
    string old_title = record_ptr->get_title();
    record_ptr->set_title(title);
    insert_record(lib_cat, record_ptr);

    // add the record back into all the collections it was in
    for_each(collections_with_record.begin(), collections_with_record.end(), [record_ptr](Collection* collection) { collection->add_member(record_ptr); });

    cout << "Title for record " << record_ptr->get_ID() << " changed to " << title << "\n";
    return false;
}

bool add_record(data_container& lib_cat)
{
    string medium, title;
    cin >> medium;
    title = title_read(cin);
    check_title_in_library(lib_cat, title);
    Record *record = insert_record(lib_cat, new Record(medium, title));
    cout << "Record " << record->get_ID() << " added\n";
    return false;
}
bool add_collection(data_container& lib_cat)
{
    string name;
    cin >> name;
    insert_collection(lib_cat, Collection(name));
    cout << "Collection " << name << " added\n";
    return false;
}
bool add_member(data_container& lib_cat)
{
    Collection& collection = *read_name_get_iter(lib_cat);
    Record *record_ptr = *read_id_get_iter(lib_cat);
    collection.add_member(record_ptr);
    cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " added\n";
    return false;
}

bool delete_record(data_container& lib_cat)
{
    auto record_iter = read_title_get_iter(lib_cat);
    if (find_if(lib_cat.catalog.begin(), lib_cat.catalog.end(), bind(&Collection::is_member_present, placeholders::_1, *record_iter)) != lib_cat.catalog.end())
    {
        throw ErrorNoClear("Cannot delete a record that is a member of a collection!");
    }
    Record *record_ptr = *record_iter;
    lib_cat.library_title.erase(record_iter);
    assert(binary_search(lib_cat.library_id.begin(), lib_cat.library_id.end(), record_ptr, ID_compare()));
    lib_cat.library_id.erase(lib_id_lower_bound(lib_cat, record_ptr));
    cout << "Record " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
    delete record_ptr;
    return false;
}
bool delete_collection(data_container& lib_cat)
{
    auto collection_iter = read_name_get_iter(lib_cat);
    Collection& collection = *collection_iter;
    string name = collection.get_name();
    lib_cat.catalog.erase(collection_iter);
    cout << "Collection " << name << " deleted\n";
    return false;
}
bool delete_member(data_container& lib_cat)
{
    Collection& collection = *read_name_get_iter(lib_cat);
    Record *record_ptr = *read_id_get_iter(lib_cat);
    collection.remove_member(record_ptr);
    cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
    return false;
}

bool clear_library(data_container& lib_cat)
{
    if (find_if(lib_cat.catalog.begin(), lib_cat.catalog.end(), [](Collection c){return !c.empty();}) != lib_cat.catalog.end())
    {
        throw Error("Cannot clear all records unless all collections are empty!");
    }
    Record::reset_ID_counter();
    clear_library_data(lib_cat);
    cout << "All records deleted\n";
    return false;
}
bool clear_catalog(data_container& lib_cat)
{
    lib_cat.catalog.clear();
    cout << "All collections deleted\n";
    return false;
}
bool clear_all(data_container& lib_cat)
{
    Record::reset_ID_counter();
    clear_library_data(lib_cat);
    lib_cat.catalog.clear();
    cout << "All data deleted\n";
    return false;
}

bool save_all(data_container& lib_cat)
{
    string filename;
    cin >> filename;
    ofstream file(filename.c_str());
    if (!file)
    {
        throw Error(FILE_OPEN_FAIL_MSG);
    }
    file << lib_cat.library_title.size() << "\n";
    for_each(lib_cat.library_title.begin(), lib_cat.library_title.end(), bind(&Record::save, placeholders::_1, ref(file)));
    file << lib_cat.catalog.size() << "\n";
    for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), bind(&Collection::save, placeholders::_1, ref(file)));
    cout << "Data saved\n";
    return false;
}

bool restore_all(data_container& lib_cat)
{
    string filename;
    cin >> filename;
    ifstream file(filename.c_str());
    if (!file)
    {
        throw Error(FILE_OPEN_FAIL_MSG);
    }
    int num_records;
    if (!(file >> num_records))
    {
        throw Error(FILE_ERROR_MSG);
    }
    data_container new_lib_cat;
    try
    {
        Record::save_ID_counter();
        Record::reset_ID_counter();
        for (int i = 0; i < num_records; i++)
        {
            insert_record(new_lib_cat, new Record(file));
        }
        int num_collections
        if (!(file >> num_collections))
        {
            throw Error(FILE_ERROR_MSG);

        }
        for (int i = 0; i < num_collections; i++)
        {
            insert_collection(new_lib_cat, Collection(file, new_lib_cat.library_title));
        }
        lib_cat.catalog.clear();
        clear_library_data(lib_cat);
        lib_cat = new_lib_cat;
        cout << "Data loaded\n";
    }
    catch (Error& e)
    {
        new_lib_cat.catalog.clear();
        clear_library_data(new_lib_cat);
        Record::restore_ID_counter();
        throw Error(FILE_ERROR_MSG);
    }
    return false;
}

bool quit(data_container& lib_cat)
{
    clear_all(lib_cat);
    cout << "Done\n";
    return true;
}