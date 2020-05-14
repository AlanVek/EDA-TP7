#include <iostream>
#include "QuadTree.h"

int main() {
	const char* in = "myImg.png";
	const char* out = "myImg2.EDA";
	const char* in2 = "myImg3.png";

	try {
		QuadTree qt;
		qt.compressAndSave(in, out);
		qt.decompressAndSave(out, in2);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}