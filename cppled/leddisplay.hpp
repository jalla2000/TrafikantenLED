#ifndef LEDDISPLAY_HPP
#define LEDDISPLAY_HPP

#include <vector>
#include <string>

class LedFont;
class Sprite;

class LedDisplay {
public:
    enum Color {
        RED,
        ORANGE,
        GREEN
    };
    static const int PIXELS_PER_TEXTLINE = 8;
    static const int BYTES_PER_LINE = 32;
    static const int BITS_PER_PIXEL = 2;
    static const int DISPLAY_HEIGHT = 32;
    static const int DISPLAY_WIDTH = 128;

    LedDisplay(const std::string & device,
               size_t lines,
               LedFont * font);
    ~LedDisplay();
    bool open(std::string & error);
    void setByte(size_t row, size_t col, unsigned char data);
    void setPixel(size_t xpos, size_t ypos, Color color);
    void flush(int line);
    size_t widthOfTxt(const std::string & text);
    bool writeCharacter(const std::string & character, Color color);
    void writeTxt(const std::string & text, Color color);
    void drawSprite(const Sprite & sprite, Color color);
    void send();

    std::vector<unsigned char> gfxBuffer_;
    int currentX_;
    int currentY_;

private:
    const std::string devicePath_;
    int deviceFileHandle_;
    int lines_;
    LedFont * font_;
};

#endif
