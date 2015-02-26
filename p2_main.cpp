#include <iostream>
#include <fstream>
#include <limits>
#include <istream>
#include <cctype>
#include <string>
#include <algorithm>

#include <vector>
#include <set>
#include <map>
#include <list>

#include "Record.h"
#include "Collection.h"
#include "Utility.h"

using namespace std;

const char * TITLE_ALREADY_FOUND_MSG = "Library already has a record with this title!";
const char * FILE_OPEN_FAIL_MSG = "Could not open file!";

/* data types */

struct data_container {
    set<Collection> catalog;
    vector<Record*> library_title;
    vector<Record*> library_id;
};

typedef bool (*data_container_func)(data_container&);

struct record_id_comp {
    bool operator() (const Record *lhs, const Record *rhs) const { return lhs->get_ID() < rhs->get_ID(); }
};

/* lib cat helper functions dec */

vector<Record*>::Iterator read_title_get_iter(data_container& lib_cat);
vector<Record*>::Iterator read_id_get_iter(data_container& lib_cat);
vector<Collection>::Iterator read_name_get_iter(data_container& lib_cat);

bool check_title_in_library(data_container& lib_cat, string title);

Record* insert_record(data_container& lib_cat, Record* record);
void insert_collection(data_container& lib_cat, Collection&& collection);

void clear_library_data(data_container& lib_cat);

/* other functions dec */

string title_read(istream &is);
string parse_title(string& title_string);
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
            if (!(cin >> action >> object) || function_map.find(action + object) == function_map.end())
            {
                throw Error("Unrecognized command!");
            }
            if (function_map[action + object](lib_cat))
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

vector<Record*>::Iterator read_title_get_iter(data_container& lib_cat)
{
    string title = title_read(cin);
    Record temp_record(title);
    auto record_iter = lower_bound(lib_cat.library_title.begin(), lib_cat.library_title.end(), &temp_record);
    if (*record_iter != temp_record || record_iter == lib_cat.library_title.end())
    {
        throw Error("No record with that title!");
    }
    return record_iter;
}

vector<Record*>::Iterator read_id_get_iter(data_container& lib_cat)
{
    int id = integer_read();
    Record temp_record(id);
    auto record_iter = lower_bound(lib_cat.library_id.begin(), lib_cat.library_id.end(), &temp_record);
    if (*record_iter != temp_record || record_iter == lib_cat.library_id.end())
    {
        throw Error("No record with that ID!");
    }
    return record_iter;
}

vector<Collection>::Iterator read_name_get_iter(data_container& lib_cat)
{
    string name;
    cin >> name;
    Collection temp_collection(name);
    auto collection_iter = lower_bound(lib_cat.catalog.begin(), lib_cat.catalog.end(), temp_collection);
    if (*collection_iter != temp_collection || collection_iter == lib_cat.catalog.end())
    {
        throw Error("No collection with that name!");
    }
    return collection_iter;
}

bool check_title_in_library(data_container& lib_cat, string title)
{
    Record temp_record(title);
    auto title_check = lower_bound(lib_cat.library_title.begin(), lib_cat.library_title.end(), &temp_record);
    if (title_check != lib_cat.library_title.end())
    {
        throw Error(TITLE_ALREADY_FOUND_MSG);
    }
}

Record* insert_record(data_container& lib_cat, Record* record)
{
    try
    {
        lib_cat.library_title.insert(lower_bound(lib_cat.library_title.begin(), lib_cat.library_title.end(), record, Less_than_ptr<Record*>), record);
        lib_cat.library_id.insert(lower_bound(lib_cat.library_id.begin(), lib_cat.library_id.end(), record, record_id_comp), record);
    } catch (...)
    {
        delete record;
        throw;
    }
    return record;
}

void insert_collection(data_container& lib_cat, Collection&& collection)
{
    auto collection_iter = lower_bound(lib_cat.catalog.begin(), lib_cat.catalog.end(), collection);
    if (collection_iter != lib_cat.catalog.end())
    {
        throw Error("Catalog already has a collection with this name!");
    }
    lib_cat.catalog.insert(collection_iter, move(collection));
}

void clear_library_data(data_container& lib_cat)
{
    for_each(lib_cat.library_title.begin(), lib_cat.library_title.end(), [](Record* record){delete record;});
    lib_cat.library_title.clear();
    lib_cat.library_id.clear();
}

/* other functions impl */

string title_read(istream &is)
{
    string title;
    getline(is, title);
    title = parse_title(title);
    if (title.size() == 0)
    {
        throw ErrorNoClear("Could not read a title!");
    }
    return title;
}

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
            if (!remove_whitespace)
            {
                title.push_back(' ');
            }
            remove_whitespace = true;
        }
    }
    string title;
    bool remove_whitespace = true;
};
string parse_title(string& original)
{
    title_parser title_helper;
    for_each(original.begin(), original.end(), title_helper);
    string title = move(title_helper.title);
    if (isspace(title[title.size() - 1]))
    {
        title.pop_back();
    }
    return title;
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

/* main lib cat functions impl */

bool find_record(data_container& lib_cat)
{
    auto record_ptr = *read_title_get_iter(lib_cat.library_title);
    cout << *record_ptr << "\n";
    return false;
}
void search_record_for_string(string key, list<Record*>& records_with_string, Record* record)
{
    if (search(record->get_title().begin(), record->get_title().end(), key.begin(), key.end(), [](char a, char b){return tolower(a) == tolower(b);}) != record->get_title().end())
    {
        records_with_string.push_back(record);
    }
}
bool find_string(data_container& lib_cat)
{
    string key;
    cin >> key;
    list<Record*> records_with_string;
    for_each(lib_cat.library_title.begin(), lib_cat.library_title.end(), bind(search_record_for_string, key, records_with_string, _1));
    if (records_with_string.size() == 0)
    {
        throw Error("No records contain that string!");
    }
    ostream_iterator<Record*> out_it(cout, "\n");
    copy(records_with_string.begin(), records_with_string.end(), out_it);
    return false;
}

bool rating_sort(const Record* a, const Record* b)
{
    if (a->get_rating() == b->get_rating())
        return a < b;
    else return a->get_rating() > b->get_rating();
}
bool list_ratings(data_container& lib_cat)
{
    data_container temp_lib_cat;
    copy(lib_cat.library_title.begin(), lib_cat.library_title.end(), temp_lib_cat.library_title.begin());
    sort(temp_lib_cat.library_title.begin, temp_lib_cat.library_title.end(), rating_sort);
    print_library(temp_lib_cat);
    return false;
}

bool print_record(data_container& lib_cat)
{
    Record *record_ptr = *read_id_get_iter(lib_cat.library_id);
    cout << *record_ptr << "\n";
    return false;
}
bool print_collection(data_container& lib_cat)
{
    Collection collection = *read_name_get_iter(lib_cat.catalog);
    cout << collection;
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

struct Collection_stats {
public:
    void operator()(Collection& collection)
    {
        for_each(collection.get_elements().begin(), collection.get_elements().end(), [](Record* record)
        {
            if (record_count.find(record->get_ID()) == record_count.end())
            {
                record_count[record->get_ID()] = 0;
            }
            int& current_count = record_count[record->get_ID()];
            if (current_count == 0)
            {
                ++at_least_one;
            } else if (current_count == 1)
            {
                ++many;
            }
            ++all;
        });
    }
    map<int, int> record_count;
    int at_least_one = 0, many = 0, all = 0;
};
bool collection_statistics(data_container& lib_cat)
{
    Collection_stats collection_stats;
    for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), collection_stats);

    int lib_size = lib_cat.library_title.size();
    cout << collection_stats.at_least_one << " out of " << lib_size << " Records appear in at least one Collection\n";
    cout << collection_stats.many << " out of " << lib_size << " Records appear in more than one Collection\n";
    cout << "Collections contain a total of " << collection_stats.all << " Records\n";
    return false;
}
bool combine_collections(data_container& lib_cat)
{
    Collection first = read_name_get_iter(lib_cat);
    Collection second = read_name_get_iter(lib_cat);
    string new_name;
    cin >> new_name;
    Collection result(new_name, first);
    result += second;
    cout << "Collections " << first.get_name() << " and " << second.get_name() << " combined into new collection " << new_name << "\n";
}

bool modify_rating(data_container& lib_cat)
{
    Record *record_ptr = *read_id_get_iter(lib_cat.library_id);
    int rating = integer_read();
    record_ptr->set_rating(rating);
    cout << "Rating for record " << record_ptr->get_ID() << " changed to " << rating << "\n";
    return false;
}
void reorder_record_in_catalog(Collection& collection, string old_title, Record* record)
{
    if (collection.is_member_present(old_title))
    {
        collection.remove_member(old_title);
        collection.add_member(record);
    }
}
bool modify_title(data_container& lib_cat)
{
    auto record_iter = read_id_get_iter(lib_cat.library_id);
    Record *record_ptr = *record_iter;

    string title = title_read(cin);
    check_title_in_library(lib_cat, title);

    lib_cat.library_id.erase(record_iter);
    assert(binary_search(lib_cat.library_title.begin(), lib_cat.library_title.end(), record_ptr, Less_than_ptr<Record*>));
    lib_cat.library_title.erase(lower_bound(lib_cat.library_title.begin(), lib_cat.library_title.end(), record_ptr, Less_than_ptr<Record*>));

    string old_title = record_ptr->get_title();
    record_ptr->set_title(title);
    insert_record(lib_cat, record_ptr);

    for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), bind(reorder_record_in_catalog, _1, old_title, record_ptr));
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
    Collection collection = *read_name_get_iter(lib_cat.catalog);
    Record *record_ptr = *read_id_get_iter(lib_cat.library_id);
    collection.add_member(record_ptr);
    cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " added\n";
    return false;
}

bool delete_record(data_container& lib_cat)
{
    auto record_iter = read_title_get_iter(library_title);
    if (find_if(lib_cat.catalog.begin(), lib_cat.catalog.end(), [&record_iter](Collection c){return c.is_member_present(*record_iter);}) != lib_cat.catalog.end())
    {
        throw Error("Cannot delete a record that is a member of a collection!");
    }
    Record *record_ptr = *record_iter;
    lib_cat.library_title.erase(record_iter);
    assert(binary_search(lib_cat.library_id.begin(), lib_cat.library_id.end(), record_ptr, record_id_comp));
    lib_cat.library_id.erase(lower_bound(lib_cat.library_id.begin(), lib_cat.library_id.end(), record_ptr, record_id_comp));
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
    Collection collection = *read_name_get_iter(lib_cat.catalog);
    Record *record_ptr = *read_id_get_iter(library_id);
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
    // the one range for
    for (auto&& record : lib_cat.library_title)
    {
        record->save(file);
    }
    file << lib_cat.catalog.size() << "\n";
    for_each(lib_cat.catalog.begin(), lib_cat.catalog.end(), [&file](Collection collection) { collection.save(file); });
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
    int num;
    if (!(file >> num))
    {
        throw Error(FILE_ERROR_MSG);
    }
    data_container new_lib_cat
    try
    {
        Record::save_ID_counter();
        Record::reset_ID_counter();
        for (int i = 0; i < num; i++)
        {
            insert_record(new_lib_cat, new Record(file));
        }
        if (!(file >> num))
        {
            throw Error(FILE_ERROR_MSG);

        }
        for (int i = 0; i < num; i++)
        {
            insert_collection(new_lib_cat, Collection(file, lib_cat.library_title));
            num--;
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
}