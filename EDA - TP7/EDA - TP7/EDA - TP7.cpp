#include <iostream>
#include "QuadTree.h"

int main() {
	const char* in = "logo.png";
	const char* out = "logo2.EDA";
	const char* in2 = "logo3.png";

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