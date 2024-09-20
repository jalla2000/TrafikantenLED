#include "leddisplay.hpp"
#include "ledfont.hpp"
#include <functional>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ncurses.h>

LedDisplay::LedDisplay(const std::string & device,
                       size_t displayHeight,
                       const LedFont& font)
    : gfxBuffer_(BYTES_PER_LINE*displayHeight)
    , currentX_(0)
    , currentY_(0)
    , displayHeight_(displayHeight)
    , textLines_(displayHeight/PIXELS_PER_TEXTLINE)
    , devicePath_(device)
    , deviceFileHandle_(-1)
    , font_(font)
{
    std::cout << "Created LED display"
              << " (width=" << LedDisplay::DISPLAY_WIDTH
              << ", height=" << displayHeight_ << ")" << std::endl;
}

LedDisplay::~LedDisplay()
{
    if (deviceFileHandle_ >= 0) {
        ::close(deviceFileHandle_);
    }
}

bool LedDisplay::open(std::string & error)
{
    if (devicePath_.empty()) {
        return false;
    } else if (devicePath_ == DEVICE_NCURSES) {
        initscr(); // ncurses
        return true;
    }
    deviceFileHandle_ = ::open(devicePath_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (deviceFileHandle_ == -1) {
        error = "Failed to open serial port. Aborting...";
        return false;
    }
    std::cout << "Successfully opened " << devicePath_ << " got fdes=" << deviceFileHandle_ << std::endl;
    termios options;
    tcgetattr(deviceFileHandle_, &options);
    // raw mode
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                         | INLCR | IGNCR | ICRNL);
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_cflag = 0;
    options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag |= CS8 | B460800;
    // options.c_cflag |= CS8 | B115200;
    tcflush(deviceFileHandle_, TCIFLUSH);
    tcsetattr(deviceFileHandle_, TCSANOW, &options);
    return true;
}

void LedDisplay::setByte(size_t row, size_t col, unsigned char data)
{
    assert(row*col < gfxBuffer_.size());
    assert(false && "Not properly implemented");
    gfxBuffer_[row*col] = data; // TODO: this is wrong
}

void LedDisplay::setPixel(size_t xpos, size_t ypos, Color color)
{
    const size_t col = xpos / 4;
    unsigned fraction = (xpos % 4) * 2;
    if (color == RED) {
        fraction += 1;
    }
    gfxBuffer_[col + ypos*BYTES_PER_LINE] |= 1 << (7-fraction);
    if (color == ORANGE) {
        gfxBuffer_[col + ypos*BYTES_PER_LINE] |= 1 << (7-fraction-1);
    }
}

void LedDisplay::flush(std::optional<size_t> line)
{
    if (line) {
        std::cout << "line=" << line.value() << std::endl;
        assert(line.value() < (displayHeight_/PIXELS_PER_TEXTLINE));
        assert(false && "Not implemented correctly");
        memset(&gfxBuffer_[line.value()*BYTES_PER_LINE], 0, BYTES_PER_LINE); // TODO: this is WRONG
    } else {
        memset(&gfxBuffer_[0], 0, gfxBuffer_.size());
    }
}

void foreachUtfCharacter(
    const std::string & text,
    std::function<bool (const std::string &)> func)
{
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned bytes = 1;
        if ((unsigned char)text[i] >= 128) {
            // multibyte character (limited support)
            bytes = 2;
        }
        if (!func(text.substr(i, bytes)))
            return;
        if (bytes > 1) {
            ++i;
        }
    }
}

struct WidthCounter {
    WidthCounter(const LedFont & font) : font_(font), width_(0), count_(0) { }
    bool sumWidth(const std::string & character) {
        if (!font_.chars_.count(character)) {
            std::cout << "Failed to lookup letter='" << character << "'" << std::endl;
            assert(false && "Missing font");
        }
        width_ += font_.chars_.at(character).spriteWidth_;
        count_++;
        return true;
    }
    const LedFont & font_;
    size_t width_;
    size_t count_;
};

size_t LedDisplay::widthOfTxt(const std::string & text)
{
    WidthCounter cnt(font_);
    foreachUtfCharacter(text, std::bind(&WidthCounter::sumWidth, &cnt, std::placeholders::_1));
    return cnt.width_ + cnt.count_-1;
}

bool LedDisplay::writeCharacter(const std::string & character,
                                Color color)
{
    if (currentX_ >= static_cast<int>(DISPLAY_WIDTH) || currentY_ >= static_cast<int>(displayHeight_)) {
        return false;
    }
    if (!font_.chars_.count(character)) {
        std::cout << "Failed to lookup letter='" << character << "'" << std::endl;
        assert(false && "Missing font");
    }
    const FontLetter& letter = font_.chars_.at(character);
    drawSprite(letter, color);
    currentX_ += letter.spriteWidth_ + 1;
    return true;
}

void LedDisplay::writeTxt(const std::string & text, Color color)
{
    if (currentX_ >= static_cast<int>(DISPLAY_WIDTH) || currentY_ >= static_cast<int>(displayHeight_)) {
        return;
    }
    foreachUtfCharacter(text, [this, color] (const std::string& character) -> bool {
        return writeCharacter(character, color);
    });
}

void LedDisplay::drawSprite(const Sprite & sprite, Color color)
{
    unsigned char colorfilter = 0b11111111;
    if (color == GREEN)
        colorfilter = 0b10101010;
    else if (color == RED)
        colorfilter = 0b01010101;

    for (size_t i = 0; i < sprite.data_.size(); ++i)
    {
        int base = currentX_;
        if (currentX_ < 0)
            base -= 3;
        const int dstCol = (base / 4) + (i % sprite.dataWidth_);
        const int dstRow = currentY_ + (i / sprite.dataWidth_);
        if (dstRow > static_cast<int>(displayHeight_))
            break;
        int fraction = (currentX_ % 4) *BITS_PER_PIXEL;
        if (fraction < 0)
            fraction += 8;
        if (dstRow >= 0 && dstRow < static_cast<int>(displayHeight_)) {
            if (fraction) {
                if (dstCol >= 0 && dstCol < static_cast<int>(BYTES_PER_LINE)) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)] |= (sprite.data_[i] & colorfilter) >> fraction;
                }
                if (dstCol+1 >= 0 && dstCol+1 < static_cast<int>(BYTES_PER_LINE)) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)+1] |= (sprite.data_[i] & colorfilter) << (8-fraction);
                }
            }
            else {
                if (dstCol >= 0 && dstCol < static_cast<int>(BYTES_PER_LINE)) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)] |= sprite.data_[i] & colorfilter;
                }
            }
        }
    }
}

void LedDisplay::send()
{
    if (devicePath_.empty()) {
        return;
    }
    /* ncurses begin */
    if (devicePath_ == DEVICE_NCURSES) {
        for (size_t i = 0; i < gfxBuffer_.size(); ++i) {
            const int row = i / BYTES_PER_LINE;
            for (size_t pixel = 0; pixel < 4; ++pixel) { // [0,3]
                const int col = (i % BYTES_PER_LINE)*4 + pixel;
                const bool set = gfxBuffer_[i] & (3 << (6-(pixel*2)));
                char block = (set ? 219 : ' ');
                mvaddch(row, col, block);
            }
        }
        refresh();
        return;
        /* ncurses end */
    }
    static const unsigned char sync = '0';
    int sent = write(deviceFileHandle_, &sync, sizeof sync);
    if (sent == -1)
        std::cout << "Oh dear, something went wrong with read()! " << strerror(errno) << std::endl;
    assert(sent == 1);
    sent = 0;
    while (sent != -1 && sent < (int)gfxBuffer_.size()) {
        //std::cout << "Sending " << gfxBuffer_.size()-sent << " bytes" << std::endl;
        sent += write(deviceFileHandle_, &gfxBuffer_[sent], (gfxBuffer_.size()-sent));
    }
    if (sent == -1) {
        std::cout << "Oh dear 2, something went wrong with read()! " << strerror(errno) << std::endl;
    }
    tcdrain(deviceFileHandle_);
    // std::cout << "Sent " << sent << " characters" << std::endl;
    assert(sent == (int)gfxBuffer_.size());
}
