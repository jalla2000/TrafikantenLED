#include "leddisplay.hpp"
#include "ledfont.hpp"
#include "sprite.hpp"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <unistd.h>

static size_t httpCallback(void * buffer,
                           size_t size,
                           size_t nmemb,
                           void * userp)
{
    std::string * dest = static_cast<std::string*>(userp);
    *dest += std::string((char*)buffer, size*nmemb);
    return size*nmemb;
}

std::string httpRequest(const std::string & url)
{
    CURL *curlHandle = curl_easy_init();;
    std::string response;
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, httpCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curlHandle);
    curl_easy_cleanup(curlHandle);
    return std::string(response);
}

class Departure {
public:
    void compressName() {
        destinationDisplay_ = destinationDisplay_.substr(
            0,
            destinationDisplay_.find(" stasjon"));
        destinationDisplay_ = destinationDisplay_.substr(
            0,
            destinationDisplay_.find(" via"));
        if (destinationDisplay_.find("bussterminal") != std::string::npos) {
            destinationDisplay_ = destinationDisplay_.substr(
                0,
                destinationDisplay_.find("bussterminal")+8);
        }
        if (destinationDisplay_.find("Bussterminal") != std::string::npos) {
            destinationDisplay_ = destinationDisplay_.substr(
                0,
                destinationDisplay_.find("Bussterminal")+8);
        }
        if (destinationDisplay_.find("(N63") != std::string::npos) {
            destinationDisplay_ = destinationDisplay_.substr(
                0,
                destinationDisplay_.find("(N63")+4);
            destinationDisplay_ += ")";
        }
    }
    std::string str() {
        std::ostringstream oss;
        oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds_
            << " dir=" << directionRef_ << (inCongestion_ ? " CONGESTION" : "");
        return oss.str();
    }
    std::string lineNo_;
    std::string destinationDisplay_;
    int expectedDepartureTime_;
    int etaSeconds_;
    std::string directionRef_;
    unsigned destinationRef_;
    bool inCongestion_;
};

int parseTime(std::string t)
{
    // Typical input "/Date(1412619557000+0200)/"
    t = t.substr(t.find("Date(")+5);
    t = t.substr(0, t.find("+")-3);
    std::istringstream ss(t);
    int ret;
    ss >> ret;
    return ret;
}

std::vector<Departure> fetchDeparture(const std::string & stopId)
{
    std::string url = "http://reis.trafikanten.no/reisrest/realtime/getalldepartures/" + stopId;
    std::cout << "Performing HTTP request...";
    std::string content = httpRequest(url);
    std::cout << "done" << std::endl;
    //std::cout << "HTTP response: BEGIN" << content << "END" << std::endl;
    Json::Reader reader;
    Json::Value parsed;
    Json::StyledWriter styledWriter;
    std::vector<Departure> departures;
    if (reader.parse(content, parsed)) {
        std::cout << "JSON parse success" << std::endl;
        time_t rawtime;
        time(&rawtime);
        //tm * ptm = gmtime(&rawtime);
        int now = rawtime;
        for (size_t i = 0; i < parsed.size(); ++i) {
            // HACK: For forgotten reasons:
            if (parsed[(int)i]["DestinationDisplay"].asString().find("Oslo") != std::string::npos) {
            departures.push_back(Departure());
            Departure & dep = departures.back();
            dep.lineNo_ = parsed[(int)i]["PublishedLineName"].asString();
            dep.destinationDisplay_ = parsed[(int)i]["DestinationDisplay"].asString();
            dep.expectedDepartureTime_ = parseTime(parsed[(int)i]["ExpectedDepartureTime"].asString());
            dep.etaSeconds_ = dep.expectedDepartureTime_ - now;
            dep.destinationRef_ = parsed[(int)i]["DestinationRef"].asUInt();
            dep.directionRef_ = parsed[(int)i]["DirectionRef"].asString();
            dep.inCongestion_ = parsed[(int)i]["InCongestion"].asBool();
            //std::cout << dep.str() << std::endl;
            dep.compressName();
            }
        }
    }
    else {
        std::cout << "PARSE ERROR: " << content << std::endl;
    }
    return departures;
    //std::cout << "BusName=" << parser.getString("BusName") << std::endl;
}

bool timeComparator(const Departure & dep1, const Departure & dep2)
{
    return dep1.etaSeconds_ < dep2.etaSeconds_;
}

std::vector<Departure> fetchDepartures()
{
    std::vector<Departure> res;
    std::vector<Departure> oksenoyveien = fetchDeparture("2190040");
    std::vector<Departure> tjernsmyr = fetchDeparture("2190105");
    std::vector<Departure> stabekkbakken = fetchDeparture("2190106");
    std::cout << "Fetched " << oksenoyveien.size() << " deps for oksenoyveien" << std::endl;
    std::cout << "Fetched " << tjernsmyr.size() << " deps for tjernsmyr" << std::endl;
    std::cout << "Fetched " << stabekkbakken.size() << " deps for stabekkbakken" << std::endl;
    res.insert(res.end(), oksenoyveien.begin(), oksenoyveien.end());
    res.insert(res.end(), tjernsmyr.begin(), tjernsmyr.end());
    res.insert(res.end(), stabekkbakken.begin(), stabekkbakken.end());
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
    int t = parseTime(testTime);
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
            departures = fetchDepartures();
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
