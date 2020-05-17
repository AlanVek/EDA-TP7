#include "filesystem_header.h"

int fileSize(const char* filename)
{
	std::cout << "Tama�o de " << filename << " es " << file_size(filename) << '\n';
	getchar();
	return 0;
}
/*
file_size recibe como par�metro una clase path, que como constructor tiene un const char.
Entonces se construye en el momento de la invocaci�n cuando se le pasa un const char a la funcion.
*/