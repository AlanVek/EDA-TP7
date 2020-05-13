#ifndef __FILESYSTEM_HEADER
#define __FILESYSTEM_HEADER

#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost::filesystem;

#define ARCHIVO_EXISTE "hola.txt"
#define ARCHIVO_INEXISTE "no_hola.txt"

int sort_directories(const char* dir);
int see_dirContent(const char* dir);
int fileSize(const char* filename);
int check_existance(const char* file);
int copyFile(const char* base, const char* targ);
int returnCurrentPath();
int createDir(const char* dir);
int seeLastWritten(const char* dir);


#endif