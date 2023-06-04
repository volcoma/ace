/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/file.h>
#include <bx/sort.h>

#include <time.h>

namespace entry
{
namespace
{

bx::AllocatorI* getDefaultAllocator()
{
	static bx::DefaultAllocator allocator;
	return &allocator;
}

static bx::AllocatorI* g_allocator = getDefaultAllocator();
typedef bx::StringT<&g_allocator> String;
static String s_currentDir;


class FileReader : public bx::FileReader
{
	typedef bx::FileReader super;

public:
	virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
	{
		String filePath(s_currentDir);
		filePath.append(_filePath);
		return super::open(filePath.getPtr(), _err);
	}
};

class FileWriter : public bx::FileWriter
{
	typedef bx::FileWriter super;

public:
	virtual bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override
	{
		String filePath(s_currentDir);
		filePath.append(_filePath);
		return super::open(filePath.getPtr(), _append, _err);
	}
};

bx::FileReaderI* getDefaultFileReader()
{
	static FileReader reader;
	return &reader;
}

static bx::FileReaderI* s_fileReader = getDefaultFileReader();

bx::FileWriterI* getDefaultFileWriter()
{
	static FileWriter writer;
	return &writer;
}
static bx::FileWriterI* s_fileWriter = getDefaultFileWriter();

} // namespace

bx::FileReaderI* getFileReader()
{
	return s_fileReader;
}

bx::FileWriterI* getFileWriter()
{
	return s_fileWriter;
}

bx::AllocatorI* getAllocator()
{
	return g_allocator;
}

} // namespace entry
