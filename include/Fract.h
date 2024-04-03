//
// Created by HORIA on 03.04.2024.
//

#ifndef HANZIP_FRACT_H
#define HANZIP_FRACT_H

#include <numeric>

struct Fract {
    Fract(int num, int denum) {
        auto gcd = std::gcd(num, denum);
        m_num = num / gcd;
        m_denum = denum / gcd;
    }
    int m_num;
    int m_denum;

    bool operator < (const Fract& fr) const {
        if (m_denum > fr.m_denum) {
            return true;
        } else if (m_denum == fr.m_denum) {
            return m_num < fr.m_num;
        } else {
            return false;
        }
    }

    Fract operator + (const Fract& fr) const {
        int num = m_num * fr.m_denum + m_denum * fr.m_num;
        int denum = m_denum * fr.m_denum;

        return Fract {num, denum};
    }

};

#endif //HANZIP_FRACT_H
