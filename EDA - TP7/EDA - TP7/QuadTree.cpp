#include "QuadTree.h"
#include "lodepng.h"
#include <iostream>
#include <math.h>

namespace {
	const unsigned char alfa = 255;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;

	const unsigned int maxNumber = 255;

	const unsigned char hasChildren = 1;
	const unsigned char noChildren = 0;
	const unsigned char filling = 10;
}
/*QuadTree constructor. Just sets mean and threshold to 0.*/
QuadTree::QuadTree() { mean = std::vector<unsigned char>(bytesPerPixel); threshold = NULL; }

/*******************************

		Compression

*******************************/

/*Decodes raw data from file.*/
void QuadTree::decodeRaw(const char* fileName) {
	unsigned char* img;

	/*Decodes file and checks for errors.*/
	int error = lodepng_decode32_file(&img, &width, &height, fileName);
	if (error) {
		std::string errStr = "Failed to decode file. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	/*Sets real width.*/
	width *= bytesPerPixel;

	/*Loads originalData with raw data from file.*/
	originalData.clear();
	for (unsigned int i = 0; i < width * height; i++) {
		if (i % bytesPerPixel != (bytesPerPixel - 1))
			originalData.push_back((unsigned int)img[i]);
		else
			originalData.push_back(alfa);
	}

	/*Frees resources.*/
	if (img)
		free(img);
}

/*Encodes compressed data to file.*/
void QuadTree::encodeCompressed(const char* fileName) {
	/*Generates size that is a multiple of bytesPerPixel.*/
	unsigned int size = findNearestMultiple(tree.size(), bytesPerPixel);

	/*Allocates memory for data array.*/
	unsigned char* encoded = (unsigned char*)malloc(size * sizeof(unsigned char));
	if (!encoded)
		throw std::exception("Failed to allocate memory for output.");

	/*Loads original image size in first position.
	The file can only take numbers from 0 to 255.
	As images are square and its sides
	are of the form 2^n, the chosen method was to
	set n as the size output.*/
	encoded[0] = (unsigned char)log2(height);

	/*Fills the rest of the space used to get the size as
	a multiple of bytesPerPixel with an arbitrary number different
	than hasChildren and noChildren.*/
	for (unsigned int i = 1; i < size - tree.size(); i++)
		encoded[i] = (unsigned char)filling;

	/*Loads the rest of the array with the real compressed data.*/
	for (unsigned int i = 0; i < tree.size(); i++) {
		encoded[i + size - tree.size()] = tree.at(i);
	}

	/*Encodes compressed data in file and checks for errors.*/
	int error = lodepng_encode32_file(fileName, encoded, size / bytesPerPixel, 1);
	if (error) {
		std::string errStr = "Failed to encode compressed file. Lodepng error: " + (std::string)lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	/*Frees resources.*/
	if (encoded)
		free(encoded);
}

/*Finds the nearest multiple of base that is higher than number.*/
unsigned int QuadTree::findNearestMultiple(unsigned int number, unsigned int base) {
	unsigned int temp = number + 1;
	while (temp % base) {
		temp++;
	}
	return temp;
}

/*Recursively compressed data to tree.*/
void QuadTree::compress(const std::vector<unsigned char>& v) {
	unsigned int size = v.size();

	/*Checks if vector is empty.*/
	if (!size) {
		throw std::exception("File is empty or doesn't exist.");
	}

	/*Checks if width is a power of 2.*/
	if ((int)log2(width / bytesPerPixel) != log2(width / bytesPerPixel))
		throw std::exception("Width not in form of 2^n.");

	/*Checks if height is a power of 2.*/
	if ((int)log2(height) != log2(height))
		throw std::exception("Height not in form of 2^n.");

	/*Checks if vector is square*/
	if (width / bytesPerPixel != height)
		throw std::exception("Image should be square.");

	/*Checks if vector has appropriate length.*/
	else if (size < bytesPerPixel) {
		throw std::exception("Compress got invalid input.");
	}

	/*If it's only one pixel...*/
	else if (size == bytesPerPixel) {
		/*Loads noChildren to tree and pushes RGB code.
		It's a leaf.*/
		tree.push_back(noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(v.at(i));
	}

	/*If vector's RGB formula is less than threshold...*/
	else if (lessThanThreshold(v)) {
		/*Loads noChildren to tree and pushes mean RGB code.
		It's a leaf.*/
		tree.push_back(noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(mean.at(i));
	}

	/*Otherwise...*/
	else {
		/*It pushes hasChildren, cuts the vector in
		'divide' parts and compresses each part separately.
		It's an inner node.*/
		tree.push_back(hasChildren);
		for (int i = 0; i < divide; i++) {
			compress(cutVector(v, i));
		}
	}
}
/*Checks if vector's RGB formula is less than threshold.
If it is, then it returns true and saves mean values to 'mean'.
Otherwise, it returns false.*/
bool QuadTree::lessThanThreshold(const std::vector<unsigned char>& v) {
	/*Checks for correct shape.*/
	if (v.size() < bytesPerPixel)
		throw std::exception("lessThanThreshold got an invalid input.");

	/*Creates variables to use in function.

	mexrgb saves max values of rgb.
	minrgb saves min values of rgb.*/
	mean = std::vector<unsigned char>(v.begin(), v.begin() + bytesPerPixel - 1);
	std::vector<unsigned char> maxrgb = mean;
	std::vector<unsigned char> minrgb = mean;
	unsigned int temp;
	bool result = false;
	int count = 0;

	/*Loops through vector, saving each value to the corresponding
	position in vectors and applying the appropiate checks.*/
	for (unsigned int i = bytesPerPixel; i < v.size(); i++) {
		/*If it's not alpha...*/
		if (i % bytesPerPixel != (bytesPerPixel - 1)) {
			temp = v[i];
			/*If value is higher than corresponding value
			in maxrgb, it changes it.*/
			if (temp > maxrgb[count])
				maxrgb[count] = temp;

			/*If value is lower than corresponding value
			in maxrgb, it changes it.*/
			if (temp < minrgb[count])
				minrgb[count] = temp;

			/*Updates mean.*/
			mean[count] += temp;
		}
		count++;

		/*Resets count when it's higher than bytesPerPixel.*/
		if (count == bytesPerPixel)
			count = 0;
	}

	temp = 0;

	/*Applies formula.*/
	for (int i = 0; i < bytesPerPixel - 1; i++) {
		temp += maxrgb[i] - minrgb[i];
	}

	/*Checks if formula is lower than threshold. */
	if (temp <= threshold) {
		for (int i = 0; i < bytesPerPixel - 1; i++)
			mean[i] /= (v.size() / bytesPerPixel);
		result = true;
	}
	return result;
}

void QuadTree::encodeRaw(const char* fileName) {
	unsigned char* updatedImg = (unsigned char*)malloc(decompressed.size() * sizeof(unsigned char));
	if (!updatedImg)
		throw std::exception("Failed to allocate memory for compressed image.");

	for (unsigned int i = 0; i < decompressed.size(); i++) {
		updatedImg[i] = decompressed.at(i);
	}
	unsigned int amount = static_cast<unsigned int> (sqrt(decompressed.size() / bytesPerPixel));
	if (lodepng_encode32_file(fileName, updatedImg, amount, amount))
		throw std::exception("Failed to encode raw file");
	if (updatedImg)
		free(updatedImg);
}

void QuadTree::compressAndSave(const char* input, const char* output, const double threshold) {
	originalData.clear();
	tree.clear();
	this->threshold = threshold;
	decodeRaw(input);
	compress(originalData);
	encodeCompressed(output);
}

void QuadTree::decodeCompressed(const char* fileName) {
	unsigned char* img;
	lodepng_decode32_file(&img, &width, &height, fileName);
	if (!img)
		throw std::exception("lodepng_decode32_file error");
	width *= bytesPerPixel;
	unsigned int size = img[0];
	unsigned int index = 1;
	for (; img[index] == filling; index++) {};

	size = static_cast<unsigned int> (pow(pow(2, size), 2) * bytesPerPixel);
	decompressed = std::vector<unsigned char>(size);
	for (; index < width * height; index++)
		originalData.push_back(img[index]);
	if (img)
		free(img);
}

void QuadTree::decompressAndSave(const char* input, const char* output) {
	originalData.clear();
	decompressed.clear();
	decodeCompressed(input);
	decompress(originalData);
	encodeRaw(output);
}

void QuadTree::decompress(std::vector<unsigned char>& v) {
	unsigned int size = v.size();

	static std::list<int> absPosit;
	if (size < 5608)
		int a = 1;
	if (!size) {
		return;
	}

	else if (v[0] == noChildren) {
		if (v.size() >= bytesPerPixel) {
			int temp[] = { v[1], v[2], v[3] };
			fillCompressedVector(temp, absPosit);
			unsigned int move = bytesPerPixel;
			v = std::vector<unsigned char>(v.begin() + move, v.end());
		}
		else {
			throw std::exception("Decompress got an invalid input.");
		}
	}
	else if (v[0] == hasChildren) {
		std::vector<unsigned char> temp;
		temp = std::vector<unsigned char>(v.begin() + 1, v.end());
		for (int i = 0; i < divide && v.size(); i++) {
			absPosit.push_back(i);
			decompress(temp);
			v = temp;
			absPosit.pop_back();
		}
	}
	else
		throw std::exception("Decompress got an invalid input.");
}

void QuadTree::fillCompressedVector(int* rgb, const std::list<int>& absPosit) {
	unsigned int level = absPosit.size();
	unsigned int size = decompressed.size() / pow(divide, level);
	unsigned int newWidth = sqrt(size * bytesPerPixel);
	unsigned int newHeight = newWidth / bytesPerPixel;
	unsigned int initCol = 0;
	unsigned int initRow = 0;
	unsigned int tempSize;
	unsigned int counter = 0;
	unsigned int halfCol, halfRow;
	for (const auto& x : absPosit) {
		tempSize = decompressed.size() / pow(divide, counter);
		counter++;
		halfCol = (sqrt(tempSize * bytesPerPixel) / 2);
		halfRow = halfCol / bytesPerPixel;
		initCol += (x % 2) * halfCol;
		initRow += (x / 2) * halfRow;
	}
	unsigned int iter = 0;
	for (unsigned int i = initRow; i < initRow + newHeight; i++) {
		for (unsigned int j = initCol; j < initCol + newWidth; j++) {
			if (iter == (bytesPerPixel - 1)) {
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = alfa;
				iter = -1;
			}
			else
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = rgb[iter];
			iter++;
		}
	}
}

QuadTree::~QuadTree() {}

const std::vector<unsigned char> QuadTree::cutVector(const std::vector<unsigned char>& v, int which) {
	unsigned int row, col;

	if (which < 0 || which >= divide)
		throw std::exception("Wrong input in cutVector.");

	if (!v.size())
		return v;

	unsigned int halfCol = (unsigned int)(sqrt(v.size() * bytesPerPixel) / 2);
	unsigned int halfRow = halfCol / bytesPerPixel;

	row = (which / 2) * halfRow;
	col = (which % 2) * halfCol;

	std::vector <unsigned char> temp;

	for (unsigned int i = row; i < row + halfRow; i++) {
		for (unsigned int j = col; j < col + halfCol; j++) {
			temp.push_back(v.at(i * (2 * halfCol) + j));
		}
	}
	return temp;
}