#pragma once
#include <vector>

//Just to see the vectors while testing the program.
auto print = [](const auto& iter) { for (const auto& x : iter)std::cout << (int)x << ' '; std::cout << std::endl; };

class QuadTree {
public:
	QuadTree();

	const std::vector<unsigned char>& getOriginalData(void) const;
	const std::vector<unsigned char>& getTree(void) const;

	~QuadTree();
	void compressAndSave(const char*, const char*);

	void decompressAndSave(const char*, const char*);

private:
	void encodeCompressed(const char*);
	void encodeRaw(const char*);
	void decodeCompressed(const char*);
	void decodeRaw(const char*);

	void compress(const std::vector<unsigned char>&);
	void decompress(const std::vector<unsigned char>&);

	const std::vector<unsigned char> cutVector(const std::vector<unsigned char>&, int);

	QuadTree(const QuadTree&);

	bool lessThanThreshold(const std::vector<unsigned char>&);

	std::vector<unsigned char> originalData, decompressed, tree;

	unsigned int width, height;

	std::vector<unsigned char> mean;
};