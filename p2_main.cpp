#include <iostream>
#include <fstream>
#include <limits>
#include <istream>
#include <cctype>
#include <string>
#include <vector>
#include "Record.h"
#include "Collection.h"
#include "Utility.h"

using namespace std;

struct record_id_comp {
    bool operator() (const Record *lhs, const Record *rhs) const { return lhs->get_ID() < rhs->get_ID(); }
};

void throw_unrecognized_command();

Record* read_title_get_record(vector<Record*, Less_than_ptr<Record*>>& library_title);
vector<Record*, Less_than_ptr<Record*>>::Iterator read_title_get_iter(vector<Record*, Less_than_ptr<Record*>>& library_title);

Record* read_id_get_record(vector<Record*, record_id_comp>& library_id);
vector<Record*, record_id_comp>::Iterator read_id_get_iter(vector<Record*, record_id_comp>& library_id);

Collection* read_name_get_collection(vector<Collection*, Less_than_ptr<Collection*>>& catalog);
vector<Collection*, Less_than_ptr<Collection*>>::Iterator read_name_get_iter(vector<Collection*, Less_than_ptr<Collection*>>& catalog);

void clear_libraries(vector<Record*, Less_than_ptr<Record*>>& library_title, vector<Record*, record_id_comp>& library_id);
void clear_catalog(vector<Collection*, Less_than_ptr<Collection*>>& catalog);

bool check_collection_not_empty(Collection *collection);
bool check_record_in_collection(Collection *collection, Record *record);

void print_record(Record* record);
void print_collection(Collection* collection);

string title_read(istream &is);
string parse_title(string& title_string);

int main()
{
    vector<Collection*, Less_than_ptr<Collection*>> catalog;
    vector<Record*, Less_than_ptr<Record*>> library_title;
    vector<Record*, record_id_comp> library_id;
    while (true)
    {
        try
        {
            char action, object;
            cout << "\nEnter command: ";
            if (!(cin >> action >> object))
            {
                throw_unrecognized_command();
            }
            switch (action)
            {
                case 'f': /* find (records only) */
                {
                    switch (object)
                    {
                        case 'r': /* find record */
                        {
                            auto record_ptr = read_title_get_record(library_title);
                            cout << *record_ptr << "\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'p': /* print */
                {
                    switch (object)
                    {
                        case 'r': /* print record */
                        {
                            Record *record_ptr = read_id_get_record(library_id);
                            cout << *record_ptr << "\n";
                            break;
                        }
                        case 'c': /* print collection */
                        {
                            Collection *collection_ptr = read_name_get_collection(catalog);
                            cout << *collection_ptr << "\n";
                            break;
                        }
                        case 'L': /* print library */
                        {
                            if (library_title.empty())
                            {
                                cout << "Library is empty\n";
                            }
                            else
                            {
                                cout << "Library contains " << library_title.size() << " records:";
                                apply(library_title.begin(), library_title.end(), print_record);
                                cout << "\n";
                            }
                            break;
                        }
                        case 'C': /* print catalog */
                        {
                            if (catalog.empty())
                            {
                                cout << "Catalog is empty\n";
                            }
                            else
                            {
                                cout << "Catalog contains " << catalog.size() << " collections:";
                                apply(catalog.begin(), catalog.end(), print_collection);
                                cout << "\n";
                            }
                            break;
                        }
                        case 'a': /* print memory allocations */
                        {
                            cout << "Memory allocations:\n";
                            cout << "Records: " << library_title.size() << "\n";
                            cout << "Collections: " << catalog.size() << "\n";
                            cout << "Lists: " << g_vector_count << "\n";
                            cout << "List Nodes: " << g_vector_Node_count << "\n";
                            cout << "strings: " << string::get_number() << " with " << string::get_total_allocation() << " bytes total\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'm': /* modify (rating only) */
                {
                    switch (object)
                    {
                        case 'r': /* modify rating of a record */
                        {
                            Record *record_ptr = read_id_get_record(library_id);
                            int rating = integer_read();
                            record_ptr->set_rating(rating);
                            cout << "Rating for record " << record_ptr->get_ID() << " changed to " << rating << "\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'a': /* add */
                {
                    switch (object)
                    {
                        case 'r': /* add record */
                        {
                            string medium, title;
                            cin >> medium;
                            title = title_read(cin);
                            Record temp_record(title);
                            if (library_title.find(&temp_record) != library_title.end())
                            {
                                throw Error("Library already has a record with this title!");
                            }
                            Record *record = new Record(medium, title);
                            library_id.insert(record);
                            library_title.insert(record);
                            cout << "Record " << record->get_ID() << " added\n";
                            break;
                        }
                        case 'c': /* add collection */
                        {
                            string name;
                            cin >> name;
                            Collection temp_collection(name);
                            if (catalog.find(&temp_collection) != catalog.end())
                            {
                                throw Error("Catalog already has a collection with this name!");
                            }
                            catalog.insert(new Collection(name));
                            cout << "Collection " << name << " added\n";
                            break;
                        }
                        case 'm': /* add record to collection */
                        {
                            Collection *collection_ptr = read_name_get_collection(catalog);
                            Record *record_ptr = read_id_get_record(library_id);
                            collection_ptr->add_member(record_ptr);
                            cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " added\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'd': /* delete */
                {
                    switch (object)
                    {
                        case 'r': /* delete record */
                        {
                            auto record_iter = read_title_get_iter(library_title);
                            if (apply_if_arg(catalog.begin(), catalog.end(), check_record_in_collection, *record_iter))
                            {
                                throw Error("Cannot delete a record that is a member of a collection!");
                            }
                            Record *record_ptr = *record_iter;
                            library_title.erase(record_iter);
                            library_id.erase(library_id.find(record_ptr));
                            cout << "Record " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
                            delete record_ptr;
                            break;
                        }
                        case 'c': /* delete collection */
                        {
                            auto collection_iter = read_name_get_iter(catalog);
                            Collection *collection_ptr = *collection_iter;
                            catalog.erase(collection_iter);
                            cout << "Collection " << collection_ptr->get_name() << " deleted\n";
                            delete collection_ptr;
                            break;
                        }
                        case 'm': /* delete record from collection */
                        {
                            Collection *collection_ptr = read_name_get_collection(catalog);
                            Record *record_ptr = read_id_get_record(library_id);
                            collection_ptr->remove_member(record_ptr);
                            cout << "Member " << record_ptr->get_ID() << " " << record_ptr->get_title() << " deleted\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'c': /* clear */
                {
                    switch (object)
                    {
                        case 'L': /* clear library */
                        {
                            if (apply_if(catalog.begin(), catalog.end(), check_collection_not_empty))
                            {
                                throw Error("Cannot clear all records unless all collections are empty!");
                            }
                            Record::reset_ID_counter();
                            clear_libraries(library_title, library_id);
                            cout << "All records deleted\n";
                            break;
                        }
                        case 'C': /* clear catalog */
                        {
                            clear_catalog(catalog);
                            cout << "All collections deleted\n";
                            break;
                        }
                        case 'A': /* clear all */
                        {
                            Record::reset_ID_counter();
                            clear_libraries(library_title, library_id);
                            clear_catalog(catalog);
                            cout << "All data deleted\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 's': /* save */
                {
                    switch (object)
                    {
                        case 'A': /* save all */
                        {
                            string filename;
                            cin >> filename;
                            ofstream file(filename.c_str());
                            if (!file)
                            {
                                throw Error("Could not open file!");
                            }
                            file << library_title.size() << "\n";
                            for (auto&& record : library_title)
                            {
                                record->save(file);
                            }
                            file << catalog.size() << "\n";
                            for (auto catalog_iter = catalog.begin(); catalog_iter != catalog.end(); ++catalog_iter)
                            {
                                (*catalog_iter)->save(file);
                            }
                            cout << "Data saved\n";
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'r': /* restore */
                {
                    switch (object)
                    {
                        case 'A': /* restore all */
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
                            vector<Collection*, Less_than_ptr<Collection*>> new_catalog;
                            vector<Record*, Less_than_ptr<Record*>> new_library_title;
                            vector<Record*, record_id_comp> new_library_id;
                            try
                            {
                                Record::save_ID_counter();
                                Record::reset_ID_counter();
                                while (num > 0)
                                {
                                    Record *record_ptr = new Record(file);
                                    new_library_title.insert(record_ptr);
                                    new_library_id.insert(record_ptr);
                                    num--;
                                }
                                if (!(file >> num))
                                {
                                    throw_file_error();
                                }
                                while (num > 0)
                                {
                                    Collection *collection_ptr = new Collection(file, new_library_title);
                                    new_catalog.insert(collection_ptr);
                                    num--;
                                }
                                clear_libraries(library_title, library_id);
                                clear_catalog(catalog);
                                library_title = new_library_title;
                                library_id = new_library_id;
                                catalog = new_catalog;
                                cout << "Data loaded\n";
                            }
                            catch (Error& e)
                            {
                                clear_catalog(new_catalog);
                                clear_libraries(new_library_title, new_library_id);
                                Record::restore_ID_counter();
                                throw_file_error();
                            }
                            break;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                case 'q': /* quit */
                {
                    switch (object)
                    {
                        case 'q': /* quit */
                        {
                            clear_libraries(library_title, library_id);
                            clear_catalog(catalog);
                            cout << "All data deleted\nDone\n";
                            return 0;
                        }
                        default:
                        {
                            throw_unrecognized_command();
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    throw_unrecognized_command();
                    break;
                }
            }
        } catch (Error& e)
        {
            cout << e.msg << "\n";
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } catch (string_exception& e)
        {
            cerr << e.msg << "\n";
            return 1;
        }
    }
}

void throw_unrecognized_command()
{
    throw Error("Unrecognized command!");
}

Record* read_title_get_record(vector<Record*, Less_than_ptr<Record*>>& library_title)
{
    return *read_title_get_iter(library_title);
}

vector<Record*, Less_than_ptr<Record*>>::Iterator read_title_get_iter(vector<Record*, Less_than_ptr<Record*>>& library_title)
{
    string title = title_read(cin);
    Record temp_record(title);
    auto record_iter = library_title.find(&temp_record);
    if (record_iter == library_title.end())
    {
        throw Error("No record with that title!");
    }
    return record_iter;
}

Record* read_id_get_record(vector<Record*, record_id_comp>& library_id)
{
    return *read_id_get_iter(library_id);
}

vector<Record*, record_id_comp>::Iterator read_id_get_iter(vector<Record*, record_id_comp>& library_id)
{
    int id = integer_read();
    Record temp_record(id);
    auto record_iter = library_id.find(&temp_record);
    if (record_iter == library_id.end())
    {
        throw Error("No record with that ID!");
    }
    return record_iter;
}

Collection* read_name_get_collection(vector<Collection*, Less_than_ptr<Collection*>>& catalog)
{
    return *read_name_get_iter(catalog);
}

vector<Collection*, Less_than_ptr<Collection*>>::Iterator read_name_get_iter(vector<Collection*, Less_than_ptr<Collection*>>& catalog)
{
    string name;
    cin >> name;
    Collection temp_collection(name);
    auto collection_iter = catalog.find(&temp_collection);
    if (collection_iter == catalog.end())
    {
        throw Error("No collection with that name!");
    }
    return collection_iter;
}

void clear_libraries(vector<Record*, Less_than_ptr<Record*>>& library_title, vector<Record*, record_id_comp>& library_id)
{
    auto title_iter = library_title.begin();
    while (title_iter != library_title.end())
    {
        delete *title_iter;
        ++title_iter;
    }
    library_title.clear();
    library_id.clear();
}

void clear_catalog(vector<Collection*, Less_than_ptr<Collection*>>& catalog)
{
    auto catalog_iter = catalog.begin();
    while (catalog_iter != catalog.end())
    {
        delete *catalog_iter;
        ++catalog_iter;
    }
    catalog.clear();
}

bool check_collection_not_empty(Collection *collection)
{
    return !(collection->empty());
}

bool check_record_in_collection(Collection *collection, Record *record)
{
    return collection->is_member_present(record);
}

void print_record(Record* record)
{
    cout << "\n" << *record;
}

void print_collection(Collection* collection)
{
    cout << "\n" << *collection;
}

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