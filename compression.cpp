#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <bitset>
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
    for (size_t i = 0; i < mergedInput.size(); ++i) {
        char ch = mergedInput[i];
        freqMap[ch]++;
    }
    return freqMap;
}

// Function to build Huffman Tree
HuffmanNode* buildHuffmanTree(const std::unordered_map<char, int>& freqMap) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;

    for (std::unordered_map<char, int>::const_iterator it = freqMap.begin(); it != freqMap.end(); ++it) {
        pq.push(new HuffmanNode(it->first, it->second));
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

// Function to generate Huffman Codes
void generateCodes(HuffmanNode* root, std::string code, std::unordered_map<char, std::string>& huffmanCode) {
    if (!root) return;

    if (root->ch != '\0') {
        huffmanCode[root->ch] = code;
    }

    generateCodes(root->left, code + "0", huffmanCode);
    generateCodes(root->right, code + "1", huffmanCode);
}

// Function to save Huffman codes to separate file
void saveHuffmanCodes(const std::unordered_map<char, std::string>& huffmanCode, const std::string& filename) {
    std::ofstream outFile(filename);
    for (std::unordered_map<char, std::string>::const_iterator it = huffmanCode.begin(); it != huffmanCode.end(); ++it) {
        outFile << it->first << " " << it->second << "\n";
    }
    outFile.close();
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

// Function to encode the merged input using Huffman codes
std::string encodeMergedInput(const std::string& mergedInput, const std::unordered_map<char, std::string>& huffmanCode) {
    std::string encodedStr = "";
    for (size_t i = 0; i < mergedInput.size(); ++i) {
        char ch = mergedInput[i];
        encodedStr += huffmanCode.at(ch);
    }
    return encodedStr;
}

// Function to write the encoded bitstream into a binary file
void writeCompressedFile(const std::string& outFilename, std::string encodedStr) {
    std::ofstream outFile(outFilename, std::ios::binary);

    // Calculate the extra bits added for padding
    int extraBits = 8 - (encodedStr.size() % 8);
    if (extraBits != 8) {
        encodedStr.append(std::string(extraBits, '0'));  // Pad with '0's
    }

    outFile.put(static_cast<char>(extraBits));  // Store the number of extra bits for decompression

    for (size_t i = 0; i < encodedStr.size(); i += 8) {
        std::string byteStr = encodedStr.substr(i, 8);
        unsigned char byte = std::bitset<8>(byteStr).to_ulong();
        outFile.put(byte);
    }

    outFile.close();
}

int main() {
    std::string filename;
    std::cout << "Enter the name of the file to compress: ";
    std::cin >> filename;

    std::string mergedInput = mergeLinesWithMarker(filename);

    if (mergedInput.empty()) { // error handling
        std::cerr << "Error: Unable to process the file. Exiting..." << std::endl;
        return 1;
    }
    
    std::unordered_map<char, int> freqMap = calculateFrequencies(mergedInput);

    HuffmanNode* root = buildHuffmanTree(freqMap);

    std::unordered_map<char, std::string> huffmanCode;
    generateCodes(root, "", huffmanCode);

    saveHuffmanCodes(huffmanCode, "huffman_codes.txt");

    std::string encodedStr = encodeMergedInput(mergedInput, huffmanCode);

    std::string compressedFilename = "compressed.bin";
    writeCompressedFile(compressedFilename, encodedStr);

    std::cout << "File compressed successfully!" << std::endl;

    return 0;
}

