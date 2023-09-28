# SafeSave

---

Allows you to save data and don't worry if your save will get corupted. In the program is closed while the file is being saved the library will load the backup.
If the original file has beed modified by the outside the library will load the backup. If both the backup and the original file are corupted the library will 
report this error.

Has other functionalities like file mappings (not working on linux yet).

---

Integration: paste the include/safeSave.h file and the src/safeSvae.cpp file into your project.

---

Basic Functions:

```cpp
  //can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(const void* data, size_t size, const char* nameWithoutExtension, bool reportnotMakingBackupAsAnError);

	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed, 
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded with the backup)
	Errors safeLoad(void* data, size_t size, const char* nameWithoutExtension, bool reportLoadingBackupAsAnError);

	//same as safeLoad but only loads the backup file.
	//can return error: couldNotOpenFinle, fileSizeDitNotMatch, checkSumFailed
	Errors safeLoadBackup(void* data, size_t size, const char* nameWithoutExtension);
```

Error reporting:
Every function returns an error code. 
You can use
```cpp
	const char* getErrorString(Errors e);
```
to get the error string 

This are the possible error codes:
```cpp
enum Errors : int
	{
		noError,
		couldNotOpenFinle,
		fileSizeDitNotMatch,
		checkSumFailed,
		couldNotMakeBackup,
		readBackup,
	};
```

---

All Functions:

```cpp
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
 
 //saved the data stored as a SafeSafeKeyValueData structure in a binary format with a check sum and a backup
	//can return error: couldNotOpenFinle, 
	//	couldNotMakeBackup (if reportnotMakingBackupAsAnError is true, but will still save the first file)
	Errors safeSave(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError);

	//loads the data that was saved using safeSave and stored as a SafeSafeKeyValueData structure
	//can return error: couldNotOpenFinle, fileSizeNotBigEnough, checkSumFailed, couldNotParseData
	//	readBackup (if reportLoadingBackupAsAnError but data will still be loaded from the backup)
	Errors safeLoad(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError);
 
```
