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

	void newPath(const std::string&);

	/*Static functions.*/
	static bool isDir(const char*);
	static bool isFile(const char*);
	static const std::string originalPath(void);
private:

	/*Data members.*/
	bool mustUpdate;
	std::string path;
	strVec path_content;
};
