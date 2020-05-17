#include "Filesystem.h"
#include <boost/filesystem.hpp>

Filesystem::Filesystem() {
	mustUpdate = true;
	pathContent(boost::filesystem::current_path().string().c_str());
}

const strVec& Filesystem::pathContent(const char* imgPath, bool force)
{
	if (mustUpdate || force || (imgPath && (imgPath != path))) {
		if (mustUpdate) mustUpdate = !mustUpdate;

		boost::filesystem::path p;
		if (imgPath) {
			if (path.length()) {
				p = (path + '\\' + imgPath);
			}
			else
				p = imgPath;
		}
		else
			p = path;

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

void Filesystem::back() {
	path = path.substr(0, path.find_last_of('\\'));
	path_content.clear();
	mustUpdate = true;
}