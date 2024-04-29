
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
#include <cmath>
#include <bitset>

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

FPTR open_file(const std::string& filePath) {
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
            ++FREQ[towlower(elem)];
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

void generateHuffmanCodes(Node* node, const std::wstring& code, std::map<wchar_t, std::wstring>& huffmanCodes, std::map<std::wstring, wchar_t>& huffmanCodesPrime) {
    if (node == nullptr) {
        return;
    }

    // If this is a leaf node (it has a character), then we've found a Huffman code for the character
    if (node->left == nullptr && node->right == nullptr) {
        huffmanCodes[node->chr] = code;
        huffmanCodesPrime[code] = node->chr;
    }

    // Traverse the left subtree with a '0' added to the code
    generateHuffmanCodes(node->left, code + L'0', huffmanCodes, huffmanCodesPrime);

    // Traverse the right subtree with a '1' added to the code
    generateHuffmanCodes(node->right, code + L'1', huffmanCodes, huffmanCodesPrime);
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

auto findValue(const std::wstring& value,  const std::map<wchar_t, std::wstring>& huffmanCodes) {

    for (auto it = huffmanCodes.begin(); it != huffmanCodes.end(); ++it) {
        if (value == it->second) {
            return it;
        }
    }
    return huffmanCodes.end();
}

std::wstring decompress(const std::wstring& input, const std::map<std::wstring, wchar_t>& huffmanCodesPrime) {

    std::wstring tmp, output;
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
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
//01000000 01000111
std::vector<uint8_t> toBytes(const std::wstring& input) {
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

float lungimeMedieCod(const std::map<wchar_t, std::wstring>& huffmanCodes) {
    float codesLength = 0;
    for (const auto& elem : huffmanCodes) {
        float pA = (float) FREQ[elem.first] / (float) NR_LITERE;
        auto lA = elem.second.size();
        codesLength += pA * lA;
    }

    return codesLength;

}

float entropy() {
    float rez = 0.f;

    for (const auto& elem : FREQ) {
        float pS = (float) FREQ[elem.first] / (float) NR_LITERE;
        rez += pS * std::log(pS);
    }
    return -rez;
}

void parse(const std::string& fileName, std::map<std::wstring, wchar_t>& huffmanCodesPrime) {

    std::wifstream fin {fileName};

    int nrChars;
    fin >> nrChars;

    std::vector<wchar_t> chars(nrChars);

    for (int i = 0; i<nrChars; ++i) {
        fin >> chars[i];
    }

    std::vector<std::wstring> codes;

    for (int i = 0; i < nrChars; ++i) {
        std::wstring tmp;
        fin >> tmp;
        codes.emplace_back(tmp);
    }

    for (int i = 0; i < nrChars; ++i) {
        auto ch = chars[i];
        auto code = codes[i];
        if (ch == L'1') {
            huffmanCodesPrime[code] = L'\n';
        } else if (ch == L'2') {
            huffmanCodesPrime[code] = L' ';
        } else
            huffmanCodesPrime[code] = ch;
    }

    //discards newline and space
    wchar_t ch;
    fin >> ch;
    fin >> ch;

    std::vector<uint16_t> compressedData;

    wchar_t data;

    std::cout << "Before read: " << "eof=" << fin.eof() << ", fail=" << fin.fail() << ", bad=" << fin.bad() << ", good=" << fin.good() << std::endl;
    fin.read(&data, sizeof(data));
    std::cout << "After read: " << "eof=" << fin.eof() << ", fail=" << fin.fail() << ", bad=" << fin.bad() << ", good=" << fin.good() << std::endl;

    while (fin.read(&data,sizeof (data))) {
        compressedData.push_back(static_cast<uint16_t>(data));
    }

    std::string bitsString;

    for (auto wch : compressedData) {
        std::bitset<sizeof (data) * 8> bits(wch);
        bitsString += bits.to_string();
    }



}

void huffmanEncode(const std::string& fileName, const FPTR& fin) {
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

    std::wstring codes;
    std::map<wchar_t, std::wstring> huffmanCodes;
    std::map<std::wstring, wchar_t> huffmanCodesPrime;
    generateHuffmanCodes(root, codes, huffmanCodes, huffmanCodesPrime);

    std::wofstream fout{"statistica.txt"};

    fout << "Sunt " << NR_LITERE << " de litere in fisier.\n";

    for (auto elem: FREQ) {
        fout << elem.first << std::left << std::setw(1) << ": " <<
             std::left << std::setw(10) <<
             elem.second << "P: " <<
             std::left << std::setw(1) <<
             elem.second << "/" << NR_LITERE <<
             std::left << std::setw(1) <<
             "          cod: " <<
             huffmanCodes.at(elem.first) <<
             '\n';
    }

    auto entrop = entropy();
    auto lungMedie = lungimeMedieCod(huffmanCodes);

    fout << "Lungimea medie a codului: " << lungMedie;
    fout << '\n';
    fout << "Entropia textului de intrare:  " << entrop << '\n';

    fout << "Eficienta codului: " << entrop / lungMedie << '\n';

    fout << "Redundanta: " << 1 - entrop / lungMedie << '\n';

    fout.close();

    auto compFile = compress(buffer, huffmanCodes);

    std::string outFile = fileName + ".HanZip";

    {
        std::wofstream wfstream {outFile};
        wfstream << huffmanCodes.size() << '\n';
        for (const auto& elem : huffmanCodes) {
            if (elem.first == L'\n') {
                wfstream << "1" << " ";
            } else if (elem.first == L' ') {
                wfstream << "2" << " ";
            } else {
                wfstream << elem.first << " ";
            }
        }
        wfstream << '\n';
        for (const auto& elem : huffmanCodes) {
            wfstream << elem.second << " ";
        }
        wfstream << '\n';
        wfstream.close();
    }

    writeBytes("Tara_Mea.HanZip", toBytes(compFile));

    //std::wifstream compIN {"compr.txt"};
    //std::wstring comprString;
    //compIN >> comprString;
    //compIN.close();

    //auto decompString = decompress(compFile, huffmanCodesPrime);

    /*
    std::wofstream decompOUT {"decompr.txt"};
    decompOUT << decompString;
    decompOUT.close();
    */
    delTree(root);
}

int main(int argc, char** argv) {
/*
    try {
        check_arg_count(argc);
    } catch (Error &e) {
        std::cout << e.what();
        return 0;
    }
*/
    //std::cout << "Path: " << argv[1];

    //std::setlocale(LC_ALL, "en_US.UTF-8");
    std::locale::global(std::locale({}, new std::codecvt_utf8<wchar_t>));
    std::setlocale(LC_ALL, "ro_RO.utf8");
    FPTR fin;

    std::string cmd = "decompress";
    std::string fileName = "Tara_Mea.HanZip";


    try {
        fin = open_file(fileName);
    } catch (Error &e) {
        std::cout << e.what();
        return 0;
    }


    if (cmd == "compress") {
        huffmanEncode(fileName,fin);
    } else if (cmd == "decompress") {
        std::map<std::wstring, wchar_t> huffmanCodesPrime;
        parse(fileName, huffmanCodesPrime);

    }



    std::cout << "Ran with success!";

    return 0;
}