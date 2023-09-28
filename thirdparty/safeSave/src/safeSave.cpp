///////////////////////////////////////////
//do not remove this notice
//(c) Luta Vlad
// 
// safeSave 1.0.0
// 
///////////////////////////////////////////

#include <safeSave.h>

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__
#define NOMINMAX 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#elif defined __linux__
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#endif


namespace sfs
{

	constexpr const char* errorsString[] =
	{
		"noError",
		"couldNotOpenFinle",
		"fileSizeDitNotMatch",
		"checkSumFailed",
		"couldNotMakeBackup",
		"readBackup",
	};

	const char* getErrorString(Errors e)
	{
		if (e >= 0 && e < sizeof(errorsString) / sizeof(errorsString[0]))
		{
			return errorsString[e];
		}
		else
		{
			return "unknown error";
		}
	}

	Errors readEntireFile(std::vector<char>& data, const char* name)
	{
		data.clear();
		std::ifstream f(name, std::ios::binary);

		if(f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t size = f.tellg();
			f.seekg(0, std::ios_base::beg);

			data.resize(size);

			f.read(&data[0], size);

			return noError;
		}else
		{
			return couldNotOpenFinle;
		}
	}

	Errors readEntireFile(void* data, size_t size, const char* name, bool shouldMatchSize, int* bytesRead)
	{
		if (bytesRead)
		{
			*bytesRead = 0;
		}

		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);

			if (shouldMatchSize)
			{
				if (readSize != size)
				{
					return fileSizeDitNotMatch;
				}
				else
				{
					f.read((char*)data, readSize);
					if (bytesRead)
					{
						*bytesRead = readSize;
					}

					return noError;
				}
			}
			else
			{
				f.read((char*)data, std::min(size, readSize));

				if (bytesRead)
				{
					*bytesRead = std::min(size, readSize);
				}

				return noError;
			}
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors getFileSize(const char *name, size_t &size)
	{
		size = 0;

		std::ifstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.close();
			size = readSize;

			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}
	
	using HashType = unsigned long long;
	
	//https://stackoverflow.com/questions/34595/what-is-a-good-hash-function
	unsigned long long fnv_hash_1a_64(const void* key, int len)
	{
		const unsigned char* p = (const unsigned char*)key;
		unsigned long long h = 0xcbf29ce484222325ULL;

		//for (int i = 0; i < len; i++)
		//{
		//	h = (h ^ p[i]) * 0x100000001b3ULL;
		//}

		if(len >= 4)
		for (int i = 0; i < len-3; i+=4)
		{
			h = (h ^ p[i + 0]) * 0x100000001b3ULL;
			h = (h ^ p[i + 1]) * 0x100000001b3ULL;
			h = (h ^ p[i + 2]) * 0x100000001b3ULL;
			h = (h ^ p[i + 3]) * 0x100000001b3ULL;
		}
		
		for (int i = len - (len%4); i < len; i++)
		{
			h = (h ^ p[i]) * 0x100000001b3ULL;
		}

		return h;
	}

	Errors readEntireFileWithCheckSum(void* data, size_t size, const char* name)
	{
		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);
			
			size_t sizeWithCheckSum = size + sizeof(HashType);

			if (readSize != sizeWithCheckSum)
			{
				return fileSizeDitNotMatch;
			}
			else
			{
				f.read((char*)data, size);
				HashType checkSum = 0;
				f.read((char*)&checkSum, sizeof(HashType));

				auto testCheck = fnv_hash_1a_64(data, size);

				if (testCheck != checkSum)
				{
					return checkSumFailed;
				}
				else
				{
					return noError;
				}
			}
			
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors readEntireFileWithCheckSum(std::vector<char> &data, const char *name)
	{
		data.clear();

		std::ifstream f(name, std::ios::binary);
		if (f.is_open())
		{
			f.seekg(0, std::ios_base::end);
			size_t readSize = f.tellg();
			f.seekg(0, std::ios_base::beg);

			if(readSize > sizeof(HashType))
			{
				data.resize(readSize - sizeof(HashType));
				f.read(&data[0], readSize - sizeof(HashType));

				HashType checkSum = 0;
				f.read((char *)&checkSum, sizeof(HashType));

				auto testCheck = fnv_hash_1a_64(&data[0], data.size());

				if (testCheck != checkSum)
				{
					return checkSumFailed;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return fileSizeNotBigEnough;
			}

		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors writeEntireFileWithCheckSum(const void* data, size_t size, const char* name)
	{
		std::ofstream f(name, std::ios::binary);

		if (f.is_open())
		{
			f.write((char*)data, size);
			auto testCheck = fnv_hash_1a_64(data, size);

			f.write((char*)&testCheck, sizeof(testCheck));

			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors writeEntireFile(const std::vector<char>& data, const char* name)
	{
		return writeEntireFile(data.data(), data.size(), name);
	}

	Errors writeEntireFile(const void* data, size_t size, const char* name)
	{
		std::ofstream f(name, std::ios::binary);
	
		if (f.is_open())
		{
			f.write((char*)data, size);
			return noError;
		}
		else
		{
			return couldNotOpenFinle;
		}
	}

	Errors safeSave(const void* data, size_t size, const char* nameWithoutExtension, bool reportnotMakingBackupAsAnError)
	{
		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		auto err = writeEntireFileWithCheckSum((char*)data, size, file1.c_str());

		if (err == couldNotOpenFinle)
		{
			return couldNotOpenFinle;
		}
		else
		{
			auto err2 = writeEntireFileWithCheckSum((char*)data, size, file2.c_str());	

			if (err2 == couldNotOpenFinle)
			{
				if (reportnotMakingBackupAsAnError)
				{
					return couldNotMakeBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return noError;
			}
		}
	}

	Errors safeLoad(void* data, size_t size, const char* nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		auto err = readEntireFileWithCheckSum((char*)data, size, file1.c_str());

		if (err == noError)
		{
			return noError;
		}
		else 
		{
			//load backup
			auto err2 = readEntireFileWithCheckSum((char*)data, size, file2.c_str());

			if (err2 == noError)
			{
				if (reportLoadingBackupAsAnError)
				{
					return readBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return err2;
			}
		}
	}

	Errors safeLoad(std::vector<char> &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		data.clear();

		std::string file1 = nameWithoutExtension; file1 += "1.bin";
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		auto err = readEntireFileWithCheckSum(data, file1.c_str());

		if (err == noError)
		{
			return noError;
		}
		else
		{
			//load backup
			auto err2 = readEntireFileWithCheckSum(data, file2.c_str());

			if (err2 == noError)
			{
				if (reportLoadingBackupAsAnError)
				{
					return readBackup;
				}
				else
				{
					return noError;
				}
			}
			else
			{
				return err2;
			}
		}
	}

	Errors safeLoadBackup(void* data, size_t size, const char* nameWithoutExtension)
	{
		std::string file2 = nameWithoutExtension; file2 += "2.bin";

		//load backup
		auto err2 = readEntireFileWithCheckSum((char*)data, size, file2.c_str());
		return err2;
	}

	Errors safeSave(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportnotMakingBackupAsAnError)
	{
		auto rez = data.formatIntoFileData();

		return safeSave(rez.data(), rez.size(), nameWithoutExtension, reportnotMakingBackupAsAnError);
	}

	Errors safeLoad(SafeSafeKeyValueData &data, const char *nameWithoutExtension, bool reportLoadingBackupAsAnError)
	{
		data = {};
		
		std::vector<char> readData;
		auto errCode = safeLoad(readData, nameWithoutExtension, reportLoadingBackupAsAnError);

		if (errCode == noError || errCode == readBackup)
		{
			return data.loadFromFileData(readData.data(), readData.size());
		}
		else
		{
			return errCode;
		}
	}


#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined __NT__

	Errors openFileMapping(FileMapping& fileMapping, const char* name, size_t size, bool createIfNotExisting)
	{
		fileMapping = {};

		DWORD createDisposition = 0;

		if (createIfNotExisting)
		{
			createDisposition = OPEN_ALWAYS;
		}
		else
		{
			createDisposition = OPEN_EXISTING;
		}

		fileMapping.internal.fileHandle = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0,
			NULL, createDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileMapping.internal.fileHandle == INVALID_HANDLE_VALUE)
		{
			auto err = GetLastError();
			return Errors::couldNotOpenFinle;
		}

		fileMapping.internal.fileMapping = CreateFileMappingA(fileMapping.internal.fileHandle, NULL, PAGE_READWRITE, 0, size, NULL);

		if (fileMapping.internal.fileMapping == NULL)
		{
			CloseHandle(fileMapping.internal.fileHandle);
			return Errors::couldNotOpenFinle;
		}


		fileMapping.pointer = MapViewOfFile(fileMapping.internal.fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);

		if (fileMapping.pointer == nullptr)
		{
			CloseHandle(fileMapping.internal.fileMapping);
			CloseHandle(fileMapping.internal.fileHandle);
			return Errors::couldNotOpenFinle;
		}

		fileMapping.size = size;


		return Errors::noError;
	}

	void closeFileMapping(FileMapping& fileMapping)
	{
		UnmapViewOfFile(fileMapping.pointer);
		CloseHandle(fileMapping.internal.fileMapping);
		CloseHandle(fileMapping.internal.fileHandle);
		fileMapping = {};
	}

#elif defined __linux__

	Errors openFileMapping(FileMapping& fileMapping, const char* name, size_t size, bool createIfNotExisting)
	{
		int createDisposition = 0;
		if(createIfNotExisting)
		{
			createDisposition = O_CREAT;
		}

		fileMapping.internal.fd = open(name, O_RDWR | createDisposition);

		if(fileMapping.internal.fd == -1)
		{
			return Errors::couldNotOpenFinle;
		}

		if(ftruncate(fileMapping.internal.fd, size) == -1)
		{
			close(fileMapping.internal.fd);
			return Errors::couldNotOpenFinle;
		}
		
		fileMapping.pointer = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED,
				fileMapping.internal.fd, 0);

		if(fileMapping.pointer == MAP_FAILED)
		{
			fileMapping.pointer = 0;
			close(fileMapping.internal.fd);
			return Errors::couldNotOpenFinle;
		}

		fileMapping.size = size;
		

		return Errors::noError;
	}

	void closeFileMapping(FileMapping& fileMapping)
	{
		fsync(fileMapping.internal.fd);
		msync(fileMapping.pointer, fileMapping.size, MS_SYNC);
		munmap(fileMapping.pointer, fileMapping.size);
		close(fileMapping.internal.fd);
		
		fileMapping = {};
	}

#endif	

	bool SafeSafeKeyValueData::entryExists(std::string at)
	{
		return entries.find(at) != entries.end();
	}

	Errors SafeSafeKeyValueData::getEntryType(std::string at, char &type)
	{
		type = 0;

		auto it = entries.find(at);

		if (it == entries.end()) { return Errors::entryNotFound; }

		type = it->second.type;

		return Errors::noError;
	}

	Errors SafeSafeKeyValueData::setRawData(std::string at, void *data, size_t size)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::rawData_type;
			it->second.data.clear();
			it->second.data.resize(size);
			memcpy(it->second.data.data(), data, size);

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};
			e.type = Entry::Types::rawData_type;
			e.data.resize(size);
			memcpy(e.data.data(), data, size);

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setInt(std::string at, int32_t i)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::int_type;
			it->second.data.clear();
			it->second.primitives.intData = i;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::int_type;
			e.primitives.intData = i;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}


	Errors SafeSafeKeyValueData::getRawDataPointer(std::string at, void *&data, size_t &size)
	{
		data = 0;
		size = 0;

		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::rawData_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				size = it->second.data.size();
				data = it->second.data.data();
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getInt(std::string at, int32_t &i)
	{
		auto it = entries.find(at);
		
		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::int_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				i = it->second.primitives.intData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setFloat(std::string at, float f)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::float_type;
			it->second.data.clear();
			it->second.primitives.floatData = f;

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::float_type;
			e.primitives.floatData = f;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}


	Errors SafeSafeKeyValueData::getFloat(std::string at, float &f)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::float_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				f = it->second.primitives.floatData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::getBool(std::string at, bool &b)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::bool_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				b = it->second.primitives.boolData;
				return Errors::noError;
			}
		}
	}

	Errors SafeSafeKeyValueData::setBool(std::string at, bool b)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::bool_type;
			it->second.data.clear();
			it->second.data.push_back(b);
			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};

			e.type = Entry::Types::bool_type;
			e.primitives.boolData = b;

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::setString(std::string at, std::string value)
	{
		auto it = entries.find(at);

		if (it != entries.end())
		{
			it->second.type = Entry::Types::string_type;
			it->second.data.clear();
			size_t size = value.length();
			it->second.data.resize(size);
			memcpy(it->second.data.data(), value.c_str(), size);

			return Errors::warningEntryAlreadyExists;
		}
		else
		{
			Entry e = {};
			e.type = Entry::Types::string_type;
			size_t size = value.length();
			e.data.resize(size);
			memcpy(e.data.data(), value.c_str(), size);

			entries.insert({at, std::move(e)});

			return Errors::noError;
		}
	}

	Errors SafeSafeKeyValueData::getString(std::string at, std::string &s)
	{
		auto it = entries.find(at);

		if (it == entries.end())
		{
			return Errors::entryNotFound;
		}
		else
		{
			if (it->second.type != Entry::Types::string_type)
			{
				return Errors::entryHasDifferentDataType;
			}
			else
			{
				s.clear();
				s.resize(it->second.data.size());
				memcpy(&s[0], it->second.data.data(), it->second.data.size());
				return Errors::noError;
			}
		}
	}

	std::vector<char> SafeSafeKeyValueData::formatIntoFileData()
	{
		std::vector<char> ret;
		ret.reserve(200);

		size_t size = 0;
		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;

			size += s.size() + 1;
			size += d.data.size();
			size += 1; //type
			size += 4; //data size
		}

		ret.reserve(size);

		for (auto &e : entries)
		{
			auto &s = e.first;
			auto &d = e.second;
			
			for (auto c : s)
			{
				ret.push_back(c);
			}
			ret.push_back(0);

			ret.push_back(d.type);
			
			if (d.type == Entry::Types::rawData_type || d.type == Entry::Types::string_type)
			{
				size_t size = d.data.size();
				ret.push_back(((char *)(&size))[0]);
				ret.push_back(((char *)(&size))[1]);
				ret.push_back(((char *)(&size))[2]);
				ret.push_back(((char *)(&size))[3]);

				for (auto d : d.data)
				{
					ret.push_back(d);
				}
			}
			else if(d.type == Entry::Types::bool_type)
			{
				ret.push_back(d.primitives.boolData);
			}
			else if (d.type == Entry::Types::int_type)
			{
				std::int32_t i = d.primitives.intData;
				ret.push_back(((char*)(&i))[0]);
				ret.push_back(((char*)(&i))[1]);
				ret.push_back(((char*)(&i))[2]);
				ret.push_back(((char*)(&i))[3]);
			}
			else if (d.type == Entry::Types::float_type)
			{
				float f = d.primitives.intData;
				ret.push_back(((char *)(&f))[0]);
				ret.push_back(((char *)(&f))[1]);
				ret.push_back(((char *)(&f))[2]);
				ret.push_back(((char *)(&f))[3]);
			}

		}

		return ret;
	}

	Errors SafeSafeKeyValueData::loadFromFileData(char *data, size_t size)
	{
		*this = {};

		std::string currentName = {};

		for (char *c = data; c < data + size; c++)
		{
			bool readingName = 1;
			if (*c == 0)
			{
				readingName = 0;
			}
			else
			{
				currentName.push_back(*c);
			}

			if (!readingName)
			{
				c++; if (c >= data + size) { return Errors::couldNotParseData; }

				char type = *c;

				if (type == Entry::Types::bool_type)
				{
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					bool b = *c;

					Entry e;
					e.type = Entry::Types::bool_type;
					e.primitives.boolData = b;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::int_type)
				{
					std::int32_t i = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&i))[3] = *c;

					Entry e;
					e.type = Entry::Types::int_type;
					e.primitives.intData = i;
					
					entries[currentName] = e;
				}
				else if (type == Entry::Types::float_type)
				{
					float f = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&f))[3] = *c;

					Entry e;
					e.type = Entry::Types::float_type;
					e.primitives.floatData = f;

					entries[currentName] = e;
				}
				else if (type == Entry::Types::rawData_type)
				{
					size_t s = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[3] = *c;

					Entry e;
					e.type = Entry::Types::rawData_type;
					e.data.reserve(s);

					for (int i = 0; i < s; i++)
					{
						c++; if (c >= data + size) { return Errors::couldNotParseData; }
						e.data.push_back(*c);
					}
					
					entries[currentName] = std::move(e);
				}
				else if (type == Entry::Types::string_type)
				{
					size_t s = 0;

					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[0] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[1] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[2] = *c;
					c++; if (c >= data + size) { return Errors::couldNotParseData; }
					((char *)(&s))[3] = *c;

					Entry e;
					e.type = Entry::Types::string_type;
					e.data.reserve(s);

					for (int i = 0; i < s; i++)
					{
						c++; if (c >= data + size) { return Errors::couldNotParseData; }
						e.data.push_back(*c);
					}

					entries[currentName] = std::move(e);

				}

				currentName = {};
			}

		}

		if (currentName == "")
		{
			return Errors::noError;
		}
		else
		{
			return Errors::couldNotParseData;
		}

	}


}
