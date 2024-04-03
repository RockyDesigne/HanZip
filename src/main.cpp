
#include "HuffmanTree.h"
#include "Fract.h"
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <unordered_map>
#include <map>
#include <sstream>
#include <memory>
#include <vector>
#include <codecvt>
#include <iomanip>

//constexpr std::uint32_t SCALE {100'000};
std::uint64_t NR_LITERE {};
std::map<wchar_t,int> FREQ;
//std::uint64_t TOTAL_CHAR_COUNT {};
using FPTR = std::shared_ptr<std::wifstream>;

class Error : public std::exception {
public:
    explicit Error(std::string_view msg) : m_msg {msg} {}
    [[nodiscard]] const char* what() const noexcept override {return m_msg.data();}
private:
    std::string m_msg;
};

void check_arg_count(int argc) {
    if (argc < 2) {
        throw Error {"No file path provided!"};
    }
}

FPTR open_file(const char* filePath) {
    auto fptr {std::make_shared<std::wifstream>(filePath)};
    if (fptr->fail()) {
        throw Error {"Could not open file!"};
    }
    return fptr;
}

void countLetters(std::wstring_view contents) {
    for (auto elem : contents) {
        if (iswalpha(elem)) {
            ++NR_LITERE;
            ++FREQ[elem];
        }
    }
}

struct Node {
    wchar_t chr;
    Fract freq;
    Node* left;
    Node* right;
};

class Compare {
public:
    bool operator()(Node* left, Node* right) {
        return right->freq < left->freq;
    }
};

void generateHuffmanCodes(Node* node, const std::wstring& code, std::map<wchar_t, std::wstring>& huffmanCodes) {
    if (node == nullptr) {
        return;
    }

    // If this is a leaf node (it has a character), then we've found a Huffman code for the character
    if (node->left == nullptr && node->right == nullptr) {
        huffmanCodes[node->chr] = code;
    }

    // Traverse the left subtree with a '0' added to the code
    generateHuffmanCodes(node->left, code + L'0', huffmanCodes);

    // Traverse the right subtree with a '1' added to the code
    generateHuffmanCodes(node->right, code + L'1', huffmanCodes);
}

std::wstring compress(const std::wstring& input, const std::map<wchar_t, std::wstring>& huffmanCodes) {
    std::wstring output;
    for (wchar_t character : input) {
        if (huffmanCodes.count(character) > 0) {
            output += huffmanCodes.at(character);
        }
    }
    return output;
}

void delTree(Node* root) {
    if (!root)
        return;
    delTree(root->left);
    delTree(root->right);
    delete root;
}

std::wstring decompress(const std::wstring& input, Node* root) {
    std::wstring output;
    Node* node = root;
    for (wchar_t bit : input) {
        if (bit == L'0') {
            node = node->left;
        } else if (bit == L'1') {
            node = node->right;
        }
        if (node->left == nullptr && node->right == nullptr) {
            output += node->chr;
            node = root;
        }
    }
    return output;
}

int main(int argc, char** argv) {
/*
    try {
        check_arg_count(argc);
    } catch (Error& e) {
        std::cout << e.what();
        return 0;
    }
*/
    //std::cout << "Path: " << argv[1];

    //std::setlocale(LC_ALL, "en_US.UTF-8");
    std::locale::global(std::locale({}, new std::codecvt_utf8<wchar_t>));
    std::setlocale(LC_ALL, "ro_RO.utf8");
    FPTR fin;

    try {
        fin = open_file("./Tara_Mea");
    } catch (Error& e) {
        std::cout << e.what();
        return 0;
    }

    std::wstringstream ss;

    ss << fin->rdbuf();

    fin->close();

    std::wstring buffer {ss.str()};

    countLetters(buffer);

    std::wofstream fout {"output.txt"};

    fout << "Sunt " << NR_LITERE << " de litere in fisier.\n";

    for (auto elem : FREQ) {
        fout << elem.first << std::left << std::setw(1) << ": " <<
        std::left << std::setw(10) <<
        elem.second << "P: " <<
        std::left << std::setw(1) <<
        elem.second << "/" << NR_LITERE << '\n';
    }

    std::cout << "Ran with success!";

    std::priority_queue<Node*, std::vector<Node*>, Compare> queue;

    for (auto& elem : FREQ) {
        Node* node = new Node {elem.first,
                                   {elem.second, (int) NR_LITERE},
                                   nullptr,
                                   nullptr};
        queue.push(node);
    }

    while (queue.size() > 1) {
        // Dequeue the two nodes with the lowest frequency
        Node* left = queue.top();
        queue.pop();
        Node* right = queue.top();
        queue.pop();

        // Create a new node
        Node* newNode = new Node {'\0', left->freq + right->freq, left, right};

        // Enqueue the new node
        queue.push(newNode);
    }

    // The remaining node is the root of the Huffman tree
    Node* root = queue.top();
    queue.pop();

    std::wstring codes;
    std::map<wchar_t, std::wstring> huffmanCodes;

    generateHuffmanCodes(root, codes, huffmanCodes);

    auto compFile = compress(buffer, huffmanCodes);

    std::wofstream compOUT {"compressed.txt"};

    compOUT << compFile;

    compOUT.close();

    auto decomp = decompress(compFile, root);

    std::wofstream decompOUT {"decomp.txt"};

    decompOUT << decomp;

    delTree(root);

    return 0;
}