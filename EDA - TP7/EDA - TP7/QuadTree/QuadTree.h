#pragma once
#include <vector>

using charVector = std::vector<unsigned char>;
using iterator = charVector::iterator;
using intVector = std::vector<unsigned int>;

class QuadTree {
public:
	QuadTree();

	~QuadTree();
	void compressAndSave(const char*, const char*, const double);

	void decompressAndSave(const char*, const char*);

private:

	/*Compression*/
	void decodeRaw(const char*);
	void compress(const iterator&, unsigned int, unsigned int);
	void encodeCompressed(const char*);

	unsigned int findNearestMultiple(unsigned int, unsigned int);
	iterator cutVector(const iterator&, unsigned int, unsigned int, unsigned int);
	bool lessThanThreshold(const iterator&, unsigned int, unsigned int);

	/*Decompression*/
	void encodeRaw(const char*);
	void decompress(iterator&);
	void decodeCompressed(const char*);

	void fillDecompressedVector(int*, const intVector&);

	/*Prevents from using copy constructor.*/
	QuadTree(const QuadTree&) {};

	/*Data members.*/
	charVector originalData, decompressed, tree;
	intVector mean;
	double threshold;
	unsigned int width, height;
};