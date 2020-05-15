#include <iostream>
#include "QuadTree/QuadTree.h"
#include <chrono>

using namespace std::chrono;

auto getTime = [](const auto& start, const auto& end) {return pow(10, -9) * duration_cast<nanoseconds>(end - start).count(); };

int main() {
	/*Sets test names for input and output files. */
	const char* format = "EDA";
	const char* in1 = "Images/logo/logo";
	const char* out = "Images/logo/logo2";
	const char* in2 = "Images/logo/logo3";

	/*Sets test threshold value. */
	const double threshold = 0.15;

	try {
		auto startAll = steady_clock::now();

		QuadTree qt(".EDA");

		/*Compresses 'in' file to 'out' file. */
		auto startComp = steady_clock::now();
		qt.compressAndSave(in1, out, threshold);
		auto endComp = steady_clock::now();

		std::cout << "Compression completed successfully. Time taken [s]: " << getTime(startComp, endComp) << std::endl;

		/*Decompresses 'out' file to 'in2' file. */
		auto startDecomp = steady_clock::now();
		qt.decompressAndSave(out, in2);
		auto endDecomp = steady_clock::now();

		auto endAll = steady_clock::now();

		std::cout << "Decompression completed successfully. Time taken [s]: " << getTime(startDecomp, endDecomp) << std::endl;

		std::cout << "Total time taken [s]: " << getTime(startAll, endAll) << std::endl;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}