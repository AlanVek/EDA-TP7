#pragma once
#include <string>
#include <vector>

class QuadTree {
public:
	QuadTree();
	QuadTree(const std::string&);

	~QuadTree();
	void compressAndSave(const std::string&, const std::string&, const double);

	void decompressAndSave(const std::string&, const std::string&);

	void setFormat(const std::string&);

private:

	/*Compression*/
	/***********************************************************************************************************/
	void decodeRaw(const std::string&);
	void checkData(void) const;
	void compress(const unsigned char*, unsigned int, unsigned int);
	void encodeCompressed(const std::string&);

	const unsigned char* getNewPosition(const unsigned char*, unsigned int, unsigned int, unsigned int) const;
	bool lessThanThreshold(const unsigned char*, unsigned int, unsigned int);
	/***********************************************************************************************************/

	/*Decompression*/
	/***********************************************************************/
	void encodeRaw(const std::string&);
	void decompress(unsigned char**);
	void decodeCompressed(const std::string&);

	void fillDecompressedVector(const unsigned char*, const std::vector<unsigned int>&);
	/***********************************************************************/

	/*Data input verifier.*/
	const std::string parse(const std::string&, const std::string&) const;

	/*Prevents from using copy constructor.*/
	QuadTree(const QuadTree&);

	/*Data members.*/
	/***********************************************/

	/*Image info.*/
	std::vector<unsigned char> tree;
	std::vector<unsigned int> absPosit;
	std::vector<float> mean;
	unsigned char* inputFile, * outputFile;

	/*Flags.*/
	unsigned int realsize, width, height, index;

	/*User input.*/
	double threshold;
	std::string format;
	/***********************************************/
};