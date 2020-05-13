#include "QuadTree.h"
#include "lodepng.h"
#include <iostream>
#include <math.h>

namespace {
	const unsigned char alfa = 255;
	const float threshold = 200;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;

	const unsigned int maxNumber = 255;

	const unsigned char hasChildren = 1;
	const unsigned char noChildren = 0;
}

QuadTree::QuadTree() { mean = { 0,0,0 }; }

bool QuadTree::lessThanThreshold(const std::vector<unsigned char>& v) {
	if (v.size() <= 1)
		return true;
	if (v.size() < bytesPerPixel)
		throw std::exception("lessThanThreshold got an invalid input.");

	std::vector<unsigned int> rgb = std::vector<unsigned int>(v.begin(), v.begin() + bytesPerPixel - 1);
	std::vector<unsigned int> maxrgb = rgb;
	std::vector<unsigned int> minrgb = rgb;
	unsigned int temp;
	bool result = false;
	int count = 0;
	long int tot = 0;
	for (unsigned int i = bytesPerPixel; i < v.size(); i++) {
		if (i % bytesPerPixel != (bytesPerPixel - 1)) {
			temp = v[i];
			if (temp > maxrgb[count])
				maxrgb[count] = temp;

			if (temp < minrgb[count])
				minrgb[count] = temp;

			rgb[count] += temp;
		}
		count++;
		if (count == bytesPerPixel)
			count = 0;
	}

	temp = 0;
	for (int i = 0; i < bytesPerPixel - 1; i++) {
		temp += maxrgb[i] - minrgb[i];
	}
	if (temp <= threshold) {
		for (int i = 0; i < bytesPerPixel - 1; i++)
			mean[i] = rgb[i] / (v.size() / bytesPerPixel);
		result = true;
	}
	return result;
}

void QuadTree::encodeCompressed(const char* fileName) {
	unsigned int size = findNearestMultiple(tree.size());
	unsigned char* encoded = (unsigned char*)malloc(size * sizeof(unsigned char));
	if (!encoded)
		throw std::exception("Failed to allocate memory for output.");
	encoded[0] = (unsigned char)(size - tree.size() - 1);
	encoded[1] = (unsigned char)log2(height);
	for (unsigned int i = 2; i < size - tree.size(); i++)
		encoded[i] = (unsigned char)5;
	for (unsigned int i = 0; i < tree.size(); i++) {
		encoded[i + size - tree.size()] = tree.at(i);
	}
	//if (strlen((char*)encoded) != size)
	//	throw std::exception("Size mismatch.");
	int res = lodepng_encode32_file(fileName, encoded, size / bytesPerPixel, 1);
	if (res) {
		std::string err = "Failed to encode compressed file: " + (std::string)lodepng_error_text(res);
		throw std::exception(err.c_str());
	}

	if (encoded)
		free(encoded);
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

void QuadTree::compressAndSave(const char* input, const char* output) {
	originalData.clear();
	tree.clear();
	decodeRaw(input);
	compress(originalData);
	encodeCompressed(output);
}
void QuadTree::compress(const std::vector<unsigned char>& v) {
	unsigned int size = v.size();
	if (!size) {
		throw std::exception("File is empty or doesn't exist.");
	}
	if ((int)log2(width / bytesPerPixel) != log2(width / bytesPerPixel))
		throw std::exception("Width not in form of 2^n.");
	if ((int)log2(height) != log2(height))
		throw std::exception("Height not in form of 2^n.");

	if (width / bytesPerPixel != height)
		throw std::exception("Image should be square.");
	else if (size < divide) {
		throw std::exception("Something weird happened");
	}
	else if (size == bytesPerPixel) {
		tree.push_back(noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(v.at(i));
	}
	else if (lessThanThreshold(v)) {
		tree.push_back(noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(mean.at(i));
	}
	else {
		tree.push_back(hasChildren);
		for (int i = 0; i < divide; i++) {
			compress(cutVector(v, i));
		}
	}
}

void QuadTree::decodeRaw(const char* fileName) {
	unsigned char* img;
	lodepng_decode32_file(&img, &width, &height, fileName);

	width *= bytesPerPixel;

	originalData.clear();
	for (unsigned int i = 0; i < width * height; i++) {
		if (i % bytesPerPixel != (bytesPerPixel - 1))
			originalData.push_back((unsigned int)img[i]);
		else
			originalData.push_back(alfa);
	}
	if (img)
		free(img);
}

void QuadTree::decodeCompressed(const char* fileName) {
	unsigned char* img;
	lodepng_decode32_file(&img, &width, &height, fileName);
	if (!img)
		throw std::exception("lodepng_decode32_file error");
	unsigned int cant = img[0];
	unsigned int size = img[1];
	unsigned int index = cant + 1;

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
			/*if (*(--absPosit.end()) == (bytesPerPixel - 1))
				move--;*/
			v = std::vector<unsigned char>(v.begin() + move, v.end());
		}
		else {
			int temp[] = { v[0], v[1], v[2] };
			fillCompressedVector(temp, absPosit);
			unsigned int move = bytesPerPixel;
			/*if (*(--absPosit.end()) == (bytesPerPixel - 1))
				move--;*/
			v.clear();
		}
		//throw std::exception("Decompress got an invalid input.");
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
	/*std::cout << "Level: " << level << std::endl;
	std::cout << "Position: ";
	print(absPosit);*/
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

const std::vector<unsigned char>& QuadTree::getOriginalData(void) const { return originalData; }
const std::vector<unsigned char>& QuadTree::getTree(void) const { return tree; }

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

unsigned int QuadTree::findNearestMultiple(unsigned int number) {
	unsigned int temp = number + 1;
	while (temp % 4) {
		temp++;
	}
	return temp;
}