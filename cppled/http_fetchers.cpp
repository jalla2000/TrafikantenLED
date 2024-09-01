#include "http_fetchers.hpp"
#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include <iostream>

namespace {
static size_t httpCallback(void * buffer,
                           size_t size,
                           size_t nmemb,
                           void * userp)
{
    std::string * dest = static_cast<std::string*>(userp);
    *dest += std::string((char*)buffer, size*nmemb);
    return size*nmemb;
}
}


void Departure::compressName()
{
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


std::string Departure::str()
{
  std::ostringstream oss;
  oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds_
      << " dir=" << directionRef_ << (inCongestion_ ? " CONGESTION" : "");
  return oss.str();
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

namespace Trafikanten {

namespace Utils {

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
            dep.expectedDepartureTime_ = Trafikanten::Utils::parseTime(parsed[(int)i]["ExpectedDepartureTime"].asString());
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

}


std::vector<Departure> Frammr::fetchDeparture()
{
    // Not implemented
    return {};
}
