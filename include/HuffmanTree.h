//
// Created by HORIA on 07.03.2024.
//

#ifndef HANZIP_HUFFMANTREE_H
#define HANZIP_HUFFMANTREE_H

#include <string>
#include <vector>
#include <queue>
#include <iostream>

class HuffmanTree {
public:
    struct Node {
        char c {};
        std::uint64_t frecv {};
        Node* left {}, *right {};
        bool operator>(const Node& rhs) const {
            return (frecv > rhs.frecv);
        }
        bool operator<(const Node& rhs) const {
            return (frecv < rhs.frecv);
        }
    };
    HuffmanTree(const std::vector<Node*>& v) {
        m_root = huffman(v);
    }

    ~HuffmanTree() {
        delTree(m_root);
    }

    void print_encodings(std::string& encoding) {
        printNumEncodings(m_root,encoding);
    }

    std::string encode_tree() {
        std::string tmp;
        std::string code;
        encode(m_root,tmp,code);
        return code;
    }

    std::string decode_tree(const std::string& code) {
        return decode(m_root,code);
    }

private:
    Node* m_root {};
    static Node* huffman(const std::vector<Node*>& v) {
        auto comp {
            [](const Node* a, const Node* b){return a->frecv > b->frecv;}
        };
        std::priority_queue<Node*, std::vector<Node*> , decltype(comp)> q(comp);
        for (auto elem : v)
            q.push(elem);
        for (int i=0;i<v.size()-1;++i) {
            auto z {new Node};
            auto x {q.top()};
            z->left = x;
            q.pop();
            auto y {q.top()};
            z->right = y;
            q.pop();
            z->frecv = y->frecv + x->frecv;
            q.push(z);
        }
        return q.top();
    }
    void printNumEncodings(Node* root, std::string& encoding) {
        if (!root) {
            return;
        }
        if (!root->left && !root->right) {
            std::cout << encoding;
            std::cout << " : ";
            std::cout << root->c << '\n';
        }
        if (root->left) {
            encoding += '0';
            printNumEncodings(root->left, encoding);
        }
        if (root->right) {
            encoding += '1';
            printNumEncodings(root->right,encoding);
        }
        if (!encoding.empty())
            encoding.pop_back();
    }
    void encode(Node* root, std::string& encoding, std::string& rez) {
        if (!root) {
            return;
        }
        if (!root->left && !root->right) {
            rez += encoding;
        }
        if (root->left) {
            encoding += '0';
            encode(root->left, encoding,rez);
        }
        if (root->right) {
            encoding += '1';
            encode(root->right,encoding,rez);
        }
        if (!encoding.empty())
            encoding.pop_back();
    }
    static std::string decode(Node* root, const std::string& code) {
        int i {};
        std::string decoded;
        while (i < code.size()) {
            Node* it {root};
            while (it) {
                if (!it->left && !it->right) {
                    decoded += it->c;
                    //decoded += " ";
                    break;
                }
                if (code[i] == '0') {
                    it = it->left;
                } else {
                    it = it->right;
                }
                ++i;
            }
        }
        return decoded;
    }
    void delTree(Node* root) {
        if (!root) {
            return;
        }
        delTree(root->left);
        delTree(root->right);
        delete root;
    }
};

/*
 * Example program
 * int main() {
    std::vector<std::pair<char,int>> pairs {
            {'A',28},
            {'B', 9},
            {'C',21},
            {'D',10},
            {'E',32}
    };

    std::vector<HuffmanTree::Node*> v;
    v.reserve(pairs.size());
    for (auto& elem : pairs) {
        v.push_back(new HuffmanTree::Node {elem.first,elem.second});
    }

    HuffmanTree t1 {v};

    std::string code = t1.encode_tree();

    std::cout << "Code: " << code << '\n';

    std::cout << "Decoded: " << t1.decode_tree(code) << "\n";

    return 0;
}
 */

#endif //HANZIP_HUFFMANTREE_H
