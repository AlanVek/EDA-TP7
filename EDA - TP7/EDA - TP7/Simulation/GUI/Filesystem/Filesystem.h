#pragma once
#include <vector>
#include <string>

using strVec = std::vector<std::string>;

class Filesystem {
public:

	Filesystem();

	const strVec& pathContent(const char* = nullptr, bool = false, int = 0, ...);

	void back();

	const std::string& getPath(void);

	/*Static functions.*/
	static bool isDir(const char*);
	static bool isFile(const char*);
private:

	/*Data members.*/
	bool mustUpdate;
	std::string path;
	strVec path_content;
};
