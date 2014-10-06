#include "leddisplay.hpp"
#include "ledfont.hpp"
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

LedDisplay::LedDisplay(const std::string & device,
                       size_t lines,
                       LedFont * font)
    : gfxBuffer_(1024)
    , currentX_(0)
    , currentY_(0)
    , devicePath_(device)
    , deviceFileHandle_(-9)
    , lines_(lines)
    , font_(font)
{
}

LedDisplay::~LedDisplay()
{
    ::close(deviceFileHandle_);
}

bool LedDisplay::open(std::string & error)
{
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
    tcflush(deviceFileHandle_, TCIFLUSH);
    tcsetattr(deviceFileHandle_, TCSANOW, &options);
    return true;
}

void LedDisplay::setByte(size_t row, size_t col, unsigned char data)
{
    assert(row*col < gfxBuffer_.size());
    gfxBuffer_[row*col] = data;
}

void LedDisplay::setPixel(size_t xpos, size_t ypos, Color color)
{
    const size_t col = xpos / 4;
    unsigned fraction = (xpos % 4) * 2;
    if (color == RED) {
        fraction += 1;
    }
    gfxBuffer_[ypos*col] |= 1 << (7-fraction);
    if (color == ORANGE) {
        gfxBuffer_[ypos*col] |= 1 << (7-fraction-1);
    }
}

void LedDisplay::flush(int line)
{
    if (line < 0) {
        memset(&gfxBuffer_[0], 0, gfxBuffer_.size());
    }
    else {
        assert(line < lines_);
        const size_t bytesPerLine = 32*8;
        memset(&gfxBuffer_[line*bytesPerLine], 0, bytesPerLine);
    }
}

void LedDisplay::writeTxt(const std::string & text, Color color)
{
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned bytes = 1;
        if ((unsigned char)text[i] >= 128) {
            // multibyte character (limited support)
            bytes = 2;
        }
        if (!font_->chars_.count(text.substr(i, bytes))) {
            std::cout << "Failed to lookup letter='" << text.substr(i, bytes) << "'" << std::endl;
            assert(false && "Missing font");
        }
        const FontLetter & letter = font_->chars_[text.substr(i, bytes)];
        drawSprite(letter, color);
        currentX_ += letter.spriteWidth_ + 1;
        if (bytes > 1) {
            ++i;
        }
    }
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
        const int dstCol = (currentX_ / 4) + (i % sprite.dataWidth_);
        const int dstRow = currentY_ + (i / sprite.dataWidth_);
        if (dstRow > DISPLAY_HEIGHT)
            break;
        const unsigned fraction = (currentX_ % 4)*BITS_PER_PIXEL;
        if (dstRow >= 0 && dstRow < DISPLAY_HEIGHT) {
            if (fraction) {
                if (dstCol >= 0 && dstCol < BYTES_PER_LINE) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)] |= (sprite.data_[i] & colorfilter) >> fraction;
                }
                if (dstCol+1 < BYTES_PER_LINE && dstCol+1 >= 0) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)+1] |= (sprite.data_[i] & colorfilter) << (8-fraction);
                }
            }
            else {
                if (dstCol >= 0 && dstCol < BYTES_PER_LINE) {
                    gfxBuffer_[dstCol+(dstRow*BYTES_PER_LINE)] |= sprite.data_[i] & colorfilter;
                }
            }
        }
    }
}

void LedDisplay::send()
{
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
    //std::cout << "Sent " << sent << " characters" << std::endl;
    assert(sent == (int)gfxBuffer_.size());
}
