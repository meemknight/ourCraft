///////////////////////////////////////////
//do not remove this notice
//(c) Luta Vlad
// 
// safeSave 1.0.0
// 
///////////////////////////////////////////

#pragma once
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cstdint>

#ifdef _MSC_VER
#pragma warning( disable : 26812 )
#endif

namespace sfs
{
	enum Errors : int
	{
		noError = 0,
		couldNotOpenFinle,
		fileSizeDitNotMatch,
		checkSumFailed,
		couldNotMakeBackup,
		readBackup,
		warningEntryAlreadyExists,
		entryNotFound,
		entryHasDifferentDataType,
		couldNotParseData,
		fileSizeNotBigEnough,
	};
	
	const char* getErrorString(Errors e);
	
	//can return error: couldNotOpenFinle
	Errors readEntireFile(std::vector<char>& data, const char* name);
	
	//reades the content of a file (size bytes), if shouldMatchSize is false will read the entire fill untill size bytes are read or the entire file was read
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch
	Errors readEntireFile(void* data, size_t size, const char* name, bool shouldMatchSize, int *bytesRead = nullptr);

	//gets the file size
	//can return error: couldNotOpenFinle
	Errors getFileSize(const char *name, size_t &size);

	//reades the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors readEntireFileWithCheckSum(void* data, size_t size, const char* name);

	//reades the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle, fileSizeNotBigEnough
	Errors readEntireFileWithCheckSum(std::vector<char> &data, const char *name);

	//writes the entire content of the data to a file and uses checkSum
	//can return error: couldNotOpenFinle
	Errors writeEntireFileWithCheckSum(const void* data, size_t size, const char* name);

	//writes the entire content of the data to a file
	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const std::vector<char>& data, const char* name);
	
	//writes the entire content of the data to a file
	//can return error: couldNotOpenFinle
	Errors writeEntireFile(const void*data, size_t size, const char* name);

	//saved the data with a check sum and a backup
	//can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(const void* data, size_t size, const char* nameWithoutExtension, bool reportnotMakingBackupAsAnError);

	//loads the data that was saved using safeSave
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed, 
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)
	Errors safeLoad(void* data, size_t size, const char* nameWithoutExtension, bool reportLoadingBackupAsAnError);

	//loads the data that was saved using safeSave and stored as a SafeSafeKeyValueData structure
	//can return error: couldNotOpenFinle, checkSumFailed, fileSizeNotBigEnough
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)
	Errors safeLoad(std::vector<char> &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);

	//same as safeLoad but only loads the backup file.
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors safeLoadBackup(void* data, size_t size, const char* nameWithoutExtension);


	//used to save data and read it
	struct SafeSafeKeyValueData
	{
		struct Entry
		{
			enum Types
			{
				no_type = 0,
				rawData_type,
				int_type,
				float_type,
				bool_type,
				string_type,
			};

			std::vector<char> data;
			char type = 0;
			union Primitives
			{
				std::int32_t intData;
				float floatData;
				bool boolData;
			}primitives;
		};

		std::unordered_map<std::string, Entry> entries;

		//returns true if entry exists
		bool entryExists(std::string at);

		//can return error: entryNotFound
		Errors getEntryType(std::string at, char &type);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setRawData(std::string at, void *data, size_t size);

		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getRawDataPointer(std::string at, void* &data, size_t &size);

		//won't change i if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getInt(std::string at, int32_t &i);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setInt(std::string at, int32_t i);

		//won't change f if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getFloat(std::string at, float &f);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setFloat(std::string at, float f);

		//won't change b if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getBool(std::string at, bool &b);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setBool(std::string at, bool b);

		//can return error: warningEntryAlreadyExists, if so it will overwrite data
		Errors setString(std::string at, std::string value);

		//won't change s if failed
		//can return error: entryNotFound, entryHasDifferentDataType
		Errors getString(std::string at, std::string &s);

		std::vector<char> formatIntoFileData();

		//can return error: couldNotParseData
		Errors loadFromFileData(char *data, size_t size);
	};
	
	//saved the data stored as a SafeSafeKeyValueData structure in a binary format with a check sum and a backup
	//can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError);

	//loads the data that was saved using safeSave and stored as a SafeSafeKeyValueData structure
	//can return error: couldNotOpenFinle, fileSizeNotBigEnough, checkSumFailed, couldNotParseData
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded from the backup)
	Errors safeLoad(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);



#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__

	struct FileMapping
	{
		void* pointer = {};
		size_t size = 0;
		struct
		{
			void* fileHandle = 0;
			void* fileMapping = 0;
		}internal = {};
	};

#elif defined __linux__

	struct FileMapping
	{
		void* pointer = {};
		size_t size = 0;
		struct
		{
			int fd = 0;
		}internal = {};
	};

#endif

	//a file mapping maps the content of a file to ram
	//can return error: couldNotOpenFinle
	Errors openFileMapping(FileMapping& fileMapping, const char* name, size_t size, bool createIfNotExisting);

	void closeFileMapping(FileMapping& fileMapping);

};