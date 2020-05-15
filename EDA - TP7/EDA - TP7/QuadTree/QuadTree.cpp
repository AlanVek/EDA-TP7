#include "QuadTree.h"
#include "lodepng.h"
//#include <fstream>
//#include <iostream>

/*Constants to use throughout program. */
namespace {
	const unsigned char alpha = 255;
	const unsigned int maxDif = 3 * 255;
	const unsigned int divide = 4;
	const unsigned int bytesPerPixel = 4;
}
namespace treeData {
	const enum : const unsigned char {
		hasChildren,
		noChildren,
		filling,
	};
}

/*QuadTree constructor. Saves format.*/
QuadTree::QuadTree(const std::string& format) {
	int pos = format.find_last_of('.');

	/*Saves format without initial '.'.*/
	this->format = format.substr(pos + 1, format.length() - pos);
}

/*******************************

		  Compression

*******************************/

/*Compresses image from input file to output file, with the given threshold. */
void QuadTree::compressAndSave(const std::string& input, const std::string& output, const double threshold) {
	if (threshold > 0 && threshold <= 1) {
		/*Transforms filenames into correct ones.*/
		const std::string realInput = parse(input, "png");
		const std::string realOutput = parse(output, format);

		/*Sets threshold.*/
		this->threshold = threshold * maxDif;

		/*Decodes raw data.*/
		decodeRaw(realInput);

		/*Checks validity of data format.*/
		checkData();

		/*Saves space for additional tree data and compresses file.*/
		tree = charVector(bytesPerPixel, treeData::filling);
		compress(originalData.begin(), width, height);

		/*Encodes compressed file.*/
		encodeCompressed(realOutput);
	}
	else
		throw std::exception("Threshold must be a non-negative value between 0 and 1.");
}

/*Decodes raw data from file.*/
void QuadTree::decodeRaw(const std::string& fileName) {
	unsigned char* img;

	/*Decodes file and checks for errors.*/
	int error = lodepng_decode32_file(&img, &width, &height, fileName.c_str());
	if (error) {
		std::string errStr = "Failed to decode file. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
	if (!img)
		throw std::exception("lodepng_decode32_file error.");

	/*Sets real width.*/
	width *= bytesPerPixel;

	/*Loads originalData with raw data from file.*/
	originalData.assign(img, img + width * height);

	/*Frees resources.*/
	if (img)
		free(img);
}

/*Checks validity of input data through width and height.*/
void QuadTree::checkData(void) {
	/*Checks for validity of width and height.*/
	if (width < 0 || height < 0)
		throw std::exception("Wrong input to compress.");

	/*Checks if it's an empty array.*/
	if (!(width * height))
		throw std::exception("File is empty or doesn't exist.");

	/*Checks if width is a power of 2.*/
	if (floor(log2(width / bytesPerPixel)) != log2(width / bytesPerPixel))
		throw std::exception("Width not in form of 2^n.");

	/*Checks if height is a power of 2.*/
	if (floor(log2(height)) != log2(height))
		throw std::exception("Height not in form of 2^n.");

	/*Checks if vector is square*/
	if (width / bytesPerPixel != height)
		throw std::exception("Image should be square.");

	/*Checks if vector has appropriate length.*/
	if (width * height < bytesPerPixel) {
		throw std::exception("Compress got invalid input.");
	}
}

/*Recursively compresses data to tree.*/
void QuadTree::compress(const iterator& start, unsigned int W, unsigned int H) {
	/*If it's only one pixel...*/
	if (W * H == bytesPerPixel) {
		/*Loads noChildren to tree and pushes RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		tree.insert(tree.end(), start, start + bytesPerPixel - 1);
	}

	/*If vector's RGB formula is less than threshold...*/
	else if (lessThanThreshold(start, W, H)) {
		/*Loads noChildren to tree and pushes mean RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		tree.insert(tree.end(), mean.begin(), mean.end());
	}

	/*Otherwise...*/
	else {
		/*Pushes hasChildren and compresses the vector in 'divide' parts.
		It's an inner node.*/
		tree.push_back(treeData::hasChildren);
		for (int i = 0; i < divide; i++)
			compress(getNewPosition(start, W, H, i), W / 2, H / 2);
	}
}

/*Encodes compressed data to file.*/
void QuadTree::encodeCompressed(const std::string& fileName) {
	/*Generates size that is a multiple of bytesPerPixel.*/
	unsigned int size = tree.size();
	while ((++size) % bytesPerPixel);

	/*Loads original image size in first position.The file can only take numbers
	from 0 to 255. As images are square and their sides are of the form 2^n,
	the chosen method was to set n as the size output.*/
	unsigned int offset = tree.size() - size + bytesPerPixel;
	tree[offset] = (unsigned char)log2(height);

	/*Encodes tree in file and checks for errors.*/
	int error = lodepng_encode32_file(fileName.c_str(), tree.data() + offset, size / bytesPerPixel, 1);
	if (error) {
		std::string errStr = "Failed to encode compressed file. Lodepng error: " + (std::string)lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
}

/*Checks if vector's RGB formula is less than threshold.
If it is, then it returns true and saves mean values to 'mean'.
Otherwise, it returns false.*/
bool QuadTree::lessThanThreshold(const iterator& start, unsigned int W, unsigned int H) {
	/*Checks for correct shape.*/
	if (W * H < bytesPerPixel)
		throw std::exception("lessThanThreshold got an invalid input.");

	/*Creates variables to use in function. Mexrgb saves max values of rgb and
	minrgb saves min values of rgb.*/
	mean = intVector(bytesPerPixel - 1);
	intVector maxrgb(start, start + bytesPerPixel - 1);
	intVector minrgb = maxrgb;
	bool result = false;
	int count = -1;

	/*Loops through the portion of the vector, applying checks to each
	value (is it the minimum? is it the maximum?).*/
	for (unsigned int i = 0; i < H; i++) {
		for (unsigned int j = 0; j < W; j++) {
			/*If it's not alpha...*/
			if ((++count) != bytesPerPixel - 1) {
				auto value = *(start + i * width + j);

				/*If value is higher than corresponding value
				in maxrgb, it changes it.*/
				if (value > maxrgb[count])
					maxrgb[count] = value;

				/*If value is lower than corresponding value
				in maxrgb, it changes it.*/
				else if (value < minrgb[count])
					minrgb[count] = value;

				/*Updates mean.*/
				mean[count] += value;
			}
			/*Resets count when it reached bytesPerPixel - 1.*/
			else
				count = -1;
		}
	}

	unsigned int temp = 0;

	/*Applies formula.*/
	for (unsigned int i = 0; i < bytesPerPixel - 1; i++)
		temp += maxrgb[i] - minrgb[i];

	/*Checks if formula is lower than threshold. */
	if (temp <= threshold) {
		for (int i = 0; i < bytesPerPixel - 1; i++)
			mean[i] /= (W * H / bytesPerPixel);
		result = true;
	}
	return result;
}

/*Returns an iterator pointing to the start of a part of the given vector
according to the parameter 'which'.*/
const iterator QuadTree::getNewPosition(const iterator& start, unsigned int W, unsigned int H, unsigned int which) const {
	/*Checks validity of input.*/
	if (which < 0 || which >= divide || W < 0 || H < 0)
		throw std::exception("Wrong input in getNewPosition.");

	/*If the vector is empty, it returns its start. It can't be cut.*/
	if (!W || !H)
		return start;

	/*Sets new row and column.*/
	unsigned int row = (which / 2) * H / 2;
	unsigned int col = (which % 2) * W / 2;

	/*Returns an interator pointing to said position.*/
	return start + row * width + col;
}

/*******************************

		  Decompression

*******************************/

/*Decompresses the input file and saves it to output file. */
void QuadTree::decompressAndSave(const std::string& input, const std::string& output) {
	const std::string realInput = parse(input, format);
	const std::string realOutput = parse(output, "png");

	/*Decodes compressed file.*/
	decodeCompressed(realInput);

	/*Decompresses file.*/
	auto start = originalData.begin();
	decompress(start);

	/*Encodes raw data file.*/
	encodeRaw(realOutput);
}

/*Decodes compressed data from file. */
void QuadTree::decodeCompressed(const std::string& fileName) {
	unsigned char* img;

	/*Decodes data and checks for errors.*/
	int error = lodepng_decode32_file(&img, &width, &height, fileName.c_str());
	if (error) {
		std::string errStr = "Failed to decode compressed file. Lodepng error: " + (std::string)lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	if (!img)
		throw std::exception("lodepng_decode32_file error");

	/*Sets real width.*/
	width *= bytesPerPixel;

	/*Sets real size of original image.*/
	unsigned int size = static_cast<unsigned int> (pow(pow(2, img[0]), 2) * bytesPerPixel);

	/*Goes to end of 'fill' portion of data.*/
	unsigned int index = 1;
	for (; img[index] == treeData::filling; index++) {};

	/*Sets decompressed to correct size.*/
	decompressed = charVector(size);

	/*Loads compressed data to originalData.*/
	originalData.assign(img + index, img + width * height);

	/*Frees resources.*/
	if (img)
		free(img);
}

/*Decompresses data from vector. */
void QuadTree::decompress(iterator& ptr) {
	/*Sets static list to check for absolut position within data.*/
	static intVector absPosit;

	/*If it found a leaf...*/
	if (*ptr == treeData::noChildren) {
		int temp[bytesPerPixel - 1];

		/*'Removes' flag from vector.*/
		ptr++;
		/*Loads 'temp' with RGB data and 'removes' RGB data from vector.*/
		for (unsigned int i = 0; i < bytesPerPixel - 1; i++) {
			temp[i] = *ptr;
			ptr++;
		}

		/*Fills the corresponding section of 'decompressed' with the RGB data.*/
		fillDecompressedVector(temp, absPosit);
	}

	/*If it found an inner node...*/
	else if (*ptr == treeData::hasChildren) {
		ptr++;
		/*For each one of the children...*/
		for (int i = 0; i < divide; i++) {
			/*Sets new absolut position.*/
			absPosit.push_back(i);

			/*Recursively calls itself to decompress the child.*/
			decompress(ptr);

			/*Removes the last parameter of the absolut position corresponding to the child.*/
			absPosit.pop_back();
		}
	}
	else
		throw std::exception("Decompress got an invalid input.");
}

/*Encodes raw data to file.*/

void QuadTree::encodeRaw(const std::string& fileName) const {
	/*Sets size equal to width-height (in pixels) of the data.*/
	unsigned int size = static_cast<unsigned int> (sqrt(decompressed.size() / bytesPerPixel));

	/*Encodes data to file and checks for errors.*/
	int error = lodepng_encode32_file(fileName.c_str(), decompressed.data(), size, size);
	if (error) {
		std::string errStr = "Failed to encode raw file. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
}

/*Fills a given portion of the decompressed vector.*/
void QuadTree::fillDecompressedVector(int* rgb, const intVector& absPosit) {
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
			/*Loads decompressed with alpha if it's the corresponding position.*/
			if (iter == (bytesPerPixel - 1)) {
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = alpha;
				iter = -1;
			}

			/*Otherwise, it loads decompressed with the corresponding value of RGB.*/
			else
				decompressed[i * sqrt(decompressed.size() * bytesPerPixel) + j] = rgb[iter];
			iter++;
		}
	}
}

/*Returns a usable string to use as filename, according to the specified format.*/
const std::string QuadTree::parse(const std::string& filename, const std::string& Format) const {
	/*If filename has no specified format, it returns the same string plus its format.*/
	if (filename.find('.') == std::string::npos)
		return filename + '.' + Format;

	/*If filename has the correct format, it returns the same filename.*/
	if (filename.find('.' + Format) != std::string::npos)
		return filename;

	const std::string errStr = "Wrong image format. Expected either no format or ." + Format + '.';
	throw std::exception(errStr.c_str());
}

QuadTree::~QuadTree() {}