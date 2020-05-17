#include "filesystem_header.h"

int main(int argc, char *argv[]) {
	cout << "\n\nBienvenidos al ejemplo de EDA. elija que hacer con el archivo de la linea de comandos.";
	cout << "\n" << "\n";

	cout << "    " << "1 - Verificar el archivo \n" << "\n";
	cout << "    " << "2 - Obtener el tamaño del archivo \n" << "\n";
	cout << "    " << "3 - Ver el contenido del directorio \n" << "\n";
	cout << "    " << "4 - Ver el contenido del directorio ordenado alfabeticamente \n" << "\n";
	
	cout << "    " << "5 - Actual directurio" << "\n" << "\n";
	cout << "    " << "6 - Copiar archivo desde (primer parametro) a (segundo parámetro)" << "\n" << "\n";

	cout << "    " << "7 - Crear directorio" << "\n" << "\n";
	cout << "    " << "8 - Ver ultima modificación" << "\n" << "\n";

	cout << "    " << "Otro - Salir" << "\n";
	char c;
	c = getchar();

	c -= '0';
	switch (c) {
	case 1:
		check_existance(argv[1]);
		break;
	case 2:
		fileSize(argv[1]);
		break;
	case 3:
		see_dirContent(argv[1]);
		break;
	case 4:
		sort_directories(argv[1]);
		break;
	case 5:
		returnCurrentPath();
		break;
	case 6:
		copyFile(argv[1], argv[2]);
		break;
	case 7:
		createDir(argv[1]);
		break;
	case 8:
		seeLastWritten(argv[1]);
		break;

	default:
		return 0;
		break;
	}
	cout << endl << "Presione Enter para salir..." << endl;
	getchar();
	return 0;
}