#include "Filesystem.h"
#include <boost/filesystem.hpp>

/*Filesystem constructor. Sets path to current path
and loads this->path_content.*/
Filesystem::Filesystem() {
	mustUpdate = true;

	/*Loads vector and string with content.*/
	pathContent(boost::filesystem::current_path().string().c_str());
}

/*Returns the contents of the given path.
If none is given, it returns the contents of this->path.*/
const strVec& Filesystem::pathContent(const char* imgPath, bool force)
{
	/*If mustUpdate is true, then path changed recently from another method.
	If force is true, then the caller is forcing to reaload file info from path.
	If imgPath is null or equal to this->path, then it doesn't reaload files because
	they are already in this->path_content.*/
	if (mustUpdate || force || (imgPath && (imgPath != path))) {
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

			/*Loops for every file/directory in path and saves it's name to path_content.*/
			for (boost::filesystem::directory_iterator itr(p); itr != boost::filesystem::directory_iterator(); itr++) {
				path_content.push_back(itr->path().filename().string());
			}

			/*Updates current path.*/
			path = p.string();
		}
	}

	/*Returns file vector.*/
	return path_content;
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
/********************************************************/