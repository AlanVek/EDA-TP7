#include "filesystem_header.h"


int sort_directories(const char* dir)
{
	// no hubo tiempo perdon :) 
	return 0;
}

int see_dirContent(const char* dir)
{
	path p(dir); 	
		if (exists(p))
		{
			if (is_regular_file(p))
				cout << p << "Es un archivo de tamaño " << file_size(p) << '\n';
			else if (is_directory(p))     // is p a directory?
			{
				cout << p << " Es un directorio, que tiene :\n \n";
				for(directory_iterator itr(p);itr != directory_iterator(); itr++)
					 cout << itr->path().filename()  << endl;
			}
		}
		else
			cout << p << " No existe\n";
		getchar();
	return 0;
}
/*
La funcion copy mueve todos los elementos comprendidos entre directory_iterator() (que devuelve un ITERATOR END)
y directory_iterator(p) que devuelve la carpeta raiz a  ostream_iterator, que se ocupa de transformar a la clase
especificada en <> (TEMPLATE) en algo legible para cout.
*/