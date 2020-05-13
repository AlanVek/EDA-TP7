#pragma once
#include <vector>

class QuadTree {
public:
	QuadTree();
	QuadTree(const char*);

	const std::vector<unsigned int>& getOriginalData(void) const;
	const std::vector<int>& getTree(void) const;

	void encode(const char*);

	~QuadTree();
	void startCompression(void);

private:
	void compress(const std::vector<unsigned int>&);

	const std::vector<unsigned int> cutVector(const std::vector<unsigned int>&, int);

	QuadTree(const QuadTree&);

	bool lessThanThreshold(const std::vector<unsigned int>&);

	std::vector<unsigned int> originalData;
	std::vector<int> tree;

	unsigned int originalWidth, originalHeight;

	std::vector<unsigned int> mean;
};