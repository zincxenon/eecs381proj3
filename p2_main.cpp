#include <iostream>
#include <fstream>
#include <limits>
#include <istream>
#include <cctype>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <unordered_map>
#include "Record.h"
#include "Collection.h"
#include "Utility.h"

using namespace std;

/* data types */

struct data_container {
    set<Collection> catalog;
    vector<Record*> library_title;
    vector<Record*> library_id;
};

typedef bool (*function)(data_container&);

struct record_id_comp {
    bool operator() (const Record *lhs, const Record *rhs) const { return lhs->get_ID() < rhs->get_ID(); }
};

/* lib cat helper functions dec */

Record* read_title_get_record(data_container& lib_cat);
vector<Record*>::Iterator read_title_get_iter(data_container& lib_cat);

Record* read_id_get_record(data_container& lib_cat);
vector<Record*>::Iterator read_id_get_iter(data_container& lib_cat);

Collection read_name_get_collection(data_container& lib_cat);
set<Collection>::Iterator read_name_get_iter(data_container& lib_cat);

void clear_libraries(data_container& lib_cat);
void clear_catalog(data_container& lib_cat);

/* other functions dec */

string title_read(istream &is);
string parse_title(string& title_string);

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
    unordered_map<string, function> function_map {
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
            if (!(cin >> action >> object) || function_map.find(action + object) == function_map.end())
            {
                throw Error("Unrecognized command!");
            }
            if (function_map[action + object](lib_cat))
            {
                return 0;
            }
        } catch (Error& e)
        {
            cout << e.msg << "\n";
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } catch (...)
        {
            // print error message
            return 1;
        }
    }
}

/* lib cat helper functions impl */

Record* read_title_get_record(data_container& lib_cat)
{
    return *read_title_get_iter(lib_cat);
}

vector<Record*>::Iterator read_title_get_iter(data_container& lib_cat)
{
    string title = title_read(cin);
    Record temp_record(title);
    auto record_iter = find(library_title.begin(), library_title.end(), &temp_record);
    if (record_iter == library_title.end())
    {
        throw Error("No record with that title!");
    }
    return record_iter;
}

Record* read_id_get_record(data_container& lib_cat)
{
    return *read_id_get_iter(lib_cat);
}

vector<Record*>::Iterator read_id_get_iter(data_container& lib_cat)
{
    int id = integer_read();
    Record temp_record(id);
    auto record_iter = find(lib_cat.library_id.begin(), lib_cat.library_id.end(), &temp_record);
    if (record_iter == lib_cat.library_id.end())
    {
        throw Error("No record with that ID!");
    }
    return record_iter;
}

Collection read_name_get_collection(data_container& lib_cat)
{
    return *read_name_get_iter(lib_cat);
}

set<Collection>::Iterator read_name_get_iter(data_container& lib_cat)
{
    string name;
    cin >> name;
    Collection temp_collection(name);
    auto collection_iter = lib_cat.catalog.find(&temp_collection);
    if (collection_iter == lib_cat.catalog.end())
    {
        throw Error("No collection with that name!");
    }
    return collection_iter;
}

void clear_libraries(data_container& lib_cat)
{
    auto title_iter = lib_cat.library_title.begin();
    while (title_iter != lib_cat.library_title.end())
    {
        delete *title_iter;
        ++title_iter;
    }
    lib_cat.library_title.clear();
    lib_cat.library_id.clear();
}

void clear_catalog(data_container& lib_cat)
{
    auto catalog_iter = lib_cat.catalog.begin();
    while (catalog_iter != lib_cat.catalog.end())
    {
        delete *catalog_iter;
        ++catalog_iter;
    }
    lib_cat.catalog.clear();
}

/* other functions impl */

string title_read(istream &is)
{
    string title;
    getline(is, title);
    title = parse_title(title);
    if (title.size() == 0)
    {
        throw Error("Could not read a title!");
    }
    return title;
}

string parse_title(string& title_string)
{
    string title(title_string);
    for (int i = 0; i < title.size(); i++)
    {
        if (!isspace(title[i]))
        {
            title.remove(0, i);
            break;
        }
    }
    for (int i = 0; i < title.size(); i++)
    {
        if (isspace(title[i]))
        {
            int j;
            for (j = i + 1; j < title.size() && isspace(title[j]); j++);
            if (j != i + 1)
            {
                title.remove(i + 1, j - (i + 1));
            }
        }
    }
    if (title.size() > 0 && isspace(title[title.size() - 1]))
    {
        title.remove(title.size() - 1, 1);
    }
    return title;
}

/* main lib cat functions impl */

bool find_record(data_container& lib_cat)
{
    auto record_ptr = read_title_get_record(lib_cat.library_title);
    cout << *record_ptr << "\n";
    return false;
}
bool find_string(data_container& lib_cat);

bool list_ratings(data_container& lib_cat);

bool print_record(data_container& lib_cat)
{
    Record *record_ptr = read_id_get_record(lib_cat.library_id);
    cout << *record_ptr << "\n";
    return false;
}
bool print_collection(data_container& lib_cat)
{
    Collection collection = read_name_get_collection(lib_cat.catalog);
    cout << collection << "\n";
    return false;
}
bool print_library(data_container& lib_cat)
{
    if (lib_cat.library_title.empty())
    {
        cout << "Library is empty\n";
    }
    else
    {
        cout << "Library contains " << lib_cat.library_title.size() << " records:";
        //apply(lib_cat.library_title.begin(), lib_cat.library_title.end(), print_record);
        cout << "\n";
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
        cout << "Catalog contains " << lib_cat.catalog.size() << " collections:";
        //apply(lib_cat.catalog.begin(), lib_cat.catalog.end(), print_collection);
        cout << "\n";
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

bool collection_statistics(data_container& lib_cat);
bool combine_collections(data_container& lib_cat);

bool modify_rating(data_container& lib_cat)
{
    Record *record_ptr = read_id_get_record(lib_cat.library_id);
    int rating = integer_read();
    record_ptr->set_rating(rating);
    cout << "Rating for record " << record_ptr->get_ID() << " changed to " << rating << "\n";
    return false;
}
bool modify_title(data_container& lib_cat);

bool add_record(data_container& lib_cat)
{
    string medium, title;
    cin >> medium;
    title = title_read(cin);
    Record temp_record(title);
    if (find(lib_cat.library_title.begin(), lib_cat.library_title.end(), &temp_record) != lib_cat.library_title.end())
    {
        throw Error("Library already has a record with this title!");
    }
    // try catch here
    Record *record = new Record(medium, title);
    lib_cat.library_id.insert(record);
    lib_cat.library_title.insert(record);
    cout << "Record " << record->get_ID() << " added\n";
    return false;
}
bool add_collection(data_container& lib_cat)
{
    string name;
    cin >> name;
    Collection collection(name);
    if (lib_cat.catalog.find(temp_collection) != lib_cat.catalog.end())
    {
        throw Error("Catalog already has a collection with this name!");
    }
    lib_cat.catalog.insert(collection);
    cout << "Collection " << name << " added\n";
    return false;
}
bool add_member(data_container& lib_cat)
{
    Collection collection = read_name_get_collection(lib_cat.catalog);
    Record *record_ptr = read_id_get_record(lib_cat.library_id);
    collection.add_member(record_ptr);
    cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " added\n";
    return false;
}

bool delete_record(data_container& lib_cat)
{
    auto record_iter = read_title_get_iter(library_title);
    if (apply_if_arg(lib_cat.catalog.begin(), lib_cat.catalog.end(), check_record_in_collection, *record_iter))
    {
        throw Error("Cannot delete a record that is a member of a collection!");
    }
    Record *record_ptr = *record_iter;
    lib_cat.library_title.erase(record_iter);
    lib_cat.library_id.erase(find(lib_cat.library_id.begin(), lib_cat.library_id.end(), record_ptr));
    cout << "Record " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
    delete record_ptr;
    return false;
}
bool delete_collection(data_container& lib_cat)
{
    auto collection_iter = read_name_get_iter(lib_cat.catalog);
    Collection collection = *collection_iter;
    lib_cat.catalog.erase(collection_iter);
    cout << "Collection " << collection.get_name() << " deleted\n";
    delete collection_ptr;
    return false;
}
bool delete_member(data_container& lib_cat)
{
    Collection collection = read_name_get_collection(lib_cat.catalog);
    Record *record_ptr = read_id_get_record(library_id);
    collection.remove_member(record_ptr);
    cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
    return false;
}

bool clear_library(data_container& lib_cat)
{
    if (apply_if(lib_cat.catalog.begin(), lib_cat.catalog.end(), check_collection_not_empty))
    {
        throw Error("Cannot clear all records unless all collections are empty!");
    }
    Record::reset_ID_counter();
    clear_libraries(lib_cat);
    cout << "All records deleted\n";
    return false;
}
bool clear_catalog(data_container& lib_cat)
{
    clear_catalog(lib_cat);
    cout << "All collections deleted\n";
    return false;
}
bool clear_all(data_container& lib_cat)
{
    Record::reset_ID_counter();
    clear_libraries(lib_cat);
    clear_catalog(lib_cat.catalog);
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
        throw Error("Could not open file!");
    }
    file << lib_cat.library_title.size() << "\n";
    for (auto&& record : library_title)
    {
        record->save(file);
    }
    file << lib_cat.catalog.size() << "\n";
    for (auto catalog_iter = lib_cat.catalog.begin(); catalog_iter != lib_cat.catalog.end(); ++catalog_iter)
    {
        (*catalog_iter)->save(file);
    }
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
        throw Error("Could not open file!");
    }
    int num;
    file >> num;
    data_container new_lib_cat
    try
    {
        Record::save_ID_counter();
        Record::reset_ID_counter();
        while (num > 0)
        {
            Record *record_ptr = new Record(file);
            lib_cat.library_title.insert(record_ptr);
            lib_cat.library_id.insert(record_ptr);
            num--;
        }
        if (!(file >> num))
        {
            throw_file_error();
        }
        while (num > 0)
        {
            Collection collection(file, lib_cat.library_title);
            lib_cat.catalog.insert(collection_ptr);
            num--;
        }
        clear_libraries(lib_cat);
        clear_catalog(lib_cat);
        lib_cat = new_lib_cat;
        cout << "Data loaded\n";
    }
    catch (Error& e)
    {
        clear_catalog(new_lib_cat);
        clear_libraries(new_lib_cat);
        Record::restore_ID_counter();
        throw_file_error();
    }
    return false;
}

bool quit(data_container& lib_cat)
{
    clear_all(lib_cat);
    cout << "Done\n";
}