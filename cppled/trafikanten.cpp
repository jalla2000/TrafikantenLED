#include "leddisplay.hpp"
#include "ledfont.hpp"
#include "sprite.hpp"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <map>
#include <sstream>
#include <cassert>
#include <cstring>
#include <ctime>

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
    std::string str() {
        std::ostringstream oss;
        oss << lineNo_ << " " << destinationDisplay_ << " " << expectedDepartureTime_
            << " dir=" << directionRef_ << (inCongestion_ ? " CONGESTION" : "");
        return oss.str();
    }
    std::string lineNo_;
    std::string destinationDisplay_;
    std::string expectedDepartureTime_;
    std::string directionRef_;
    unsigned destinationRef_;
    bool inCongestion_;
};

std::vector<Departure> fetchDepartures()
{
    std::string stopId = "3010531";
    std::string url = "http://reis.trafikanten.no/reisrest/realtime/getalldepartures/" + stopId;
    std::string content = httpRequest(url);
    //std::cout << "HTTP response: BEGIN" << content << "END" << std::endl;
    Json::Reader reader;
    Json::Value parsed;
    Json::StyledWriter styledWriter;
    std::vector<Departure> departures;
    if (reader.parse(content, parsed)) {
        std::cout << "parse success:\n";
        std::cout << styledWriter.write(parsed[0]) << std::endl;
        for (size_t i = 0; i < parsed.size(); ++i) {
            departures.push_back(Departure());
            Departure & dep = departures.back();
            dep.lineNo_ = parsed[(int)i]["PublishedLineName"].asString();
            dep.destinationDisplay_ = parsed[(int)i]["DestinationDisplay"].asString();
            dep.expectedDepartureTime_ = parsed[(int)i]["ExpectedDepartureTime"].asString();
            dep.destinationRef_ = parsed[(int)i]["DestinationRef"].asUInt();
            dep.directionRef_ = parsed[(int)i]["DirectionRef"].asString();
            dep.inCongestion_ = parsed[(int)i]["InCongestion"].asBool();
            std::cout << dep.str() << std::endl;
        }
    }
    else {
        std::cout << "PARSE ERROR" << std::endl;
    }
    return departures;
    //std::cout << "BusName=" << parser.getString("BusName") << std::endl;
}

int main()
{
    time_t rawtime;
    time(&rawtime);
    std::cout << "time=" << rawtime << std::endl;
    //tm * ptm = gmtime(&rawtime);
    return 0;

    std::vector<Departure> departures = fetchDepartures();
    LedFont busFont;
    LedDisplay display("/dev/ttyUSB0", 4, &busFont);
    std::string error;
    display.open(error);
    if (!error.empty()) {
        std::cout << "ERROR" << std::endl;
        return 1;
    }
    Funky funk;
    while (true) {
        // for (int i = -50; i < 140; ++i) {
        //     display.currentX_ = i;
        //     display.currentY_ = i;
        //     display.flush(-1);
        //     display.drawSprite(funk, LedDisplay::RED);
        //     display.send();
        // }
        for (int i = 128; i > -160; --i) {
            display.flush(-1);
            display.currentX_ = i;
            display.currentY_ = 0;
            display.writeTxt("Hello dickheads!", LedDisplay::RED);
            display.currentX_ = i+10;
            display.currentY_ = 8;
            display.writeTxt("Hello dickheads!", LedDisplay::GREEN);
            display.currentX_ = i+20;
            display.currentY_ = 16;
            display.writeTxt("Hello dickheads!", LedDisplay::ORANGE);
            display.currentX_ = i+30;
            display.currentY_ = 24;
            display.writeTxt("Hello dickheads!", LedDisplay::RED);
            display.send();
        }
    }
    return 0;
}
