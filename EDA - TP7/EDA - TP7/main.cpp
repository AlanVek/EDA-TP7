#include <iostream>
#include "QuadTree.h"

int main() {
	/*Sets test names for input and output files. */
	const char* in = "myImg.png";
	const char* out = "myImg2.EDA";
	const char* in2 = "myImg3.png";

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