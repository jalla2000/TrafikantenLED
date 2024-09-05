#pragma once

#include <vector>
#include <string>
#include <optional>

class LedFont;
class Sprite;

class LedDisplay {
public:
    enum Color {
        RED,
        ORANGE,
        GREEN
    };
    static constexpr auto DEVICE_NCURSES = "NCURSES";
    static constexpr auto DEVICE_TEMINAL = "TERMINAL";
    static const size_t PIXELS_PER_TEXTLINE = 8;
    static const size_t BYTES_PER_LINE = 32;
    static const size_t BITS_PER_PIXEL = 2;
    static const size_t DISPLAY_WIDTH = 128;

    LedDisplay(const std::string& device,
               size_t lines,
               const LedFont& font);
    ~LedDisplay();
    bool open(std::string & error);
    void setByte(size_t row, size_t col, unsigned char data);
    void setPixel(size_t xpos, size_t ypos, Color color);
    void flush(std::optional<size_t> line = {});
    size_t widthOfTxt(const std::string & text);
    bool writeCharacter(const std::string & character, Color color);
    void writeTxt(const std::string & text, Color color);
    void drawSprite(const Sprite & sprite, Color color);
    void send();

    std::vector<unsigned char> gfxBuffer_;
    int currentX_ = 0;
    int currentY_ = 0;
    const size_t displayHeight_ = 0;
    const size_t textLines_ = 0;

private:
    const std::string devicePath_;
    int deviceFileHandle_ = 0;
    const LedFont& font_;
};
