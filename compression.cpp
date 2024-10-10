#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <sstream>  // for merging lines

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char character, int frequency) : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};

// Custom comparator for priority queue (min-heap)
struct Compare {
    bool operator()(HuffmanNode* left, HuffmanNode* right) {
        return left->freq > right->freq;
    }
};

// Function to calculate the frequency of characters in the input file
std::unordered_map<char, int> calculateFrequencies(const std::string& mergedInput) {
    std::unordered_map<char, int> freqMap;
    for (char ch : mergedInput) {
        freqMap[ch]++;
    }
    return freqMap;
}

// Function to merge multiple lines into a single line, replacing '/n' with the actual characters '/n'
std::string mergeLinesWithMarker(const std::string& filename) {
    std::ifstream file(filename);
    std::string mergedInput, line;

    while (std::getline(file, line)) {
        mergedInput += line + "/n";  // Add the line and replace newline with '/n'
    }
    file.close();

    return mergedInput;
}

// Function to build Huffman Tree
HuffmanNode* buildHuffmanTree(const std::unordered_map<char, int>& freqMap) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

    for (auto pair : freqMap) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    while (pq.size() != 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();

        int sumFreq = left->freq + right->freq;
        HuffmanNode* newNode = new HuffmanNode('\0', sumFreq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    return pq.top(); // root of Huffman Tree
}

int main() {
    std::string filename = "input.txt";

    std::string mergedInput = mergeLinesWithMarker(filename);

    std::unordered_map<char, int> freqMap = calculateFrequencies(mergedInput);

    HuffmanNode* root = buildHuffmanTree(freqMap);

    return 0;
}
