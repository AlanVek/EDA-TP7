#include "QuadTree.h"
#include "lodepng.h"
#include <math.h>

/*Constants to use throughout program. */
namespace {
	const unsigned char alfa = 255;
	const unsigned int maxDif = 3 * 255;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;
}
namespace treeData {
	const enum : unsigned char {
		hasChildren,
		noChildren,
		filling,
	};
}

/*QuadTree constructor. Just sets mean and threshold to 0.*/
QuadTree::QuadTree() {
	mean = std::vector<unsigned int>(bytesPerPixel);
	threshold = NULL;
}

/*******************************

		  Compression

*******************************/

/*Compresses image from input file to output file, with the given threshold. */
void QuadTree::compressAndSave(const char* input, const char* output, const double threshold_) {
	if (threshold_ > 0 && threshold_ <= 1) {
		/*Clears tree.*/
		tree.clear();

		/*Sets threshold.*/
		threshold = threshold_ * maxDif;

		/*Decodes file.*/
		decodeRaw(input);

		/*Compresses file.*/
		compress(originalData);

		/*Encodes compressed file.*/
		encodeCompressed(output);
	}
	else
		throw std::exception("Threshold must be a non-negative value between 0 and 1.");
}

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
		originalData.push_back(img[i]);
	}

	/*Frees resources.*/
	if (img)
		free(img);
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
		/*Loads noChildren to tree and pushes RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(v.at(i));
	}

	/*If vector's RGB formula is less than threshold...*/
	else if (lessThanThreshold(v)) {
		/*Loads noChildren to tree and pushes mean RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(mean.at(i));
	}

	/*Otherwise...*/
	else {
		/*It pushes hasChildren, cuts the vector in
		'divide' parts and compresses each part separately.
		It's an inner node.*/
		tree.push_back(treeData::hasChildren);
		for (int i = 0; i < divide; i++) {
			compress(cutVector(v, i));
		}
	}
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
		encoded[i] = treeData::filling;

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
	mean = std::vector<unsigned int>(v.begin(), v.begin() + bytesPerPixel - 1);
	std::vector<unsigned int> maxrgb = mean;
	std::vector<unsigned int> minrgb = mean;
	bool result = false;
	int count = 0;

	/*Loops through vector, saving each value to the corresponding
	position in vectors and applying the appropiate checks.*/
	for (unsigned int i = bytesPerPixel; i < v.size(); i++) {
		/*If it's not alpha...*/
		if (i % bytesPerPixel != (bytesPerPixel - 1)) {
			/*If value is higher than corresponding value
			in maxrgb, it changes it.*/
			if (v[i] > maxrgb[count])
				maxrgb[count] = v[i];

			/*If value is lower than corresponding value
			in maxrgb, it changes it.*/
			if (v[i] < minrgb[count])
				minrgb[count] = v[i];

			/*Updates mean.*/
			mean[count] += v[i];
		}
		/*Updates count and resets it when it reached bytesPerPixel.*/
		if ((++count) == bytesPerPixel)
			count = 0;
	}

	unsigned int temp = 0;

	/*Applies formula.*/
	for (unsigned int i = 0; i < bytesPerPixel - 1; i++) {
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

/*Cuts a part of the given vector according to the parameter which and returns said part.*/
const std::vector<unsigned char> QuadTree::cutVector(const std::vector<unsigned char>& v, int which) {
	unsigned int row, col;

	/*Checks validity of input.*/
	if (which < 0 || which >= divide)
		throw std::exception("Wrong input in cutVector.");

	/*If the vector is empty, it returns it. It can't be cut.*/
	if (!v.size())
		return v;

	/*Sets paraemeters for row and column at the half of each dimension.*/
	unsigned int halfCol = sqrt(v.size() * bytesPerPixel) / 2;
	unsigned int halfRow = halfCol / bytesPerPixel;

	/*Sets starting row and column.*/
	row = (which / 2) * halfRow;
	col = (which % 2) * halfCol;

	std::vector <unsigned char> temp;

	/*Fills 'temp' with the given part of the original vector.*/
	for (unsigned int i = row; i < row + halfRow; i++) {
		for (unsigned int j = col; j < col + halfCol; j++) {
			temp.push_back(v.at(i * (2 * halfCol) + j));
		}
	}
	return temp;
}

/*Finds the nearest multiple of base that is higher than number.*/
unsigned int QuadTree::findNearestMultiple(unsigned int number, unsigned int base) {
	unsigned int temp = number + 1;
	while (temp % base) {
		temp++;
	}
	return temp;
}

/*******************************

		  Decompression

*******************************/

/*Decompresses the input file and saves it to output file. */
void QuadTree::decompressAndSave(const char* input, const char* output) {
	/*Clears placeholder for decompressed data.*/
	decompressed.clear();

	/*Decodes compressed file.*/
	decodeCompressed(input);

	/*Decompresses file.*/
	decompress(originalData);

	/*Encodes raw data file.*/
	encodeRaw(output);
}

/*Decodes compressed data from file. */
void QuadTree::decodeCompressed(const char* fileName) {
	unsigned char* img;

	/*Decodes data and checks for errors.*/
	lodepng_decode32_file(&img, &width, &height, fileName);
	if (!img)
		throw std::exception("lodepng_decode32_file error");

	/*Sets real width.*/
	width *= bytesPerPixel;

	/*Gets size parameter from first position of data.*/
	unsigned int size = img[0];

	/*Goes to end of 'fill' portion of data.*/
	unsigned int index = 1;
	for (; img[index] == treeData::filling; index++) {};

	/*Sets real size of original image.*/
	size = static_cast<unsigned int> (pow(pow(2, size), 2) * bytesPerPixel);

	/*Sets decompressed to correct size.*/
	decompressed = std::vector<unsigned char>(size);

	originalData.clear();

	/*Loads compressed data to originalData.*/
	for (; index < width * height; index++)
		originalData.push_back(img[index]);

	/*Frees resources.*/
	if (img)
		free(img);
}

/*Decompresses data from vector. */
void QuadTree::decompress(std::vector<unsigned char>& v) {
	unsigned int size = v.size();

	/*Sets static list to check for absolut position within data.*/
	static std::list<int> absPosit;

	/*If the vector is empty, it can't be decompressed.*/
	if (!size) {
		return;
	}

	/*If it found a leaf...*/
	else if (v[0] == treeData::noChildren) {
		/*If the format is correct...*/
		if (v.size() >= bytesPerPixel) {
			int temp[bytesPerPixel - 1];

			/*Loads 'temp' with RGB data.*/
			for (unsigned int i = 0; i < bytesPerPixel - 1; i++)
				temp[i] = v[i + 1];

			/*Fills the corresponding section of 'decompressed' with the RGB data.*/
			fillDecompressedVector(temp, absPosit);

			/*Cuts the used portion from vector.*/
			v = std::vector<unsigned char>(v.begin() + bytesPerPixel, v.end());
		}
		else {
			throw std::exception("Decompress got an invalid input.");
		}
	}

	/*If it found an inner node...*/
	else if (v[0] == treeData::hasChildren) {
		std::vector<unsigned char> temp;

		/*Sets 'temp' to be the rest of the vector without the hasChildren parameter.*/
		temp = std::vector<unsigned char>(v.begin() + 1, v.end());

		/*For each one of the children...*/
		for (int i = 0; i < divide && v.size(); i++) {
			/*Sets new absolut position.*/
			absPosit.push_back(i);

			/*Recursively calls itself to decompress the child.*/
			decompress(temp);

			/*Sets vector to new parameters.*/
			v = temp;

			/*Removes the last parameter of the absolut position corresponding to the child.*/
			absPosit.pop_back();
		}
	}
	else
		throw std::exception("Decompress got an invalid input.");
}

/*Encodes raw data to file.*/
void QuadTree::encodeRaw(const char* fileName) {
	/*Allocates memory for data array and checks for errors.*/
	unsigned char* updatedImg = (unsigned char*)malloc(decompressed.size() * sizeof(unsigned char));
	if (!updatedImg)
		throw std::exception("Failed to allocate memory for compressed image.");

	/*Loads data array with data from the decompressed vector.*/
	for (unsigned int i = 0; i < decompressed.size(); i++) {
		updatedImg[i] = decompressed.at(i);
	}
	/*Sets size equal to width/height (in pixels) of the data.*/
	unsigned int size = static_cast<unsigned int> (sqrt(decompressed.size() / bytesPerPixel));

	/*Encodes data to file and checks for errors.*/
	int error = lodepng_encode32_file(fileName, updatedImg, size, size);
	if (error) {
		std::string errStr = "Failed to encode raw file. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	/*Frees resources.*/
	if (updatedImg)
		free(updatedImg);
}

/*Fills a given portion of the decompressed vector.*/
void QuadTree::fillDecompressedVector(int* rgb, const std::list<int>& absPosit) {
	/*Level is the depth in the tree.*/
	unsigned int level = absPosit.size();

	/*Size is the total amount of data corresponding to the portion to fill.*/
	unsigned int size = decompressed.size() / pow(divide, level);

	/*newWidth/newHeight are the sides of the square to fill.*/
	unsigned int newWidth = sqrt(size * bytesPerPixel);
	unsigned int newHeight = newWidth / bytesPerPixel;

	/*Sets initial column and row to 0.
	It will move from there to the actual position to fill.*/
	unsigned int initCol = 0;
	unsigned int initRow = 0;

	unsigned int tempSize;
	unsigned int counter = 0;
	unsigned int halfCol, halfRow;

	/*For each value of the absolut position...*/
	for (const auto& x : absPosit) {
		/*There is a new division, therefore a new total size.*/
		tempSize = decompressed.size() / pow(divide, counter);
		counter++;

		/*There are new side lengths of the division.*/
		halfCol = sqrt(tempSize * bytesPerPixel) / 2;
		halfRow = halfCol / bytesPerPixel;

		/*So the current row/column changes accordingly. */
		initCol += (x % 2) * halfCol;
		initRow += (x / 2) * halfRow;
	}

	unsigned int iter = 0;

	/*For each row in the square to fill...*/
	for (unsigned int i = initRow; i < initRow + newHeight; i++) {
		/*For each column in the square to fill...*/
		for (unsigned int j = initCol; j < initCol + newWidth; j++) {
			/*Loads decompressed with alpha if it's the bytesPerPixel - 1 time.*/
			if (iter == (bytesPerPixel - 1)) {
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = alfa;
				iter = -1;
			}

			/*Otherwise, it loads decompressed with the corresponding value of RGB.*/
			else
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = rgb[iter];
			iter++;
		}
	}
}

QuadTree::~QuadTree() {}