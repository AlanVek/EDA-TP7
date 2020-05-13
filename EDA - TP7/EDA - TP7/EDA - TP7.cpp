#include <iostream>
#include "QuadTree.h"

int main() {
	const char* in = "myImg.png";
	const char* out = "myImg2BIG.EDA";
	const char* in2 = "myImg2_decompBIG.png";

	try {
		QuadTree qt;
		qt.compressAndSave(in, out);
		//print(qt.getTree());
		qt.decompressAndSave(out, in2);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}