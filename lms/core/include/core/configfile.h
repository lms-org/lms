#ifndef CONFIGFILE_H
#define CONFIGFILE_H

/** 
 * \file: configfile.h
 * \author: Johannes Walcher
 * \version: 1.1
 * \date: 20 / Aug / 2013
 * (c) Johannes Walcher, all rights reserved
 */
 
#include <string>
#include <sstream>
#include <vector>
#include <map>
/** 
 * ConfigFile Class 
 *
 * Represents a File where settings can be stored and read. 
 * The Format is like KEY = VALUE. The Key may contain anything except 
 * the Equalsign. A Commentary Row must begin with # (no spaces before) 
 * Items with a * after the Key may occure more often (the * is not part 
 * of the key. 
 */

namespace lms{
class ConfigFile {
public:
	/// Open and parse config file. 
	/// \param name Name of the file without ending
	ConfigFile ( const std::string &name );
	
	bool good() { return _good; }
	
	/** 
	 * Works just like get(std::string), only if not found sets the default 
	 * and save it to the file. Afterwards, Returns default. Is not Created if 
	 * Already existing. 
	 * \param field Key to the that should be got or created. 
	 * \param dflt Default Value to be set or ignored
	 * \return Value of that field as _T
	 */
	template <typename _T>
	_T get_or_default(const std::string &field, const _T &dflt) {

		std::stringstream s; 
		s << dflt;
		itemlist::iterator it = select_element(field, s.str());
		if (it != _items.end()) {
			_T t;
			std::stringstream s(it->value);
			s >> t;
			return t;
		} else {
			insert (field, s.str());
			return dflt;
		}
	}
		 
	/// Set or Insert the Field given. 
	/// \param field The Key to the field to be set
	/// \param value Value to be set. 
	template <typename _T>
	void set (const std::string &field, _T value) {
		std::stringstream s;
		s << value;
		set_str (field, s.str());
	}
		
		
	/// Just like set, just with string (faster version)
	void set_str (const std::string &field, const std::string &v);
	
	/// Finish Changes and write them to File
	bool flush();
	
	void getKeys(std::vector<std::string> &keys) {
		for(auto it = _items.begin(); it != _items.end(); ++it) {
			keys.push_back(it->name);
		}
	}
	
protected:	
	/**
	 * Get a Field as Type out of the Configfile. 
	 * \param	field Key of the field, whoose first match will be extracted
	 * \return	Type of the Template given. 
	 *						Has to be extractable from stringstream.
	 */ 
	template <typename _T>
	_T get(const std::string &field) {
		std::stringstream s;
		s << get_str(field);
		_T t;
		s >> t;
		return t;
	}
		
	/// Get a field out of the file. 
	/// \param field Key to the field, whoose first occurence is returned
	/// \return Value of that field
	std::string get_str(const std::string &field);

private:
	bool _good;
	/** Entries to the files */
	struct file_entry {
		std::string name;		///< Name of the Entry
		std::string value;		///< Value of the Entry
		bool changed;			///< Has the Value changed?
		bool repeatable;		///< Can it occure multiple times
		bool comment;			///< Is this entry / Line comment
	};

	/** Internal Mapping */
	typedef std::multimap<std::string, std::string> mapping_t;

	/** Type of Listing of the rows. */
	typedef std::vector<file_entry> itemlist;
		
	/** Every Row of the file gets one item in the list. */
	itemlist _items;
		
	/** Filename */
	const std::string _name;
		
	/** Parsed Input into String => Value */
	mapping_t _mapping;
		
	///File Management Functions
	///Select an Element based on its field name. 
	/// \return found: First Element that matches
	///						not found _items.end is returned
	itemlist::iterator select_element(const std::string &field);
	
	///Select an Element based on its field name and an offset, if multiple exist. 
	/// \return found: First Element that matches
	///						not found _items.end is returned
	itemlist::iterator select_element_offset(const std::string &field, int offset);
	
	///Select an Element based on its field name and its value. 
	/// \return found: First Element that matches
	///						not found _items.end is returned
	itemlist::iterator select_element(const std::string &field, const std::string &old_value);
		
		
	///Check if element exists, and create it. Otherwise: set. 
	void insert_or_update(const std::string &field, const std::string &value);
	
	enum direction {
		DIRECTION_FORWARD, ///< The needle should be placed in back if haystack
		DIRECTION_BACKWARD ///< The needle should be placed in front if haystack
	};
	///Calculate, how good two elements match
	int matching (const std::string &needle, const std::string &haystack, direction *out_dir);
		
	///Get the element, that matches best on the field, and the direction where it will match
	itemlist::iterator select_element_fuzzy(const std::string &field, direction* out_dir);
		
	///Insert an element with the field name, at a position that matches best possible
	void insert(const std::string &field, const std::string &value);
	///Insert an element as file entry, at a position that matches best possible
	void insert(const file_entry &);
	///Update a given Element
	void update(itemlist::iterator it, const std::string &new_value);
	///Erase ONE element
	void erase(itemlist::iterator element);
	///Erase the Range [from ... to)...
	void erase(itemlist::iterator begin, itemlist::iterator end);
	
	/// Decoder of a row 
	/// \return entry into items table
	file_entry entry_from_row(std::string row);
	 
	/// Coding from File Entry to row 
	/// \return Row without \n
	std::string row_from_entry(const file_entry &entry);
};
}
#endif
