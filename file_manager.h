#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <stdexcept>
#include <sstream>

class FileManager {
private:
	const std::filesystem::path _path;
	std::vector<std::string> _content;
	std::vector<size_t> _mapping;
	size_t _appended = 0;
	bool _modified = false;

	// Frees memory by letting go of unused values in _content.
	void _compact() {
		if (_mapping.empty()) {
			_content.clear();
			return;
		}

		std::vector<std::string> result;
		result.reserve(_mapping.size());

		for (auto row : _mapping) {
			result.push_back(std::move(_content[row]));
		}

		_content = std::move(result);

		_mapping.resize(_content.size());
		std::iota(_mapping.begin(), _mapping.end(), 0);
	}

public:
	FileManager(std::filesystem::path path) :
		_path(std::move(path))
	{
		if (!std::filesystem::exists(_path)) {
			std::filesystem::create_directories(_path.parent_path());
			std::ofstream(_path);
		}

		std::ifstream in(_path);
		std::string row;
		size_t i = 0;

		while (std::getline(in, row)) {
			_content.push_back(std::move(row));
			_mapping.push_back(i++);
		}
	}

	~FileManager() {
		save();
	}

	// Returns the content at the specified row.
	std::string read(size_t row) const {
		if (row >= _mapping.size()) {
			throw std::out_of_range("FileManager::read: row out of bounds");
		}

		return _content[_mapping[row]];
	}

	// Splits the content of the specified row by a delimiter and returns it.
	std::vector<std::string> split(size_t row, char delimiter) const {
		if (row >= _mapping.size()) {
			throw std::out_of_range("FileManager::split: row out of bounds");
		}

		std::vector<std::string> result;
		std::stringstream ss(_content[_mapping[row]]);
		std::string part;

		while (std::getline(ss, part, delimiter)) {
			result.push_back(std::move(part));
		}

		return result;
	}

	// Returns the first row.
	std::string first() const {
		if (_mapping.size() == 0) {
			throw std::out_of_range("FileManager::first: no rows present");
		}

		return _content[_mapping[0]];
	}

	// Returns the last row.
	std::string last() const {
		if (_mapping.size() == 0) {
			throw std::out_of_range("FileManager::last: no rows present");
		}

		return _content[_mapping[_mapping.size() - 1]];
	}

	// Returns every row defined inside the file.
	std::vector<std::string> all() const {
		std::vector<std::string> result;
		result.reserve(_mapping.size());

		for (auto row : _mapping) {
			result.push_back(_content[row]);
		}

		return result;
	}

	// Appends given arguments to the file.
	template<typename... Args>
	void append(Args... args) {
		std::stringstream ss;
		(ss << ... << args);

		_content.push_back(ss.str());
		_mapping.push_back(_content.size() - 1);
		++_appended;
	}
	
	// Overwrites specified row with given arguments.
	template<typename... Args>
	void overwrite(size_t row, Args... args) {
		if (row >= _mapping.size()) {
			throw std::out_of_range("FileManager::overwrite: row out of bounds");
		}

		std::stringstream ss;
		(ss << ... << args);

		_content[_mapping[row]] = ss.str();
		_modified = true;
	}

	// Deletes the specified row.
	void erase(size_t row) {
		if (row >= _mapping.size()) {
			throw std::out_of_range("FileManager::erase: row out of bounds");
		}

		if (row >= _mapping.size() - _appended) {
			--_appended;
		}
		else {
			_modified = true;
		}

		_mapping.erase(_mapping.begin() + row);

		if (_content.size() >= _mapping.size() * 2) {
			_compact();
		}
	}

	// Clears the file.
	void clear() {
		_mapping.clear();
		_modified = true;
		_appended = 0;
	}

	// Saves all changes.
	void save() {
		if (_modified) {
			std::ofstream out(_path);

			for (auto row : _mapping) {
				out << _content[row] << std::endl;
			}

			_appended = 0;
			_modified = false;
		}
		else if (_appended > 0) {
			std::ofstream out(_path, std::ios::app);

			for (size_t n = _mapping.size(), i = n - _appended; i < n; ++i) {
				out << _content[_mapping[i]] << std::endl;
			}

			_appended = 0;
		}
	}

	// Returns number of present rows.
	size_t size() const noexcept {
		return _mapping.size();
	}

	// Returns true if the file is empty.
	bool empty() const noexcept {
		return _mapping.empty();
	}
};

