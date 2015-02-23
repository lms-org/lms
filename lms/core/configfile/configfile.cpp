#include <core/configfile.h>
#include <limits>
#include <sstream>
#include <fstream>
/**
 * Remove surrounding whitespace from a std::string.
 * @param s The string to be modified.
 * @param t The set of characters to delete from each end
 * of the string.
 * @return The same string passed in as a parameter reference.
 */
static std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
	s.erase(0, s.find_first_not_of(t));
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

ConfigFile::ConfigFile( const std::string &name) : 
	_name(name)
{
	std::fstream stream(name);
	_good = stream.good();
	while (stream.good() && !stream.bad()) {
		std::string row;
		std::getline(stream, row);
		file_entry entry = entry_from_row(row);
		_items.push_back(entry);
	}
	stream.close();
}

bool ConfigFile::flush() {
	bool changed = false;	
	for (auto it = _items.begin(); it != _items.end(); ++it) {
		if (it->changed) changed = true;
	}
	if (!changed) return false; 
	std::stringstream stream;

	for (auto it = _items.begin(); it != _items.end(); ++it) {
		stream << row_from_entry(*it);
		it->changed = false;
		if (it+1 != _items.end()) stream << "\n";
	}
	std::string tmp = stream.str();
	FILE* f=fopen(_name.c_str(), "w");
//	fprintf(f, "%s", stream.str().c_str());
	fprintf(f, "%s", tmp.c_str());
	fclose(f);
	return true;
}

std::string ConfigFile::get_str(const std::string &field) {
	mapping_t::iterator it = _mapping.lower_bound(field);
	if (it != _mapping.end())
		return _mapping.lower_bound(field)->second;
	else
		return "";
}

void ConfigFile::set_str (const std::string &field, const std::string &v) {
	insert_or_update(field, v);
}

template <>
std::string ConfigFile::get<std::string>(const std::string &field) {
	return get_str(field);
}

template <>
void ConfigFile::set<std::string>(const std::string &field, std::string v) {
	set_str(field, v);
}

ConfigFile::itemlist::iterator ConfigFile::select_element(const std::string &field) {
	for (itemlist::iterator it = _items.begin(); it != _items.end(); ++it) {
		if (it->name == field) 
			return it;
	}
	return _items.end();
}

ConfigFile::itemlist::iterator ConfigFile::select_element_offset(const std::string &field, int offset) {
	for (itemlist::iterator it = _items.begin(); it != _items.end(); ++it) {
		if (it->name == field) {
			it = it+offset; 
			if (it != _items.end() && it->name == field)
				return it;
			else 
				return _items.end();
		}
	}
	return _items.end();
}

ConfigFile::itemlist::iterator ConfigFile::select_element(const std::string &field, const std::string &) {
	for (itemlist::iterator it = _items.begin(); it != _items.end(); ++it) {
		if (it->name == field) {
			return it;
		}
	}
	return _items.end();	
}

void ConfigFile::insert_or_update(const std::string &field, const std::string &value) {
	mapping_t::iterator it = _mapping.lower_bound(field);
	if (it == _mapping.end()) {
		//It does not exist
		insert(field, value);
	} else {
		//It does exist
		update(select_element(field), value);
	}
}

int ConfigFile::matching (const std::string &needle, const std::string &haystack, direction *out_dir) {
	int matching = 0;
	size_t i;
	*out_dir = DIRECTION_FORWARD;
	for (i = 0; i < needle.size() && i < haystack.size(); i++) {
		if (std::tolower(needle[i]) == std::tolower(haystack[i]))
			matching ++;
		else {
//			if (needle[i] > haystack[i])
//				*out_dir = DIRECTION_FORWARD;
//			else
//				*out_dir = DIRECTION_BACKWARD;
			break;
		}
	}
		
	//Perfect match (needle could fit in haystack)
	if (i == needle.size()) {
		return std::numeric_limits<int>::max();
	}
	
//	printf("n: %s; h: %s; m:%i \n", needle.c_str(), haystack.c_str(), matching);

	return matching;
}

ConfigFile::itemlist::iterator ConfigFile::select_element_fuzzy(const std::string &field, direction* out_dir) {
	direction best_dir = DIRECTION_FORWARD;
	itemlist::iterator best_iterator = _items.end();
	int best_matching = 0;
		
	for (itemlist::iterator it = _items.begin(); it != _items.end(); ++it) {
		direction dir;
		int match = matching(field, it->name, &dir);
		if (match > best_matching) {
			best_dir = dir;
			best_iterator = it;
			best_matching = match;
		}
	}
//	printf("\033[034mn: %s, h: %s, m: %i\033[0m\n", field.c_str(), best_iterator->name.c_str(), best_matching);
	if (best_iterator != _items.end()) {
		//Found a matching
		*out_dir = best_dir;
		return best_iterator;
	} else {
		*out_dir = DIRECTION_FORWARD;
		return _items.end();
	}
}

void ConfigFile::insert(const std::string &field, const std::string &value) {
	file_entry entry;
	entry.name = field;
	entry.value = value;
	entry.changed = true;
	entry.repeatable = false;
	entry.comment = false;
	
	insert(entry);
}

void ConfigFile::insert(const file_entry &entry) {
	if (_items.empty()) {
		_items.insert(_items.end(), entry);
		_mapping.insert(std::pair<std::string, std::string>(
            entry.name, entry.value));

		
		return;
	}

	direction dir;
	itemlist::iterator best = select_element_fuzzy(entry.name, &dir);
	if (dir == DIRECTION_BACKWARD && best != _items.begin())
		best--;

	if (best == _items.end()) 
		_items.push_back(entry);
	else
		_items.insert(best, entry);

	_mapping.insert(std::pair<std::string, std::string>(
		entry.name, entry.value));
}

void ConfigFile::update(itemlist::iterator it, const std::string &new_value) {
	std::pair<mapping_t::iterator, mapping_t::iterator> 
		match = _mapping.equal_range(it->name);
	for (mapping_t::iterator mapit = match.first; mapit != match.second; mapit++) {
		if (mapit->second == it->name) {
			mapit->second = new_value;
			break;
		}
	}
	it->changed = true;
	it->value = new_value;
}

void ConfigFile::erase(itemlist::iterator it) {
	std::pair<mapping_t::iterator, mapping_t::iterator> 
		match = _mapping.equal_range(it->name);
	for (mapping_t::iterator mapit = match.first; mapit != match.second; mapit++) {
		if (mapit->second == it->name) {
			_mapping.erase(mapit);
			break;
		}
	}
		
	_items.erase(it);
}

void ConfigFile::erase(itemlist::iterator begin, itemlist::iterator end) {
	for (; begin != end; begin++)
		erase(begin);
}

ConfigFile::file_entry ConfigFile::entry_from_row(std::string row) {
	file_entry entry;
	if (row[0] == '#') {
		entry.name = row;
		entry.value = "";
		entry.changed = false;
		entry.repeatable = true;
		entry.comment = true;
	} else if (row[0] == '\n' || row.size() <= 1) {
		entry.name = "";
		entry.value = "";
		entry.changed = false;
		entry.repeatable = true;
		entry.comment = true;			
	} else {
		size_t seperate = row.find_first_of('=');
		if (seperate != std::string::npos) {
			entry.name = row.substr(0, seperate);
			entry.name = trim(entry.name);
			//if (entry.name.back() == '*'){
			//	entry.name = entry.name.substr(0, entry.name.size() - 1);
			//	entry.repeatable = true;
			//} else {
			//	entry.repeatable = false;
			//}
		
			entry.value = row.substr(seperate+1);
			entry.value = trim (entry.value);
			entry.changed = false;
			entry.comment = false;
		} else {
			entry.comment = true;
			entry.changed = false;
			entry.value = "";
			entry.name = row;
		}
	}
	return entry;
}

std::string ConfigFile::row_from_entry(const ConfigFile::file_entry &entry) {
	if (entry.comment) {
		return entry.name;
	}  else {
		std::stringstream stream;
		stream << entry.name;
		//if (entry.repeatable)
		//	stream << "*";
		stream << " = ";
		stream << entry.value;
	
		return stream.str();
	}
}
