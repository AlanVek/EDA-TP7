#include "filesystem_header.h"


int not_sort_directories(const char* dir)
{
	path p(dir);   // p se crea a partir de un const char
		if (exists(p))
		{
			if (is_regular_file(p)) 
				cout << p << " size is " << file_size(p) << '\n';
			else if (is_directory(p))   
			{
				cout << p << " is a directory containing:\n";

				for(directory_iterator itr(p);itr != directory_iterator(); itr++)
					 cout << itr->path().filename()  << endl;
			}
		}
		else
			cout << p << "No existe\n";
		getchar();
	return 0;
}