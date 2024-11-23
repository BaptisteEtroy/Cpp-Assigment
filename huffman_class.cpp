#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>

using namespace std;

// Base class Huffman
class Huffman {
public:
    // Node structure for Huffman tree
    struct Node {
        char data;
        unsigned freq;
        Node *left, *right;
        Node(char data, unsigned freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
        ~Node() {
            delete left;
            delete right;
        }
    };

    Huffman() : root(nullptr) {}

    virtual ~Huffman() {
        delete root;
    }

protected:
    Node* root;
    unordered_map<char, string> huffmanCodes;

    struct Compare {
        bool operator()(Node* left, Node* right) {
            return left->freq > right->freq;
        }
    };

    void generateCodes(Node* node, const string& str) {
        if (!node)
            return;

        if (!node->left && !node->right) {
            huffmanCodes[node->data] = str;
        }

        generateCodes(node->left, str + "0");
        generateCodes(node->right, str + "1");
    }
};

// Compression class derived from Huffman
class Compression : public Huffman {
public:
    bool compress(const string& inputFile, const string& outputFile) {
        ifstream inFile(inputFile, ios::binary | ios::ate);
        if (!inFile.is_open()) {
            cerr << "Error opening input file: " << inputFile << endl;
            return false;
        }

        // Get original file size
        streamsize originalSize = inFile.tellg();
        inFile.seekg(0, ios::beg); // Reset file pointer to beginning

        unordered_map<char, int> frequency;

        // Read the input file and calculate frequency
        char ch;
        while (inFile.get(ch)) {
            frequency[ch]++;
        }
        inFile.clear();
        inFile.seekg(0, ios::beg); // Reset file pointer to beginning

        // Create priority queue for Huffman tree
        priority_queue<Node*, vector<Node*>, Compare> minHeap;

        // Create leaf nodes
        for (auto it = frequency.begin(); it != frequency.end(); ++it) {
            Node* node = new Node(it->first, it->second);
            minHeap.push(node);
        }

        // Build Huffman Tree
        while (minHeap.size() > 1) {
            Node *left = minHeap.top(); minHeap.pop();
            Node *right = minHeap.top(); minHeap.pop();
            Node *newNode = new Node('\0', left->freq + right->freq);
            newNode->left = left;
            newNode->right = right;
            minHeap.push(newNode);
        }

        root = minHeap.top();

        // Generate codes
        generateCodes(root, "");

        // Open the output file to write codebook and compressed data
        ofstream outFile(outputFile, ios::binary);
        if (!outFile.is_open()) {
            cerr << "Error opening output file: " << outputFile << endl;
            return false;
        }

        // Write the codebook size placeholder (will update later)
        size_t codebookSizePosition = outFile.tellp();
        size_t codebookSize = 0;
        outFile.write(reinterpret_cast<char*>(&codebookSize), sizeof(codebookSize));

        // Write the codebook
        size_t codeCount = huffmanCodes.size();
        outFile.write(reinterpret_cast<char*>(&codeCount), sizeof(codeCount));

        for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
            // Write character
            outFile.put(it->first);

            // Write code length
            size_t codeLength = it->second.size();
            outFile.write(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));

            // Convert code string to bits and write
            string codeStr = it->second;
            size_t numFullBytes = codeLength / 8;
            size_t remainingBits = codeLength % 8;
            size_t totalBytes = numFullBytes + (remainingBits ? 1 : 0);

            vector<unsigned char> codeBytes(totalBytes, 0);

            for (size_t i = 0; i < codeLength; ++i) {
                if (codeStr[i] == '1') {
                    codeBytes[i / 8] |= (1 << (7 - (i % 8)));
                }
            }

            // Write the code bytes
            outFile.write(reinterpret_cast<char*>(&codeBytes[0]), totalBytes);
        }

        // Calculate and update the codebook size
        size_t currentPosition = outFile.tellp();
        codebookSize = currentPosition - sizeof(size_t); // Exclude the size placeholder itself
        outFile.seekp(codebookSizePosition, ios::beg);
        outFile.write(reinterpret_cast<char*>(&codebookSize), sizeof(codebookSize));
        outFile.seekp(currentPosition, ios::beg); // Return to the end to write compressed data

        // Compress the input file and write to output file
        unsigned char buffer = 0;
        int bitCount = 0;

        size_t totalBitsWritten = 0;

        while (inFile.get(ch)) {
            string code = huffmanCodes[ch];
            for (char bitChar : code) {
                buffer <<= 1;
                if (bitChar == '1') {
                    buffer |= 1;
                }
                bitCount++;
                if (bitCount == 8) {
                    outFile.put(buffer);
                    buffer = 0;
                    bitCount = 0;
                }
                totalBitsWritten++;
            }
        }

        // Handle remaining bits
        if (bitCount > 0) {
            buffer <<= (8 - bitCount);
            outFile.put(buffer);
            totalBitsWritten += (8 - bitCount);
        }

        inFile.close();
        outFile.close();

        // Get compressed file size
        ifstream compressedFile(outputFile, ios::binary | ios::ate);
        if (!compressedFile.is_open()) {
            cerr << "Error opening compressed file to get size: " << outputFile << endl;
            return false;
        }
        streamsize compressedSize = compressedFile.tellg();
        compressedFile.close();

        // Calculate and display compression ratio
        double compressionRatio = static_cast<double>(compressedSize) / static_cast<double>(originalSize);
        cout << "Original size: " << originalSize << " bytes" << endl;
        cout << "Compressed size: " << compressedSize << " bytes" << endl;
        cout << "Compression ratio: " << compressionRatio << endl;

        return true;
    }
};

// Decompression class derived from Huffman
class Decompression : public Huffman {
public:
    bool decompress(const string& inputFile, const string& outputFile) {
        // Open the input file to read codebook and compressed data
        ifstream inFile(inputFile, ios::binary);
        if (!inFile.is_open()) {
            cerr << "Error opening input file: " << inputFile << endl;
            return false;
        }

        // Read the codebook size
        size_t codebookSize;
        inFile.read(reinterpret_cast<char*>(&codebookSize), sizeof(codebookSize));
        size_t codebookEndPosition = sizeof(size_t) + codebookSize;

        // Read the number of codes
        size_t codeCount;
        inFile.read(reinterpret_cast<char*>(&codeCount), sizeof(codeCount));

        unordered_map<string, char> reverseHuffmanCodes;

        // Read each character and its code
        for (size_t i = 0; i < codeCount; ++i) {
            char ch = inFile.get();

            size_t codeLength;
            inFile.read(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));

            size_t numFullBytes = codeLength / 8;
            size_t remainingBits = codeLength % 8;
            size_t totalBytes = numFullBytes + (remainingBits ? 1 : 0);

            vector<unsigned char> codeBytes(totalBytes);
            inFile.read(reinterpret_cast<char*>(&codeBytes[0]), totalBytes);

            // Convert code bytes to string
            string codeStr = "";
            for (size_t j = 0; j < codeLength; ++j) {
                unsigned char byte = codeBytes[j / 8];
                bool bit = (byte >> (7 - (j % 8))) & 1;
                codeStr += bit ? '1' : '0';
            }

            reverseHuffmanCodes[codeStr] = ch;
        }

        // Open the output file
        ofstream outFile(outputFile, ios::binary);
        if (!outFile.is_open()) {
            cerr << "Error opening output file: " << outputFile << endl;
            return false;
        }

        // Decompress the data
        string currentCode = "";
        unsigned char byte;

        // Read from the current position after the codebook
        while (inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            for (int i = 7; i >= 0; --i) {
                bool bit = (byte >> i) & 1;
                currentCode += bit ? '1' : '0';
                auto it = reverseHuffmanCodes.find(currentCode);
                if (it != reverseHuffmanCodes.end()) {
                    outFile.put(it->second);
                    currentCode = "";
                }
            }
        }

        inFile.close();
        outFile.close();

        cout << "Decompression completed successfully." << endl;
        return true;
    }
};

int main() {
    string inputFile, outputFile;
    string choice;

    cout << "Do you want to (c)ompress or (d)ecompress? ";
    cin >> choice;
    cin.ignore(); // Ignore remaining newline

    if (choice == "c") {
        cout << "Enter the path to the input file to compress: ";
        getline(cin, inputFile);

        cout << "Enter the path for the compressed output file: ";
        getline(cin, outputFile);

        Compression compressor;
        bool success = compressor.compress(inputFile, outputFile);

        if (success) {
            cout << "Compression completed successfully." << endl;
        } else {
            cout << "Compression failed." << endl;
        }
    } else if (choice == "d") {
        cout << "Enter the path to the compressed input file: ";
        getline(cin, inputFile);

        cout << "Enter the path for the decompressed output file: ";
        getline(cin, outputFile);

        Decompression decompressor;
        bool success = decompressor.decompress(inputFile, outputFile);

        if (success) {
            cout << "Decompression completed successfully." << endl;
        } else {
            cout << "Decompression failed." << endl;
        }
    } else {
        cout << "Invalid choice." << endl;
    }

    return 0;
}
