#include "HuffmanTree.h"

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
#include <locale>
#include <cwctype>
#include <iomanip>

//constexpr std::uint32_t SCALE {100'000};
std::uint64_t NR_LITERE {};
std::map<wchar_t,int> FREQ;
std::uint64_t TOTAL_CHAR_COUNT {};
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
        std::left << std::setw(10) <<
        std::fixed << std::setprecision(10)
        << elem.second/static_cast<double>(NR_LITERE) << '\n';
    }

    //std::cout << "Ran with success!";

    return 0;
}