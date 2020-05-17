#pragma once
#include <vector>
#include <string>

using strVec = std::vector<std::string>;

class Filesystem {
public:

	Filesystem();

	const strVec& pathContent(const char* = nullptr, bool = false);

	void back();

	const std::string& getPath(void);

	static bool isDir(const char*);
	static bool isFile(const char*);
private:
	bool mustUpdate;
	std::string path;
	strVec path_content;
};
