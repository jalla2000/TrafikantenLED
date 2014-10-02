#ifndef LEDFONT_HPP
#define LEDFONT_HPP

#include "fontletter.hpp"

class LedFont {
public:
    LedFont()
    {
        std::string valid = "abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVXYZ .!/0123456789";
        for (size_t i = 0; i < valid.size(); ++i) {
            std::string c = valid.substr(i, 1);
            chars_[c] = FontLetter(c);
        }
    }
    std::map<std::string, FontLetter> chars_;
};

#endif
