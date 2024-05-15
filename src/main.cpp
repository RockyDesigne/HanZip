#include "Fract.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <exception>
#include <unordered_map>
#include <map>
#include <sstream>
#include <memory>
#include <vector>
#include <codecvt>
#include <iomanip>
#include <cmath>
#include <locale>
#include <codecvt>
#include <bitset>
#define RELEASE

std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

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
    } else if (argc < 3) {
        throw Error {"No command provided! Input decompress or compress"};
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
            ++FREQ[towlower(elem)];
        } else if (iswspace(elem) || elem == L'\n') {
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

void generateHuffmanCodes(Node* node, const std::string& code, std::map<wchar_t, std::string>& huffmanCodes, std::map<std::string, wchar_t>& huffmanCodesPrime) {
    if (node == nullptr) {
        return;
    }

    // If this is a leaf node (it has a character), then we've found a Huffman code for the character
    if (node->left == nullptr && node->right == nullptr) {
        huffmanCodes[node->chr] = code;
        huffmanCodesPrime[code] = node->chr;
    }

    // Traverse the left subtree with a '0' added to the code
    generateHuffmanCodes(node->left, code + '0', huffmanCodes, huffmanCodesPrime);

    // Traverse the right subtree with a '1' added to the code
    generateHuffmanCodes(node->right, code + '1', huffmanCodes, huffmanCodesPrime);
}

std::string compress(const std::wstring& input, const std::map<wchar_t, std::string>& huffmanCodes) {
    std::string output;
    for (wchar_t character : input) {
        if (huffmanCodes.count(tolower(character)) > 0) {
            output += huffmanCodes.at(tolower(character));
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

std::wstring decompress(const std::string& input, const std::map<std::string, wchar_t>& huffmanCodesPrime) {

    std::string tmp;
    std::wstring output;
    for (auto ch : input) {
        tmp += ch;
        if (huffmanCodesPrime.contains(tmp)) {
            output += huffmanCodesPrime.at(tmp);
            tmp.clear();
        }
    }
    return output;
}

void writeBytes(const std::string& filename, const std::vector<uint8_t>& bytes) {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
//01000000 01000111
std::vector<uint8_t> toBytes(const std::string& input) {
    std::vector<uint8_t> output;
    for (size_t i = 0; i < input.size(); i += 8) {
        uint8_t byte = 0;
        //00000001
        for (size_t j = 0; j < 8 && i + j < input.size(); ++j) {
            byte = (byte << 1) | (input[i + j] == L'1' ? 1 : 0);
        }
        output.push_back(byte);
    }
    return output;
}

float meanCodeLength(const std::map<wchar_t, std::string>& huffmanCodes) {
    float codesLength = 0;
    for (const auto& elem : huffmanCodes) {
        float pA = (float) FREQ[elem.first] / (float) NR_LITERE;
        auto lA = elem.second.size();
        codesLength += pA * lA;
    }

    return codesLength;

}

//-S[ps*log(ps)]
float entropy() {
    float rez = 0.f;

    for (const auto& elem : FREQ) {
        float pS = (float) FREQ[elem.first] / (float) NR_LITERE;
        rez += pS * std::log2(pS);
    }
    return -rez;
}

void expand(std::map<std::string, wchar_t>& huffmanCodesPrime) {

    std::wifstream f {"codes.txt"};

    if (!f.is_open()) {
        throw std::runtime_error("Could not open code.txt file");
    }

    std::wstringstream ss;

    ss << f.rdbuf();
    f.close();
    //std::wcout << ss.str();

    auto contents = ss.str();

    std::vector<wchar_t> v1;

    int i = 0;
    while (iswalpha(contents[i]) || contents[i] == L'\n' || contents[i] == L' ' || contents[i] == L'\t') {
        v1.push_back(contents[i]);
        ++i;
    }

    std::vector<std::string> v2;

    while (i < contents.size()) {
        std::string tmp;
        while (contents[i] != '.') {
            tmp += contents[i];
            ++i;
        }
        ++i;
        v2.emplace_back(tmp);
    }

    for (int l = 0; l < v1.size(); ++l) {
        huffmanCodesPrime[v2[l]] = v1[l];
    }

}

int main(int argc, char** argv) {
#ifndef DEBUG
    try {
        check_arg_count(argc);
    } catch (Error& e) {
        std::cout << e.what();
        return 0;
    }
#endif
    //std::cout << "Path: " << argv[1];

    //std::setlocale(LC_ALL, "en_US.UTF-8");
    std::locale::global(std::locale({}, new std::codecvt_utf8<wchar_t>));
    std::setlocale(LC_ALL, "ro_RO.utf8");
    FPTR fin;
#ifndef DEBUG
    std::string cmd = argv[2];
    std::string filePath = argv[1];
    std::string fileName {filePath.begin(), filePath.end() - 4};
#else
    std::string cmd = "compress";
    std::string filePath = "eminescu.txt";
    std::string fileName {"eminescu"};
#endif
    if (cmd == "decompress") {

        std::map<std::string, wchar_t> huffmanCodesPrime;
        expand(huffmanCodesPrime);
        std::ifstream f{filePath, std::ios::binary};
        if (!f.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
        std::stringstream ss;
        ss << f.rdbuf();
        f.close();
        auto bytes = ss.str();

        std::string s;
        uint8_t mask = 0x01;
        for (auto elem: bytes) {
            for (int i = 7; i >= 0; --i) {
                if ((elem & (mask << i))) {
                    s += '1';
                } else {
                    s += '0';
                }
            }
        }

        auto rez = decompress(s, huffmanCodesPrime);
        std::wofstream fout{fileName + "Decomp.txt"};
        fout << rez;
        fout.close();
    } else if (cmd == "compress") {
        try {
            fin = open_file(filePath.c_str());
        } catch (Error &e) {
            std::cout << e.what();
            return 0;
        }

        std::wstringstream ss;

        ss << fin->rdbuf();

        fin->close();

        std::wstring buffer{ss.str()};

        countLetters(buffer);

        std::priority_queue<Node *, std::vector<Node *>, Compare> queue;

        for (auto &elem: FREQ) {
            Node *node = new Node{elem.first,
                                  {elem.second, (int) NR_LITERE},
                                  nullptr,
                                  nullptr};
            queue.push(node);
        }

        while (queue.size() > 1) {
            // Dequeue the two nodes with the lowest frequency
            Node *left = queue.top();
            queue.pop();
            Node *right = queue.top();
            queue.pop();

            Node *newNode = new Node{'\0', left->freq + right->freq, left, right};

            queue.push(newNode);
        }

        // The remaining node is the root of the Huffman tree
        Node *root = queue.top();
        queue.pop();

        std::string codes;
        std::map<wchar_t, std::string> huffmanCodes;
        std::map<std::string, wchar_t> huffmanCodesPrime;
        generateHuffmanCodes(root, codes, huffmanCodes, huffmanCodesPrime);

        std::wofstream fout{"statistics.txt"};

        fout << "There are " << NR_LITERE << " letters in the file.\n";

        for (auto elem: FREQ) {
            fout << elem.first << std::left << std::setw(1) << ": " <<
                 std::left << std::setw(10) <<
                 elem.second << "P: " <<
                 std::left << std::setw(1) <<
                 elem.second << "/" << NR_LITERE <<
                 std::left << std::setw(1);

            std::wstring tmp{huffmanCodes.at(elem.first).begin(), huffmanCodes.at(elem.first).end()};
            fout << "          cod: " << tmp << '\n';
        }

        auto entrop = entropy();
        auto meanLength = meanCodeLength(huffmanCodes);

        fout << "Mean code length: " << meanLength << " (binary digits)";
        fout << '\n';
        fout << "Entropy of input text:  " << entrop << " bits" << '\n';

        fout << "Code efficiency: " << entrop / meanLength << " %" << '\n';

        fout << "Redundancy: " << 1 - entrop / meanLength << " %" << '\n';

        fout << entrop << " < " << meanLength << " < " << entrop + 1 << " (Shannon's source coding theorem)\n";

        fout.close();

        auto compFile = compress(buffer, huffmanCodes);

        std::wstring s{};


        for (const auto &elem: huffmanCodes) {
            s.push_back(elem.first);
        }

        for (const auto &elem: huffmanCodes) {
            std::wstring t{elem.second.begin(), elem.second.end()};
            s += t;
            s.push_back('.');
        }

        auto compFileBytes = toBytes(compFile);

        std::string bytes{converter.to_bytes(s)};

        std::ofstream file{fileName + ".HanZip", std::ios::binary};
        std::ofstream huffmanFile{"codes.txt"};

        huffmanFile.write(bytes.c_str(), bytes.size());
        file.write(reinterpret_cast<const char *>(compFileBytes.data()), compFileBytes.size());
        file.close();
        delTree(root);

        std::filesystem::path p1 {filePath};
        std::filesystem::path p2 {fileName + ".HanZip"};
        auto p1Size = std::filesystem::file_size(p1);
        auto p2Size = std::filesystem::file_size(p2);
        auto compressionRate = 1 - (p2Size/(float)p1Size);

        std::ofstream stats {"statistics.txt", std::ios::app};
        stats << "Compression Rate: " << compressionRate << " % (data rate saving)";
        stats.close();
    }

    std::cout << "Ran with success!";

    return 0;
}