#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sstream>

class FileManager {
private:
	const std::filesystem::path _recoveryPath;
	const std::filesystem::path _filePath;
	std::vector<std::string> _cache;
	std::vector<size_t> _rowMapping;
	size_t _appendedRows = 0;
	bool _rewriteNecessary = false;
	bool _recoveryExists = false;

	enum class Error {
		FailedOpeningFile
	};

	enum class SaveMode {
		Best,
		Rewrite,
		Append
	};

	[[noreturn]] void _throw(Error e, const std::string& extra = "") {
		switch (e) {
		case Error::FailedOpeningFile:
			throw std::runtime_error("<FileManager> Couldn't open file " + extra);
		}
	}

	void _initCache(const std::filesystem::path& filePath) {
		std::ifstream in(filePath);

		if (!in.is_open()) {
			_throw(Error::FailedOpeningFile, filePath.string());
		}

		std::string rowContent;
		size_t row = 0;

		while (std::getline(in, rowContent)) {
			_cache.push_back(std::move(rowContent));
			_rowMapping.push_back(row++);
		}
	}

	bool _saveToFile(const std::filesystem::path& filePath, SaveMode saveMode) {
		if (!std::filesystem::exists(filePath)) {
			std::filesystem::create_directories(filePath.parent_path());
			saveMode = SaveMode::Rewrite;
		}

		if (saveMode == SaveMode::Best) {
			saveMode = _rewriteNecessary ? SaveMode::Rewrite : SaveMode::Append;
		}

		std::ios::openmode mode = saveMode == SaveMode::Rewrite ? std::ios::out : std::ios::app;
		std::ofstream out{ filePath, mode };

		if (!out.is_open()) {
			return false;
		}

		if (saveMode == SaveMode::Rewrite) {
			for (size_t rowIdx : _rowMapping) {
				out << _cache[rowIdx] << "\n";
			}
		}
		else {
			size_t total = _rowMapping.size();

			for (size_t rowIdx = total - _appendedRows; rowIdx < total; ++rowIdx) {
				out << _cache[_rowMapping[rowIdx]] << "\n";
			}
		}

		return true;
	}

public:
	FileManager(std::filesystem::path filePath) :
		_recoveryPath(filePath.parent_path() / ("RECOVERY_" + filePath.filename().string())),
		_filePath(std::move(filePath))
	{
		if (std::filesystem::exists(_recoveryPath)) {
			_initCache(_recoveryPath);
			_recoveryExists = true;
		}
		else if (std::filesystem::exists(_filePath)) {
			_initCache(_filePath);
		}
	}

	~FileManager() {
		save();
	}

	template<typename... Args>
	void append(Args... args) {
		std::stringstream ss;
		(ss << ... << args);

		_cache.push_back(ss.str());
		_rowMapping.push_back(_cache.size() - 1);
		++_appendedRows;
	}

	void save() {
		if (!_recoveryExists && !_rewriteNecessary && _appendedRows == 0) {
			return;
		}

		bool success;

		if (_recoveryExists) {
			success = _saveToFile(_filePath, SaveMode::Rewrite);
		}
		else {
			success = _saveToFile(_filePath, SaveMode::Best);
		}

		if (!success && !_saveToFile(_recoveryPath, SaveMode::Best)) {
			_throw(Error::FailedOpeningFile);
		}

		if (success && _recoveryExists) {
			std::error_code ec;
			std::filesystem::remove(_recoveryPath, ec);
			_recoveryExists = false;
		}

		_rewriteNecessary = false;
		_appendedRows = 0;
	}
};