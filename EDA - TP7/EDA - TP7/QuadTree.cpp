#include "QuadTree.h"
#include "lodepng.h"
#include <iostream>
#include <math.h>

namespace {
	const unsigned char alfa = 99;
	const float threshold = 0;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;
}

//auto print = [](const auto& iter) { for (const auto& x : iter)std::cout << x << ' '; std::cout << std::endl; };

QuadTree::QuadTree() { mean = { 0,0,0 }; }

QuadTree::QuadTree(const char* fileName) {
	mean = { 0,0,0 };
	unsigned char* img;
	lodepng_decode32_file(&img, &originalWidth, &originalHeight, fileName);
	originalWidth *= bytesPerPixel;
	for (unsigned int i = 0; i < originalWidth * originalHeight; i++) {
		if (i % bytesPerPixel != (bytesPerPixel - 1))
			originalData.push_back((unsigned int)img[i]);
		else
			originalData.push_back(alfa);
	}
	free(img);
}

bool QuadTree::lessThanThreshold(const std::vector<unsigned int>& v) {
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

void QuadTree::encode(const char* fileName) {
	unsigned char* updatedImg = (unsigned char*)malloc((tree.size() + 1) * sizeof(unsigned char));
	if (!updatedImg)
		throw std::exception("Failed to allocate memory for compressed image.");
	updatedImg[0] = originalHeight * originalWidth;
	for (unsigned int i = 0; i < tree.size(); i++) {
		updatedImg[i + 1] = tree.at(i);
	}
	lodepng_encode32_file(fileName, updatedImg, tree.size() / 2, tree.size() / 2);

	free(updatedImg);
}

void QuadTree::compressAndSave(const char* fileName) {
	compress(originalData);
	encode(fileName);
}
void QuadTree::compress(const std::vector<unsigned int>& v) {
	unsigned int size = v.size();

	if ((int)log2(originalWidth / bytesPerPixel) != log2(originalWidth / bytesPerPixel))
		throw std::exception("Width not in form of 2^n.");
	if ((int)log2(originalHeight) != log2(originalHeight))
		throw std::exception("Height not in form of 2^n.");

	if (originalWidth / bytesPerPixel != originalHeight)
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

void QuadTree::decompress();

const std::vector<unsigned int>& QuadTree::getOriginalData(void) const { return originalData; }
const std::vector<int>& QuadTree::getTree(void) const { return tree; }

QuadTree::~QuadTree() {}

const std::vector<unsigned int> QuadTree::cutVector(const std::vector<unsigned int>& v, int which) {
	unsigned int row, col;

	if (which < 0 || which >= divide)
		throw std::exception("Wrong input in cutVector.");

	unsigned int halfCol = (unsigned int)(sqrt(v.size() * bytesPerPixel) / 2);
	unsigned int halfRow = halfCol / bytesPerPixel;

	row = (which / 2) * halfRow;
	col = (which % 2) * halfCol;

	std::vector <unsigned int> temp;

	for (unsigned int i = row; i < row + halfRow; i++) {
		for (unsigned int j = col; j < col + halfCol; j++) {
			temp.push_back(v.at(i * (2 * halfCol) + j));
		}
	}
	return temp;
}