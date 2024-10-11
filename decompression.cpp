#include <iostream>
#include <fstream>
#include <unordered_map>
#include <bitset>
#include <string>

// Function to load the Huffman code mapping from a file
std::unordered_map<std::string, char> loadHuffmanCodes(const std::string& filename) {
    std::unordered_map<std::string, char> huffmanCode;
    std::ifstream inFile(filename);
    std::string line;

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;

        char ch = line[0];
        std::string code = line.substr(2);  // Skip the character and space between them
        huffmanCode[code] = ch;
    }

    inFile.close();
    return huffmanCode;
}

// Function to decode the compressed file using the Huffman codes
void decodeFile(const std::string& inFilename, const std::unordered_map<std::string, char>& huffmanCode, const std::string& outFilename) {
    std::ifstream inFile(inFilename, std::ios::binary);
    std::ofstream outFile(outFilename);

    char extraBitsChar;
    inFile.get(extraBitsChar);  // Read the extra bits character to handle padding
    int extraBits = static_cast<int>(extraBitsChar);

    char byte;
    std::string encodedStr = "";

    // Read the entire bitstream from the compressed file
    while (inFile.get(byte)) {
        std::string byteStr = std::bitset<8>(byte).to_string();  // Convert byte to 8-bit binary string
        encodedStr += byteStr;
    }

    // Remove the extra padding bits at the end
    if (extraBits != 8) {
        encodedStr.erase(encodedStr.size() - extraBits);
    }

    std::string currentCode = "";
    std::string decodedString = "";

    // Decode the bitstream using the Huffman codes
    for (char bit : encodedStr) {
        currentCode += bit;

        // Check if the current code matches any Huffman code
        if (huffmanCode.find(currentCode) != huffmanCode.end()) {
            char decodedChar = huffmanCode.at(currentCode);
            decodedString += decodedChar;
            currentCode = "";
        }
    }

    // Now replace "/n" with an actual newline character
    size_t pos = 0;
    while ((pos = decodedString.find("/n", pos)) != std::string::npos) {
        decodedString.replace(pos, 2, "\n");
    }

    // Write the final decoded string to the output file
    outFile << decodedString;

    inFile.close();
    outFile.close();
}

int main() {
    std::string compressedFilename = "compressed.bin";
    std::string codeMapFilename = "huffman_codes.txt";
    std::string outputFilename = "decompressed.txt";

    std::unordered_map<std::string, char> huffmanCode = loadHuffmanCodes(codeMapFilename);

    decodeFile(compressedFilename, huffmanCode, outputFilename);

    std::cout << "File decompressed successfully!" << std::endl;

    return 0;
}
