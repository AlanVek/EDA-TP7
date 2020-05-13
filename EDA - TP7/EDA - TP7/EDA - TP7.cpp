#include <iostream>
#include "QuadTree.h"

auto print = [](const auto& iter) { for (const auto& x : iter)std::cout << x << ' '; std::cout << std::endl; };

int main() {
	const char* name = "myImgBlack.png";

	try {
		QuadTree qt(name);
		qt.compressAndSave("myImgBlack2.EDA");
		print(qt.getTree());
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}