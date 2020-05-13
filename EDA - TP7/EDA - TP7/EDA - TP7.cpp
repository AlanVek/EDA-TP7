#include <iostream>
#include "QuadTree.h"

int main() {
	const char* in = "myImgBlack.png";
	const char* out = "myImgBlack2.EDA";
	const char* in2 = "myImgBlack2.png";

	try {
		QuadTree qt;
		qt.compressAndSave(in, out);
		print(qt.getTree());
		//qt.decompressAndSave(out, in2);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}