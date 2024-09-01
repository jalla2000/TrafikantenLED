#include "leddisplay.hpp"
#include "ledfont.hpp"
#include "sprite.hpp"
#include "http_fetchers.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <unistd.h>

bool timeComparator(const Departure & dep1, const Departure & dep2)
{
    return dep1.etaSeconds_ < dep2.etaSeconds_;
}

std::vector<Departure> fetchOsloDepartures()
{
    std::vector<Departure> res;
    std::vector<Departure> oksenoyveien = Trafikanten::fetchDeparture("2190040");
    std::vector<Departure> tjernsmyr = Trafikanten::fetchDeparture("2190105");
    std::vector<Departure> stabekkbakken = Trafikanten::fetchDeparture("2190106");
    std::cout << "Fetched " << oksenoyveien.size() << " deps for oksenoyveien" << std::endl;
    std::cout << "Fetched " << tjernsmyr.size() << " deps for tjernsmyr" << std::endl;
    std::cout << "Fetched " << stabekkbakken.size() << " deps for stabekkbakken" << std::endl;
    res.insert(res.end(), oksenoyveien.begin(), oksenoyveien.end());
    res.insert(res.end(), tjernsmyr.begin(), tjernsmyr.end());
    res.insert(res.end(), stabekkbakken.begin(), stabekkbakken.end());
    std::sort(res.begin(), res.end(), timeComparator);
    return res;
}

std::vector<Departure> fetchAalesundDepartures()
{
    std::vector<Departure> res;
    std::vector<Departure> sjukehuslomma = Frammr::fetchDeparture();
    std::cout << "Fetched " << sjukehuslomma.size() << " deps for sjukehuslomma" << std::endl;
    res.insert(res.end(), sjukehuslomma.begin(), sjukehuslomma.end());
    std::sort(res.begin(), res.end(), timeComparator);
    return res;
}

void smartFilter(std::vector<Departure> & deps)
{
    for (size_t i = 0; i < deps.size(); ++i) {
        if (i > 15 && deps[i].etaSeconds_ >= 60*30) {
            deps.resize(i-1);
            return;
        }
    }
}

void drawBusList(LedDisplay & display,
                 const std::vector<Departure> & deps,
                 size_t count)
{
    if (count > deps.size())
        count = deps.size();
    for (size_t j = 0; j < count; ++j)
    {
        const Departure & dep = deps[j];
        display.currentX_ = 0;
        display.writeTxt(dep.lineNo_, LedDisplay::ORANGE);
        display.currentX_ += 2;
        display.writeTxt(dep.destinationDisplay_, LedDisplay::ORANGE);
        if (dep.etaSeconds_ < 60) {
            display.currentX_ = 117;
            display.writeTxt("nå", LedDisplay::RED);
        }
        else {
            std::stringstream ss;
            ss << dep.etaSeconds_ / 60;
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

int main()
{
    std::string testTime = "/Date(1412619557000+0200)/";
    int t = Trafikanten::Utils::parseTime(testTime);
    std::cout << "time=" << t << std::endl;
    //return 0;
    LedFont busFont;
    LedDisplay display("/dev/ttyUSB0", 16, &busFont);
    std::string error;
    display.open(error);
    if (!error.empty()) {
        std::cout << "ERROR: Failed to open display device: " << error << std::endl;
        return 1;
    }
    int timeOfLastFetch = 0;
    std::vector<Departure> departures;
    while (true) {
        time_t now;
        time(&now);
        if ((now - timeOfLastFetch) > 30)
        {
            std::cout << "Data is >30 old. Fetching." << std::endl;
            departures = fetchOsloDepartures();
            if (departures.empty()) {
                display.flush(-1);
                display.currentY_ = 0;
                display.currentX_ = 0;
                display.writeTxt("Se tidtabell...", LedDisplay::RED);
                display.send();
                sleep(5);
                continue;
            }
            else {
                timeOfLastFetch = now;
            }
            smartFilter(departures);
        }
            display.currentX_ = 0;
            for (int i = display.displayHeight_; i > -(LedDisplay::PIXELS_PER_TEXTLINE*(int)departures.size()+1); --i)
            {
                display.flush(-1);
                display.currentY_ = i;
                drawBusList(display, departures, departures.size());
                display.send();
                if (i % 16 == 0) {
                    usleep(1000*1000);
                }
                usleep(1000*20);
            }
    }
    return 0;
}
