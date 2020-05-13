#pragma once
#include <vector>

//Just to see the vectors while testing the program.
auto print = [](const auto& iter) { for (const auto& x : iter)std::cout << x << ' '; std::cout << std::endl; };

class QuadTree {
public:
	QuadTree();

	const std::vector<unsigned int>& getOriginalData(void) const;
	const std::vector<unsigned int>& getTree(void) const;

	~QuadTree();
	void compressAndSave(const char*, const char*);

	void decompressAndSave(const char*, const char*);

private:
	void encodeCompressed(const char*);
	void encodeRaw(const char*);
	void decodeCompressed(const char*);
	void decodeRaw(const char*);

	void compress(const std::vector<unsigned int>&);
	void decompress(const std::vector<unsigned int>&);

	const std::vector<unsigned int> cutVector(const std::vector<unsigned int>&, int);

	QuadTree(const QuadTree&);

	bool lessThanThreshold(const std::vector<unsigned int>&);

	std::vector<unsigned int> originalData, decompressed, tree;

	unsigned int width, height;

	std::vector<unsigned int> mean;
};