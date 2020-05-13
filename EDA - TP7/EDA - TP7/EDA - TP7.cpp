#include <iostream>
#include "QuadTree.h"

auto print = [](const auto& iter) { for (const auto& x : iter)std::cout << x << ' '; std::cout << std::endl; };

int main() {
	const char* name = "myImg.png";

	try {
		QuadTree qt(name);
		qt.startCompression();
		qt.encode("mySecondImg.EDA");
		print(qt.getTree());
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}