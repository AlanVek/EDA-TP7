#pragma once
#include <vector>
#include <string>

using strVec = std::vector<std::string>;

class Filesystem {
public:

	Filesystem();

	const strVec& pathContent(const char* = nullptr, bool = false);

	const std::string& getPath(void);

	static bool isDir(const char*);
	static bool isFile(const char*);
private:
	std::string path;
	strVec path_content;
};
