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
    }
    std::string str() {
        std::ostringstream oss;
        oss << lineNo_ << " " << destinationDisplay_ << " " << expectedDepartureTime_
            << " dir=" << directionRef_ << (inCongestion_ ? " CONGESTION" : "");
        return oss.str();
    }
    std::string lineNo_;
    std::string destinationDisplay_;
    std::string expectedDepartureTime_;
    unsigned int etaSeconds_;
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
        time_t rawtime;
        time(&rawtime);
        //tm * ptm = gmtime(&rawtime);
        //int a = rawtime;
        for (size_t i = 0; i < parsed.size(); ++i) {
            departures.push_back(Departure());
            Departure & dep = departures.back();
            dep.lineNo_ = parsed[(int)i]["PublishedLineName"].asString();
            dep.destinationDisplay_ = parsed[(int)i]["DestinationDisplay"].asString();
            dep.expectedDepartureTime_ = parsed[(int)i]["ExpectedDepartureTime"].asString();
            dep.destinationRef_ = parsed[(int)i]["DestinationRef"].asUInt();
            dep.directionRef_ = parsed[(int)i]["DirectionRef"].asString();
            dep.inCongestion_ = parsed[(int)i]["InCongestion"].asBool();
            //std::cout << dep.str() << std::endl;
            dep.compressName();
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
    std::string testTime = "/Date(1412619557000+0200)/";
    int t = parseTime(testTime);
    std::cout << "time=" << t << std::endl;
    return 0;
    LedFont busFont;
    LedDisplay display("/dev/ttyUSB0", 4, &busFont);
    std::string error;
    display.open(error);
    if (!error.empty()) {
        std::cout << "ERROR: Failed to open tty device" << std::endl;
        return 1;
    }
    // for (int i = 0; i < 40; ++i) {
    //     //const Departure & dep = departures[0];
    //     display.flush(-1);
    //     display.currentX_ = 0;
    //     display.currentY_ = 0;
    //     display.writeTxt("Bygdøy", LedDisplay::RED);
    //     display.send();
    // }
    // return 0;
    std::vector<Departure> departures = fetchDepartures();
    Funky funk;
    while (true) {
    for (int i = 32; i > -(LedDisplay::PIXELS_PER_TEXTLINE*(int)departures.size()); --i)
    {
        display.flush(-1);
        for (size_t j = 0; j < departures.size(); ++j)
        {
            const Departure & dep = departures[j];
            display.currentX_ = 0;
            display.currentY_ = i+(j*8);
            display.writeTxt(dep.lineNo_, LedDisplay::ORANGE);
            display.currentX_ = 16;
            display.writeTxt(dep.destinationDisplay_, LedDisplay::ORANGE);
        }
        display.send();
        //usleep(1000*20);
    }
    }
    return 0;
}
