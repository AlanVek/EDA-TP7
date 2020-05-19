#include "QuadTree.h"
#include "lodepng.h"

/*Constants to use throughout program. */
/********************************************/
namespace {
	const unsigned char minVal = 0;
	const unsigned char maxVal = 255;
	const unsigned char alpha = maxVal;
	const unsigned int maxDif = 3 * maxVal;
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
/********************************************/

QuadTree::QuadTree() : inputFile(nullptr), outputFile(nullptr), height(0), width(0), threshold(0), realsize(0) {};

/*QuadTree constructor. Saves format.*/
QuadTree::QuadTree(const std::string& format) : inputFile(nullptr), outputFile(nullptr), height(0), width(0), threshold(0), realsize(0)
{
	setFormat(format);
}

/*Sets new inputFile format for compression.*/
void QuadTree::setFormat(const std::string& format) {
	int pos = format.find_last_of('.');

	/*Saves format without initial '.'.*/
	this->format = format.substr(pos + 1, format.length() - pos);
}

/*******************************

		  Compression

*******************************/

/*Compresses image from input inputFile to output inputFile, with the given threshold. */
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

		/*Saves space for additional tree data and compresses inputFile.*/
		tree.assign(bytesPerPixel, treeData::filling);
		compress(inputFile, width, height);

		/*Encodes compressed inputFile.*/
		encodeCompressed(realOutput);

		/*Frees memory.*/
		if (inputFile) {
			free(inputFile);
			inputFile = nullptr;
		}
	}
	else
		throw std::exception("Threshold must be a non-negative value between 0 and 1.");
}

/*Decodes raw data from inputFile.*/
void QuadTree::decodeRaw(const std::string& fileName) {
	/*Decodes inputFile and checks for errors.*/
	int error = lodepng_decode32_file(&inputFile, &width, &height, fileName.c_str());
	if (error) {
		std::string errStr = "Failed to decode inputFile. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
	if (!inputFile)
		throw std::exception("lodepng_decode32_file error.");

	/*Sets real width.*/
	width *= bytesPerPixel;
}

/*Checks validity of input data through width and height.*/
void QuadTree::checkData(void) const {
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
void QuadTree::compress(const unsigned char* start, unsigned int W, unsigned int H) {
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

/*Encodes compressed data to inputFile.*/
void QuadTree::encodeCompressed(const std::string& fileName) {
	/*Generates size that is a multiple of bytesPerPixel.*/
	unsigned int size = tree.size();
	while ((++size) % bytesPerPixel);

	/*Loads original image size in first position.The inputFile can only take numbers
	from 0 to 255. As images are square and their sides are of the form 2^n,
	the chosen method was to set n as the size output.*/
	unsigned int offset = tree.size() - size + bytesPerPixel;
	tree[offset] = (unsigned char)log2(height);

	/*Encodes tree in inputFile and checks for errors.*/
	int error = lodepng_encode32_file(fileName.c_str(), tree.data() + offset, size / bytesPerPixel, 1);
	if (error) {
		std::string errStr = "Failed to encode compressed inputFile. Lodepng error: " + (std::string)lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
}

/*Checks if vector's RGB formula is less than threshold.
If it is, then it returns true and saves mean values to 'mean'.
Otherwise, it returns false.*/
bool QuadTree::lessThanThreshold(const unsigned char* start, unsigned int W, unsigned int H) {
	/*Checks for correct shape.*/
	if (W * H < bytesPerPixel)
		throw std::exception("lessThanThreshold got an invalid input.");

	/*Creates variables to use in function. Mexrgb saves max values of rgb and
	minrgb saves min values of rgb.*/
	mean.assign(bytesPerPixel - 1, 0);
	std::vector<unsigned int> maxrgb(bytesPerPixel - 1, minVal);
	std::vector<unsigned int> minrgb(bytesPerPixel - 1, maxVal);
	int count = -1;

	unsigned int value;

	/*Loops through the portion of the vector, applying checks to each
	value (is it the minimum? is it the maximum?).*/
	for (unsigned int i = 0; i < H; i++) {
		for (unsigned int j = 0; j < W; j++) {
			/*If it's not alpha...*/
			if ((++count) != bytesPerPixel - 1) {
				value = *(start + i * width + j);

				/*If value is higher than corresponding value
				in maxrgb, it changes it.*/
				if (value > maxrgb[count])
					maxrgb[count] = value;

				/*If value is lower than corresponding value
				in maxrgb, it changes it.*/
				else if (value < minrgb[count])
					minrgb[count] = value;

				/*Updates mean.*/
				mean[count] += value * bytesPerPixel / (float)(W * H);
			}
			/*Resets count when it reached bytesPerPixel - 1.*/
			else
				count = -1;
		}
	}

	value = 0;
	/*Applies formula.*/
	for (unsigned int i = 0; i < bytesPerPixel - 1; i++)
		value += maxrgb[i] - minrgb[i];

	return value <= threshold;
}

/*Returns an iterator pointing to the start of a part of the given vector
according to the parameter 'which'.*/
const unsigned char* QuadTree::getNewPosition(const unsigned char* start, unsigned int W, unsigned int H, unsigned int which) const {
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

/*Decompresses the input inputFile and saves it to output inputFile. */
void QuadTree::decompressAndSave(const std::string& input, const std::string& output) {
	const std::string realInput = parse(input, format);
	const std::string realOutput = parse(output, "png");

	/*Decodes compressed inputFile.*/
	decodeCompressed(realInput);

	/*Decompresses inputFile.*/
	unsigned char* substitute = inputFile;
	decompress(&substitute);

	/*Resets absolute position vector.*/
	absPosit.clear();

	/*Encodes raw data inputFile.*/
	encodeRaw(realOutput);

	/*Returns to original position.*/
	inputFile -= index;

	/*Frees memory.*/
	if (inputFile) {
		free(inputFile);
		inputFile = nullptr;
	}
	if (outputFile) {
		free(outputFile);
		outputFile = nullptr;
	}
}

/*Decodes compressed data from inputFile. */
void QuadTree::decodeCompressed(const std::string& fileName) {
	//unsigned char* img;

	/*Decodes data and checks for errors.*/
	int error = lodepng_decode32_file(&inputFile, &width, &height, fileName.c_str());
	if (error) {
		std::string errStr = "Failed to decode compressed inputFile. Lodepng error: " + (std::string)lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	if (!inputFile)
		throw std::exception("lodepng_decode32_file error");

	/*Sets real width.*/
	width *= bytesPerPixel;

	/*Sets real size of original image.*/
	realsize = (unsigned int)(pow(pow(2, inputFile[0]), 2) * bytesPerPixel);

	/*Goes to end of 'fill' portion of data. Saves offset in 'index' to
	return when decompression is finished.*/
	index = 1;
	for (; inputFile[index] == treeData::filling; index++) {};
	inputFile += index;

	/*Allocates space for decompressed file.*/
	outputFile = (unsigned char*)malloc(realsize * sizeof(unsigned char));
}

/*Decompresses data from vector. */
void QuadTree::decompress(unsigned char** ptr) {
	/*If it found a leaf...*/
	if (**ptr == treeData::noChildren) {
		/*'Removes' flag from vector.*/
		(*ptr)++;

		/*Fills the corresponding section of 'decompressed' with the RGB data.*/
		fillDecompressedVector((*ptr), absPosit);
		(*ptr) += bytesPerPixel - 1;
	}

	/*If it found an inner node...*/
	else if (**ptr == treeData::hasChildren) {
		(*ptr)++;
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

/*Encodes raw data to outputFile.*/
void QuadTree::encodeRaw(const std::string& fileName) const {
	/*Sets size equal to width-height (in pixels) of the data.*/
	unsigned int size = static_cast<unsigned int> (sqrt(realsize / bytesPerPixel));

	/*Encodes data to outputFile and checks for errors.*/
	int error = lodepng_encode32_file(fileName.c_str(), outputFile, size, size);
	if (error) {
		std::string errStr = "Failed to encode raw inputFile. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}
}

/*Fills a given portion of the decompressed vector.*/
void QuadTree::fillDecompressedVector(const unsigned char* rgb, const std::vector<unsigned int>& absPosit) {
	/*There is a new total amount of data corresponding to the portion to fill.
	It depends on the depth of the leaf within the tree, which is equal to the
	size of the absPosit vector. newWidth is the horizontal side of the square to fill.*/
	unsigned int fullWidth = (unsigned int)sqrt((double)bytesPerPixel * realsize);
	unsigned int newWidth = fullWidth / (unsigned int)pow(divide, absPosit.size() / 2.0);

	/*Sets initial column and row to 0.
	It will move from there to the actual position to fill.*/
	unsigned int initCol = 0;
	unsigned int initRow = 0;

	unsigned int Col;

	/*For each value of the absolut position...*/
	unsigned int i = 0;
	for (const auto& pos : absPosit)
		/*for (unsigned int i = 0; i < absPosit.size(); i++*/ {
		/*There is a new division, therefore a new total size and new side lengths of the division.*/
		Col = fullWidth / (unsigned int)(2 * pow(divide, i / 2.0));

		/*So the current row/column changes accordingly. */
		initCol += (pos % 2) * Col;
		initRow += (pos / 2) * Col / bytesPerPixel;;
		i++;
	}

	/*For each row in the square to fill...*/
	for (unsigned int i = initRow; i < initRow + newWidth / bytesPerPixel; i++) {
		/*For each column in the square to fill...*/
		for (unsigned int j = initCol; j < initCol + newWidth; j++) {
			/*Loads decompressed with alpha if it's the corresponding position.*/
			if (j % bytesPerPixel == (bytesPerPixel - 1))
				outputFile[i * fullWidth + j] = alpha;

			/*Otherwise, it loads decompressed with the corresponding value of RGB.*/
			else
				outputFile[i * fullWidth + j] = rgb[j % bytesPerPixel];
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

/*Frees memory if it hasn't already been freed.*/
QuadTree::~QuadTree() {
	if (inputFile)
		free(inputFile);
	if (outputFile)
		free(outputFile);
}