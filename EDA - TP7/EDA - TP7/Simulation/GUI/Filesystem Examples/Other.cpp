#include "filesystem_header.h"
#pragma warning(disable : 4996)

int returnCurrentPath() {
	cout << "\n";
	cout << current_path().string();
	cout << "\n";
	return 1;
}

int copyFile(const char* base, const char* targ) {
	copy_file(base, targ);
	return 1;
}
/*
supone que queres copiar hola.txt de la carpeta music a la carpeta imagenes. Entonces entras con {ruta}/music/hola.txt
e {ruta}/imagenes/hola.txt, aunque {ruta}/imagenes/hola.txt no exista es lo que estas creando
*/

int createDir(const char* dir) {
	return create_directory(dir);
}
#define _CRT_SECURE_NO_WARNINGS
int seeLastWritten(const char* dir) {
	char storeDate[32];

	time_t lastTime = last_write_time(dir);
	tm *readable = localtime(&lastTime);

	strftime(storeDate, 32, "%a, %d.%m.%Y %H:%M:%S", readable);
	cout << storeDate;
	return 1;
}