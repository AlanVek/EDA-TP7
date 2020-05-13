#include "QuadTree.h"
#include "lodepng.h"
#include <iostream>
#include <math.h>
#include <fstream>

namespace {
	const unsigned char alfa = 99;
	const float threshold = 0;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;

	const unsigned int maxNumber = 255;
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
	unsigned char* encoded = (unsigned char*)malloc((tree.size() + 1) * sizeof(unsigned char));
	if (!encoded)
		throw std::exception("Failed to allocate memory for output.");
	unsigned char size = static_cast<unsigned char> (log2(height));
	encoded[0] = size;
	for (unsigned int i = 0; i < tree.size(); i++) {
		encoded[i + 1] = tree.at(i);
	}
	lodepng_encode32_file(fileName, encoded, tree.size() + 1, 1);

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
	lodepng_encode32_file(fileName, updatedImg, amount, amount);

	free(updatedImg);
}

void QuadTree::compressAndSave(const char* in, const char* out) {
	originalData.clear();
	tree.clear();
	decodeRaw(in);
	compress(originalData);
	encodeCompressed(out);
}
void QuadTree::compress(const std::vector<unsigned char>& v) {
	unsigned int size = v.size();

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
		tree.push_back(0);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(v.at(i));
	}
	else if (lessThanThreshold(v)) {
		tree.push_back(0);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(mean.at(i));
	}
	else {
		tree.push_back(1);
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
	free(img);
}

void QuadTree::decodeCompressed(const char* fileName) {
	unsigned char* img;
	lodepng_decode32_file(&img, &width, &height, fileName);
	unsigned int size = static_cast<unsigned int> (pow(pow(2, img[0]), 2) * bytesPerPixel);
	decompressed = std::vector<unsigned char>(size);
	for (unsigned int i = 1; i < width * height; i++)
		originalData.push_back(img[i]);
	free(img);
}

void QuadTree::decompressAndSave(const char* in, const char* out) {
	originalData.clear();
	decompressed.clear();
	decodeCompressed(in);
	//decompress(originalData);
	//encodeRaw(out);
}

//void QuadTree::decompress(const std::vector<unsigned char>& v) {
//	unsigned int size = v.size();
//
//	if ((int)log2(width / bytesPerPixel) != log2(width / bytesPerPixel))
//		throw std::exception("Width not in form of 2^n.");
//	if ((int)log2(height) != log2(height))
//		throw std::exception("Height not in form of 2^n.");
//
//	if (width / bytesPerPixel != height)
//		throw std::exception("Image should be square.");
//	else if (size < divide) {
//		throw std::exception("Something weird happened");
//	}
//	else if (size == bytesPerPixel) {
//		tree.push_back(0);
//		for (int i = 0; i < bytesPerPixel - 1; i++)
//			tree.push_back(v.at(i));
//	}
//	else if (lessThanThreshold(v)) {
//		tree.push_back(0);
//		for (int i = 0; i < bytesPerPixel - 1; i++)
//			tree.push_back(mean.at(i));
//	}
//	else {
//		tree.push_back(1);
//		for (int i = 0; i < divide; i++) {
//			compress(cutVector(v, i));
//		}
//	}
//}

const std::vector<unsigned char>& QuadTree::getOriginalData(void) const { return originalData; }
const std::vector<unsigned char>& QuadTree::getTree(void) const { return tree; }

QuadTree::~QuadTree() {}

const std::vector<unsigned char> QuadTree::cutVector(const std::vector<unsigned char>& v, int which) {
	unsigned int row, col;

	if (which < 0 || which >= divide)
		throw std::exception("Wrong input in cutVector.");

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