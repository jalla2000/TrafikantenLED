#ifndef LEDFONT_HPP
#define LEDFONT_HPP

#include "fontletter.hpp"
#include <map>

class LedFont {
public:
    LedFont()
    {
        std::string valid = "abcdefghijklmnopqrstuvxyzæøåABCDEFGHIJKLMNOPQRSTUVXYZÆØÅ .!/0123456789";
        for (size_t i = 0; i < valid.size(); ++i) {
            std::string c = valid.substr(i, 1);
            if ((unsigned char)c[0] < 128) {
                chars_[c] = FontLetter(c);
            }
            else {
                // multibyte character (limited support)
                chars_[c] = FontLetter(std::string(valid.substr(i, 2)));
                ++i;
            }
        }
    }
    std::map<std::string, FontLetter> chars_;
};

#endif
