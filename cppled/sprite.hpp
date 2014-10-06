#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <cstddef>
#include <vector>

class Sprite {
public:
Sprite();
void recalculateSpriteWidth();
void P(unsigned char data);
size_t width();
size_t height();
size_t dataWidth_;
size_t dataHeight_;
size_t spriteWidth_;
std::vector<unsigned char> data_;
};

class Funky : public Sprite {
public:
Funky();
};

#endif
