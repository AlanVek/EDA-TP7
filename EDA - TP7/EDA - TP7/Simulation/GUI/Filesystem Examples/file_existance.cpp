#include "filesystem_header.h"

int check_existance(const char* file)
{
	path p(file); //Constructor de path. lleva un const char.
	if (exists(p))  
	{
		if (is_regular_file(p)) 
			cout << p << " existe y su tamaño es " << file_size(p) << '\n';
		else if (is_directory(p))
			cout << p << " Es un directorio!\n";
		else
			cout << p << "Es un dispositivo Pipe o un Socket. En windows, capaz la lectora de CDs\n";
	}
	else
		cout << p << " No existe\n";
	getchar();
	return 0;
}