#include "leddisplay.hpp"
#include "ledfont.hpp"
#include "sprite.hpp"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <map>
#include <cassert>
#include <cstring>

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

void fetchRealTimeData()
{
    std::string stopId = "3010531";
    std::string url = "http://reis.trafikanten.no/reisrest/realtime/getalldepartures/" + stopId;
    std::string content = httpRequest(url);
    //std::cout << "HTTP response: BEGIN" << content << "END" << std::endl;
    Json::Reader reader;
    Json::Value parsed;
    Json::StyledWriter styledWriter;
    if (reader.parse(content, parsed)) {
        std::cout << "parse success:\n";
        //std::cout << styledWriter.write(parsed) << std::endl;
        for (size_t i = 0; i < parsed.size(); ++i) {
            std::string lineNo = parsed[(int)i]["PublishedLineName"].asString();
            std::string destName = parsed[(int)i]["DestinationName"].asString();
            std::cout << lineNo << " " << destName << std::endl;
        }
    }
    else {
        std::cout << "PARSE ERROR" << std::endl;
    }
    //std::cout << "BusName=" << parser.getString("BusName") << std::endl;
}

int main()
{
    fetchRealTimeData();
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
