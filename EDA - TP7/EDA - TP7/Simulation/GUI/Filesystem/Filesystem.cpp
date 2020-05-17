#include "Filesystem.h"
#include <boost/filesystem.hpp>
#include <iostream>
Filesystem::Filesystem() {
	pathContent(boost::filesystem::current_path().string().c_str());
}

const strVec& Filesystem::pathContent(const char* imgPath, bool force)
{
	if (imgPath && (imgPath != path || force)) {
		boost::filesystem::path p;
		if (path.length())
			p = (path + '\\' + imgPath);
		else
			p = imgPath;

		if (boost::filesystem::exists(p) && boost::filesystem::is_directory(p)) {
			path_content.clear();
			for (boost::filesystem::directory_iterator itr(p); itr != boost::filesystem::directory_iterator(); itr++) {
				path_content.push_back(itr->path().filename().string());
			}
			path = p.string();
		}
	}
	return path_content;
}

const std::string& Filesystem::getPath(void) { return path; }

bool Filesystem::isFile(const char* path) {
	return boost::filesystem::is_regular_file(path);
}

bool Filesystem::isDir(const char* path) {
	return boost::filesystem::is_directory(path);
}