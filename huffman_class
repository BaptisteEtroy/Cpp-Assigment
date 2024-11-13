#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
using namespace std;

// Node structure for Huffman tree
typedef struct MinHeapNode {
    char data;
    unsigned freq;
    MinHeapNode *left, *right;
} Node;

struct Compare {
    bool operator()(Node* left, Node* right) {
        return left->freq > right->freq;
    }
};

class Huffman {
public:
    void buildHuffmanTree(const string& inputFile, const string& codeFile) {
        ifstream inFile(inputFile.c_str());
        ofstream codesFile(codeFile.c_str());

        if (!inFile.is_open() || !codesFile.is_open()) {
            cerr << "Error opening file(s)" << endl;
            return;
        }

        string data((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        unordered_map<char, int> frequency;

        // Calculate frequency of each character
        for (size_t i = 0; i < data.length(); ++i) {
            char ch = data[i];
            frequency[ch]++;
        }

        // Create priority queue for Huffman tree
        priority_queue<Node*, vector<Node*>, Compare> minHeap;

        // Create leaf nodes
        for (unordered_map<char, int>::iterator it = frequency.begin(); it != frequency.end(); ++it) {
            Node* node = new Node;
            node->data = it->first;
            node->freq = it->second;
            node->left = nullptr;
            node->right = nullptr;
            minHeap.push(node);
        }

        // Build Huffman Tree
        while (minHeap.size() > 1) {
            Node *left = minHeap.top(); minHeap.pop();
            Node *right = minHeap.top(); minHeap.pop();
            Node *newNode = new Node;
            newNode->data = '\0';
            newNode->freq = left->freq + right->freq;
            newNode->left = left;
            newNode->right = right;
            minHeap.push(newNode);
        }

        root = minHeap.top();

        // Generate codes
        generateCodes(root, "", huffmanCodes);

        // Write codes to file
        for (unordered_map<char, string>::iterator it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
            codesFile << it->first << " " << it->second << "\n";
        }

        inFile.close();
        codesFile.close();
    }

    void compression(const string& inputFile, const string& outputFile) {
        ifstream inFile(inputFile.c_str());
        ofstream outFile(outputFile.c_str());

        if (!inFile.is_open() || !outFile.is_open()) {
            cerr << "Error opening file(s)" << endl;
            return;
        }

        string data((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        string encodedData;
        for (size_t i = 0; i < data.length(); ++i) {
            char ch = data[i];
            encodedData += huffmanCodes[ch];
        }

        outFile << encodedData;

        inFile.close();
        outFile.close();
    }

    void decompression(const string& inputFile, const string& outputFile, const string& codeFile) {
        ifstream inFile(inputFile.c_str());
        ofstream outFile(outputFile.c_str());
        ifstream codesFile(codeFile.c_str());

        if (!inFile.is_open() || !outFile.is_open() || !codesFile.is_open()) {
            cerr << "Error opening file(s)" << endl;
            return;
        }

        unordered_map<string, char> huffmanCodes;
        string line;
        // Read codes from file
        while (getline(codesFile, line)) {
            if (line.length() > 1) {
                huffmanCodes[line.substr(2)] = line[0];
            }
        }

        string encodedData((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        string currentCode;
        string decodedData;

        // Decode the encoded data
        for (size_t i = 0; i < encodedData.length(); ++i) {
            char bit = encodedData[i];
            currentCode += bit;
            if (huffmanCodes.find(currentCode) != huffmanCodes.end()) {
                decodedData += huffmanCodes[currentCode];
                currentCode = "";
            }
        }

        outFile << decodedData;

        inFile.close();
        outFile.close();
        codesFile.close();
    }

private:
    Node* root;
    unordered_map<char, string> huffmanCodes;

    void generateCodes(Node* root, string str, unordered_map<char, string>& huffmanCodes) {
        if (!root) return;

        if (!root->left && !root->right) {
            huffmanCodes[root->data] = str;
        }

        generateCodes(root->left, str + "0", huffmanCodes);
        generateCodes(root->right, str + "1", huffmanCodes);
    }
};

int main() {
    Huffman huff;
    huff.buildHuffmanTree("input.txt", "codes.txt");
    huff.compression("input.txt", "compressed.bin");
    // Uncomment the following line when you want to decompress
    // huff.decompression("compressed.bin", "output.txt", "codes.txt");
    return 0;
}
