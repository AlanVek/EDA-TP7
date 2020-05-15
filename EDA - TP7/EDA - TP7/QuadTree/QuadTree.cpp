#include "QuadTree.h"
#include "lodepng.h"

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

/*QuadTree constructor. Sets mean and threshold to 0, and saves format.*/
QuadTree::QuadTree(const std::string& format_) {
	int pos = format_.find_last_of('.');

	/*Saves format without initial '.'.*/
	if (pos != std::string::npos)
		format = format_.substr(pos + 1, format_.length() - pos);
	else
		format = format_;
	mean = intVector(bytesPerPixel - 1);
	threshold = NULL;
}

/*******************************

		  Compression

*******************************/

/*Compresses image from input file to output file, with the given threshold. */
void QuadTree::compressAndSave(const std::string& input, const std::string& output, const double threshold_) {
	if (threshold_ > 0 && threshold_ <= 1) {
		/*Transforms filenames into correct ones.*/
		const std::string realInput = parse(input, "png");
		const std::string realOutput = parse(output, format);

		tree.clear();

		/*Sets threshold.*/
		threshold = threshold_ * maxDif;

		/*Decodes raw data.*/
		decodeRaw(realInput);

		/*Compresses file.*/
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
	originalData.clear();
	for (unsigned int i = 0; i < width * height; i++)
		originalData.push_back(img[i]);

	/*Frees resources.*/
	if (img)
		free(img);
}

/*Recursively compresses data to tree.*/
void QuadTree::compress(const iterator& start, unsigned int W, unsigned int H) {
	/*Checks if vector is empty.*/

	if (W < 0 || H < 0)
		throw std::exception("Wrong input to compress.");
	if (!(W * H))
		throw std::exception("File is empty or doesn't exist.");

	/*Checks if width is a power of 2.*/
	if (floor(log2(W / bytesPerPixel)) != log2(W / bytesPerPixel))
		throw std::exception("Width not in form of 2^n.");

	/*Checks if height is a power of 2.*/
	if (floor(log2(H)) != log2(H))
		throw std::exception("Height not in form of 2^n.");

	/*Checks if vector is square*/
	if (W / bytesPerPixel != H)
		throw std::exception("Image should be square.");

	/*Checks if vector has appropriate length.*/
	else if (W * H < bytesPerPixel) {
		throw std::exception("Compress got invalid input.");
	}

	/*If it's only one pixel...*/
	else if (W * H == bytesPerPixel) {
		/*Loads noChildren to tree and pushes RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		for (unsigned int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(*(start + i));
	}

	/*If vector's RGB formula is less than threshold...*/
	else if (lessThanThreshold(start, W, H)) {
		/*Loads noChildren to tree and pushes mean RGB code. It's a leaf.*/
		tree.push_back(treeData::noChildren);
		for (int i = 0; i < bytesPerPixel - 1; i++)
			tree.push_back(mean.at(i));
	}

	/*Otherwise...*/
	else {
		/*It pushes hasChildren and compresses the vector in 'divide' parts.
		It's an inner node.*/
		tree.push_back(treeData::hasChildren);
		for (int i = 0; i < divide; i++)
			compress(getNewPosition(start, W, H, i), W / 2, H / 2);
	}
}

/*Encodes compressed data to file.*/
void QuadTree::encodeCompressed(const std::string& fileName) const {
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
	for (unsigned int i = 0; i < tree.size(); i++)
		encoded[i + size - tree.size()] = tree.at(i);

	/*Encodes compressed data in file and checks for errors.*/
	int error = lodepng_encode32_file(fileName.c_str(), encoded, size / bytesPerPixel, 1);
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
bool QuadTree::lessThanThreshold(const iterator& start, unsigned int W, unsigned int H) {
	/*Checks for correct shape.*/
	if (W * H < bytesPerPixel)
		throw std::exception("lessThanThreshold got an invalid input.");

	/*Creates variables to use in function. Mexrgb saves max values of rgb and
	minrgb saves min values of rgb.*/

	mean = intVector(bytesPerPixel - 1);
	intVector maxrgb = intVector(start, start + bytesPerPixel - 1);
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
				if (value < minrgb[count])
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
iterator QuadTree::getNewPosition(const iterator& start, unsigned int W, unsigned int H, unsigned int which) const {
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

/*Finds the nearest multiple of base that is higher than number.*/
unsigned int QuadTree::findNearestMultiple(unsigned int number, unsigned int base) const {
	unsigned int temp = number + 1;
	while (temp % base)
		temp++;
	return temp;
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

	/*Gets size parameter from first position of data.*/
	unsigned int size = img[0];

	/*Goes to end of 'fill' portion of data.*/
	unsigned int index = 1;
	for (; img[index] == treeData::filling; index++) {};

	/*Sets real size of original image.*/
	size = static_cast<unsigned int> (pow(pow(2, size), 2) * bytesPerPixel);

	/*Sets decompressed to correct size.*/
	decompressed = charVector(size);

	originalData.clear();

	/*Loads compressed data to originalData.*/
	for (; index < width * height; index++)
		originalData.push_back(img[index]);

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
	int error = lodepng_encode32_file(fileName.c_str(), updatedImg, size, size);
	if (error) {
		std::string errStr = "Failed to encode raw file. Lodepng error: " + (std::string) lodepng_error_text(error);
		throw std::exception(errStr.c_str());
	}

	/*Frees resources.*/
	if (updatedImg)
		free(updatedImg);
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