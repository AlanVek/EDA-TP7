#pragma once
#include <vector>
#include <list>

class QuadTree {
public:
	QuadTree();

	~QuadTree();
	void compressAndSave(const char*, const char*, const double);

	void decompressAndSave(const char*, const char*);

private:

	/*Compression*/
	void decodeRaw(const char*);
	void encodeCompressed(const char*);
	unsigned int findNearestMultiple(unsigned int, unsigned int);
	void compress(const std::vector<unsigned char>&);
	const std::vector<unsigned char> cutVector(const std::vector<unsigned char>&, int);
	bool lessThanThreshold(const std::vector<unsigned char>&);

	/*Decompression*/
	void decodeCompressed(const char*);
	void encodeRaw(const char*);
	void fillCompressedVector(int*, const std::list<int>&);
	void decompress(std::vector<unsigned char>&);

	/*Prevents from using copy constructor.*/
	QuadTree(const QuadTree&);

	/*Data members.*/
	std::vector<unsigned char> originalData, decompressed, tree;
	double threshold;
	unsigned int width, height;
	std::vector<unsigned char> mean;
};