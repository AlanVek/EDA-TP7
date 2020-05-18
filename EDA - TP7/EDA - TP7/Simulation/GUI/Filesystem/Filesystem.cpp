#include "Filesystem.h"
#include <boost/filesystem.hpp>
#include <stdarg.h>

/*Filesystem constructor. Sets path to current path
and loads this->path_content.*/
Filesystem::Filesystem() {
	mustUpdate = true;

	/*Loads vector and string with content.*/
	pathContent(boost::filesystem::current_path().string().c_str());
}

/*Returns the contents of the given path. If none is given,
it returns the contents of this->path. In both cases, if the variadic
arguments are given, it filters files by format type of said arguments.*/
const strVec& Filesystem::pathContent(const char* imgPath, bool force, int count, ...)
{
	/*If mustUpdate is true, then path changed recently from another method.
	If force is true, then the caller is forcing to reaload file info from path.
	If imgPath is null or equal to this->path, then it doesn't reaload files because
	they are already in this->path_content.*/
	if (mustUpdate || force || (imgPath && (imgPath != path))) {
		std::map<std::string, int> formats;

		/*Gets format arguments (if any were given).*/
		if (count) {
			va_list args;
			va_start(args, count);

			for (int i = 0; i < count; i++)
				formats[va_arg(args, const char*)] = 1;

			va_end(args);
		}

		/*Toggles mustUpdate.*/
		if (mustUpdate) mustUpdate = !mustUpdate;

		boost::filesystem::path p;

		/*If imgPath was not null...*/
		if (imgPath) {
			/*If path has already been initialized...*/
			if (path.length()) {
				/*Sets new path to path\imgPath. */
				p = (path + '\\' + imgPath);
			}
			/*Otherwise, imgPath is the actual path to get files from.*/
			else
				p = imgPath;
		}

		/*If path was null, then it's a reaload from this->path. */
		else
			p = path;

		/*If the given path was correct and it is a directory...*/
		if (boost::filesystem::exists(p) && boost::filesystem::is_directory(p)) {
			/*Clears file vector.*/
			path_content.clear();

			bool loadCondition;
			/*Loops for every file/directory in path. If it's a directory, it saves
			it to path_content. If it's a file and its format is has been passed as
			argument, it saves it to path_content. Otherwise, it skips it.*/
			for (boost::filesystem::directory_iterator itr(p); itr != boost::filesystem::directory_iterator(); itr++) {
				/*Checks if it's either directory or a file with format given as argument.*/
				loadCondition = !count || isDir(itr->path().string().c_str()) ||
					(isFile(itr->path().string().c_str()) && formats.find(itr->path().extension().string()) != formats.end());

				if (loadCondition)
					path_content.push_back(itr->path().filename().string());
			}

			/*Updates current path.*/
			path = p.string();
		}
	}

	/*Returns file vector.*/
	return path_content;
}
/*Sets new path.*/
void Filesystem::newPath(const std::string& newPath_) {
	if (path != newPath_) {
		boost::filesystem::path p(newPath_);

		if (boost::filesystem::exists(p)) {
			path = newPath_;
			mustUpdate = true;
		}
	}
}

/*Getter.*/
const std::string& Filesystem::getPath(void) { return path; }

/*Moves path to the previous directory (closer to C:\).*/
void Filesystem::back() {
	path = path.substr(0, path.find_last_of('\\'));
	path_content.clear();
	mustUpdate = true;
}

/*Static methods.*/
/********************************************************/
/*Checks if a path is a file.*/
bool Filesystem::isFile(const char* path) {
	return boost::filesystem::is_regular_file(path);
}

/*Checks if a path is a directory.*/
bool Filesystem::isDir(const char* path) {
	return boost::filesystem::is_directory(path);
}

/*Returns current path.*/
const std::string Filesystem::originalPath(void) {
	return boost::filesystem::current_path().string();
};

/********************************************************/