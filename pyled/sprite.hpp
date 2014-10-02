#ifndef SPRITE_HPP
#define SPRITE_HPP

class Sprite {
public:
    Sprite()
        : dataWidth_(0)
        , dataHeight_(0)
        , data_()
    { }
    void P(unsigned char data) { data_.push_back(data); }
    size_t width() { return dataWidth_*8; }
    size_t height() { return dataHeight_; };
    size_t dataWidth_;
    size_t dataHeight_;
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
