#include "leddisplay.hpp"
#include "ledfont.hpp"
#include "sprite.hpp"
#include "http_fetchers.hpp"
#include "command_line.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <unistd.h>
#include <thread>

using namespace std::chrono_literals;

namespace {

constexpr size_t CROP_DEPARTUES_LIST_SIZE = 10;

bool timeComparator(const std::shared_ptr<Departure>& dep1, const std::shared_ptr<Departure>& dep2)
{
    return dep1->etaSeconds() < dep2->etaSeconds();
}

}

std::vector<std::shared_ptr<Departure>> fetchOsloDepartures()
{
    std::vector<std::shared_ptr<Departure>> res;
    std::vector<std::shared_ptr<Departure>> oksenoyveien = Trafikanten::fetchDeparture("2190040");
    std::vector<std::shared_ptr<Departure>> tjernsmyr = Trafikanten::fetchDeparture("2190105");
    std::vector<std::shared_ptr<Departure>> stabekkbakken = Trafikanten::fetchDeparture("2190106");
    std::cout << "Fetched " << oksenoyveien.size() << " deps for oksenoyveien" << std::endl;
    std::cout << "Fetched " << tjernsmyr.size() << " deps for tjernsmyr" << std::endl;
    std::cout << "Fetched " << stabekkbakken.size() << " deps for stabekkbakken" << std::endl;
    res.insert(res.end(), oksenoyveien.begin(), oksenoyveien.end());
    res.insert(res.end(), tjernsmyr.begin(), tjernsmyr.end());
    res.insert(res.end(), stabekkbakken.begin(), stabekkbakken.end());
    std::sort(res.begin(), res.end(), timeComparator);
    return res;
}

std::vector<std::shared_ptr<Departure>> fetchAalesundDepartures(const std::string& inputFilePath)
{
    std::vector<std::shared_ptr<Departure>> sjukehuslomma = Frammr::fetchDeparture(inputFilePath);
    std::cout << "Fetched " << sjukehuslomma.size() << " deps for sjukehuslomma" << std::endl;
    std::sort(sjukehuslomma.begin(), sjukehuslomma.end(), timeComparator);
    return sjukehuslomma;
}

template <typename T>
void smartFilter(std::vector<std::shared_ptr<T>> & deps)
{
    for (size_t i = 0; i < deps.size(); ++i) {
        if (i > CROP_DEPARTUES_LIST_SIZE && deps[i]->etaSeconds() >= (60s*30)) {
            deps.resize(i-1);
            return;
        }
    }
}

void drawBusList(LedDisplay& display,
                 const std::vector<std::shared_ptr<Departure>>& deps,
                 size_t count)
{
    if (count > deps.size()) {
        count = deps.size();
    }
    for (size_t j = 0; j < count; ++j) {
        const Departure& dep = *deps[j];
        display.currentX_ = 0;
        display.writeTxt(dep.lineNo_, LedDisplay::ORANGE);
        display.currentX_ += 2;
        display.writeTxt(dep.destinationDisplay_, LedDisplay::ORANGE);
        if (dep.etaSeconds_ < 60s) {
            display.currentX_ = 117;
            display.writeTxt("nå", LedDisplay::RED);
        }
        else {
            std::stringstream ss;
            ss << dep.etaSeconds_.count() / 60;
            switch (ss.str().size()) {
            case 1: display.currentX_ = 106; break;
            case 2: display.currentX_ = 99; break;
            case 3: display.currentX_ = 92; break;
            }
            display.writeTxt(ss.str() + "min", LedDisplay::ORANGE);
        }
        display.currentY_ += 8;
    }
}

std::string getAlbertText()
{
    time_t rawtime;
    time(&rawtime);
    int now = rawtime;
    int age = now - 1393336800;
    int days = age/(60*60*24)-2;
    int months = days / 30;
    int restDays = days % 30;

    std::stringstream ss;
    ss << "Albert is " << months << " months and "
       << restDays << " days old";
    return ss.str();
}

bool isSerial(const std::string& outputDevice)
{
    return !outputDevice.empty() && outputDevice != LedDisplay::DEVICE_NCURSES;
}

void verticalScrollList(LedDisplay& display, std::vector<std::shared_ptr<Departure>>& departures)
{
    display.currentX_ = 0;
    for (int i = display.displayHeight_; i > -((int)LedDisplay::PIXELS_PER_TEXTLINE*(int)departures.size()+1); --i)
    {
        display.flush();
        display.currentY_ = i;
        drawBusList(display, departures, departures.size());
        display.send();
        if (i % 16 == 0) {
            std::this_thread::sleep_for(1s);
        }
        std::this_thread::sleep_for(20ms);
    }
}

void listWithHorizontalScroll(LedDisplay& display, std::vector<std::shared_ptr<Departure>>& departures)
{
    for (size_t i = display.textLines_ - 1; i < departures.size(); ++i) {
        const auto& dep = *departures[i];
        // std::cout << "Scrolling departure #" << i << " " << departures[i]->destinationDisplay_ << std::endl;
        size_t widthOfScrollString =
            display.widthOfTxt(dep.lineNo_) +
            2 +
            display.widthOfTxt(dep.destinationDisplay_ + " ") + 50;

        for (int x = display.DISPLAY_WIDTH; x > -(static_cast<int>(widthOfScrollString)); --x) {

            display.flush();
            display.currentX_ = 0;
            display.currentY_ = 0;
            drawBusList(display, departures, display.textLines_ - 1);

            display.currentX_ = x;
            display.currentY_ = (display.textLines_ - 1) * display.PIXELS_PER_TEXTLINE;

            display.writeTxt(dep.lineNo_, LedDisplay::ORANGE);
            display.currentX_ += 2;
            display.writeTxt(dep.destinationDisplay_, LedDisplay::ORANGE);
            display.writeTxt(" ", LedDisplay::ORANGE);
            if (dep.etaSeconds_ < 60s) {
                display.currentX_ = 117;
                display.writeTxt("nå", LedDisplay::RED);
            } else {
                std::stringstream ss;
                ss << dep.etaSeconds_.count() / 60;
                display.writeTxt(ss.str() + "min", LedDisplay::ORANGE);
            }
            display.send();
            std::this_thread::sleep_for(20ms);
        }
    }
}

int main(int argc, char* argv[])
{
    std::string inputFilePath;
    std::string outputDevice;
    {
        CommandLine cmdParser("An application for displaying public transport realtime info");
        cmdParser.addArgument({"-i", "--inputFile"}, &inputFilePath, "Path to file with fake HTML data");
        cmdParser.addArgument({"-o", "--output"}, &outputDevice, "Output device: ncurses, terminal or serial device");
        try {
            cmdParser.parse(argc, argv);
        } catch (std::runtime_error const& e) {
            std::cout << e.what() << std::endl;
            return -1;
        }
    }
    std::string testTime = "/Date(1412619557000+0200)/";
    int t = Trafikanten::Utils::parseTime(testTime);
    std::cout << "time=" << t << std::endl;
    if (outputDevice.empty()) {
        std::cout << "Output device must be specified: --output /dev/ttyUSB0" << std::endl;
	return -1;
    }
    if (outputDevice == "ncurses") {
        outputDevice = LedDisplay::DEVICE_NCURSES;
    }
    LedFont busFont;
    LedDisplay display(
        outputDevice,
        32,
        busFont);
    std::string error;
    display.open(error);
    if (isSerial(outputDevice) && !error.empty()) {
        std::cout << "ERROR: Failed to open display device: " << error << std::endl;
        return 1;
    }
    int timeOfLastFetch = 0;
    std::vector<std::shared_ptr<Departure>> departures;
    while (true) {
        time_t now;
        time(&now);
        if ((now - timeOfLastFetch) > 30)
        {
            std::cout << "Data is >30s old. Fetching." << std::endl;
            departures = fetchAalesundDepartures(inputFilePath);
            if (departures.empty()) {
                display.flush();
                display.currentY_ = 0;
                display.currentX_ = 0;
                display.writeTxt("Se tidtabell...", LedDisplay::RED);
                display.send();
                std::this_thread::sleep_for(5s);
                continue;
            }
            else {
                timeOfLastFetch = now;
            }
            smartFilter(departures);
        }
        // verticalScrollList(display, departures);
        listWithHorizontalScroll(display, departures);
    }
    return 0;
}
