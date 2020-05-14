#include <iostream>
#include "QuadTree.h"

int main() {
	/*Sets test names for input and output files. */
	const char* in = "logo.png";
	const char* out = "logo2.EDA";
	const char* in2 = "logo3.png";

	/*Sets test threshold value. */
	const double threshold = 0;

	try {
		QuadTree qt;

		/*Compresses 'in' file to 'out' file. */
		qt.compressAndSave(in, out, threshold);

		/*Decompresses 'out' file to 'in2' file. */
		qt.decompressAndSave(out, in2);

		std::cout << "Compression and decompression completed successfully." << std::endl;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}