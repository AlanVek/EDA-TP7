#pragma once
#include <vector>
#include <string>

using charVector = std::vector<unsigned char>;
using iterator = charVector::iterator;
using intVector = std::vector<unsigned int>;

class QuadTree {
public:
	QuadTree(const std::string&);

	~QuadTree();
	void compressAndSave(const std::string&, const std::string&, const double);

	void decompressAndSave(const std::string&, const std::string&);

private:

	/*Compression*/
	void decodeRaw(const std::string&);
	void compress(const iterator&, unsigned int, unsigned int);
	void encodeCompressed(const std::string&);

	unsigned int findNearestMultiple(unsigned int, unsigned int);
	iterator cutVector(const iterator&, unsigned int, unsigned int, unsigned int);
	bool lessThanThreshold(const iterator&, unsigned int, unsigned int);

	/*Decompression*/
	void encodeRaw(const std::string&);
	void decompress(iterator&);
	void decodeCompressed(const std::string&);

	void fillDecompressedVector(int*, const intVector&);

	const std::string parse(const std::string&, const std::string&);

	/*Prevents from using copy constructor.*/
	QuadTree(const QuadTree&) {};

	/*Data members.*/
	charVector originalData, decompressed, tree;
	intVector mean;
	double threshold;
	unsigned int width, height;
	std::string format;
};