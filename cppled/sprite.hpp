#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <cstddef>
#include <vector>
#include <cassert>

class Sprite {
public:
    Sprite()
        : dataWidth_(0)
        , dataHeight_(0)
        , spriteWidth_(7)
        , data_()
    { }
    void recalculateSpriteWidth()
    {
        size_t widest = 0;
        for (size_t row = 0; row < dataHeight_; ++row) {
            for (size_t col = 0; col < dataWidth_; ++col) {
                size_t width = 0;
                unsigned char data = data_[col+row*dataWidth_];
                if (data & 0b11000000)
                    width = 1;
                if (data & 0b00110000)
                    width = 2;
                if (data & 0b00001100)
                    width = 3;
                if (data & 0b00000011)
                    width = 4;
                if (width)
                    width = width+(4*col);
                if (width > widest) {
                    widest = width;
                }
            }
        }
        assert(widest > 0);
        spriteWidth_ = widest;
    }
    void P(unsigned char data) { data_.push_back(data); }
    size_t width() { return dataWidth_*8; }
    size_t height() { return dataHeight_; };
    size_t dataWidth_;
    size_t dataHeight_;
    size_t spriteWidth_;
    std::vector<unsigned char> data_;
};

class Funky : public Sprite {
public:
    Funky()
    {
        P(0b00111111);P(0b11111111);P(0b11111111);P(0b11111100);
        P(0b11110000);P(0b00000000);P(0b00000000);P(0b00000011);
        P(0b11001100);P(0b00000000);P(0b00000000);P(0b00110011);
        P(0b11000011);P(0b00000000);P(0b00000000);P(0b00000011);
        P(0b11000000);P(0b11000000);P(0b00000000);P(0b00000011);
        P(0b11000000);P(0b00110000);P(0b00000000);P(0b00000011);
        P(0b11000000);P(0b00001100);P(0b00000000);P(0b00000011);
        P(0b11000000);P(0b00000011);P(0b00000000);P(0b00000011);
        P(0b11000000);P(0b00000000);P(0b11000000);P(0b00000011);
        P(0b11000000);P(0b00000000);P(0b00110000);P(0b00000011);
        P(0b11000000);P(0b00000000);P(0b00001100);P(0b00000011);
        P(0b11000000);P(0b00000000);P(0b00000011);P(0b00000011);
        P(0b11000000);P(0b00000000);P(0b00000000);P(0b11000011);
        P(0b11001100);P(0b00000000);P(0b00000000);P(0b00110011);
        P(0b11000000);P(0b00000000);P(0b00000000);P(0b00001111);
        P(0b00111111);P(0b11111111);P(0b11111111);P(0b11111100);
        dataWidth_ = 4;
        dataHeight_ = 16;
    }
};

#endif
