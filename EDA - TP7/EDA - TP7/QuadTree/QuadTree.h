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
	void compress(const std::vector<unsigned char>&);
	void encodeCompressed(const char*);

	unsigned int findNearestMultiple(unsigned int, unsigned int);
	const std::vector<unsigned char> cutVector(const std::vector<unsigned char>&, int);
	bool lessThanThreshold(const std::vector<unsigned char>&);

	/*Decompression*/
	void encodeRaw(const char*);
	void decompress(std::vector<unsigned char>&);
	void decodeCompressed(const char*);

	void fillDecompressedVector(int*, const std::list<int>&);

	/*Prevents from using copy constructor.*/
	QuadTree(const QuadTree&);

	/*Data members.*/
	std::vector<unsigned char> originalData, decompressed, tree;
	std::vector <unsigned int> mean;
	double threshold;
	unsigned int width, height;
};