#include <iostream>
#include "QuadTree/QuadTree.h"

int main() {
	/*Sets test names for input and output files. */
	const char* in = "Images/logo.png";
	const char* out = "Images/logo2.EDA";
	const char* in2 = "Images/logo3.png";

	/*Sets test threshold value. */
	const double threshold = 0.1;

	try {
		QuadTree qt;

		/*Compresses 'in' file to 'out' file. */
		qt.compressAndSave(in, out, threshold);

		std::cout << "Compression completed successfully." << std::endl;

		/*Decompresses 'out' file to 'in2' file. */
		qt.decompressAndSave(out, in2);

		std::cout << "Decompression completed successfully." << std::endl;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}