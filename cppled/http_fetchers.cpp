#include "http_fetchers.hpp"
#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>

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

std::vector<std::shared_ptr<Departure>> fetchDeparture(const std::string & stopId)
{
    std::string url = "http://reis.trafikanten.no/reisrest/realtime/getalldepartures/" + stopId;
    std::cout << "Performing HTTP request...";
    std::string content = httpRequest(url);
    std::cout << "done" << std::endl;
    //std::cout << "HTTP response: BEGIN" << content << "END" << std::endl;
    Json::Reader reader;
    Json::Value parsed;
    Json::StyledWriter styledWriter;
    std::vector<std::shared_ptr<Departure>> departures;
    if (reader.parse(content, parsed)) {
        std::cout << "JSON parse success" << std::endl;
        time_t rawtime;
        time(&rawtime);
        //tm * ptm = gmtime(&rawtime);
        int now = rawtime;
        for (size_t i = 0; i < parsed.size(); ++i) {
            // HACK: For forgotten reasons:
            if (parsed[(int)i]["DestinationDisplay"].asString().find("Oslo") != std::string::npos) {
	      auto departure = std::make_shared<TrafikantenDeparture>();
	      auto& dep = *departure;
	      dep.lineNo_ = parsed[(int)i]["PublishedLineName"].asString();
	      dep.destinationDisplay_ = parsed[(int)i]["DestinationDisplay"].asString();
	      dep.expectedDepartureTime_ = Trafikanten::Utils::parseTime(parsed[(int)i]["ExpectedDepartureTime"].asString());
	      dep.etaSeconds_ = dep.expectedDepartureTime_ - now;
	      dep.destinationRef_ = parsed[(int)i]["DestinationRef"].asUInt();
	      dep.directionRef_ = parsed[(int)i]["DirectionRef"].asString();
	      dep.inCongestion_ = parsed[(int)i]["InCongestion"].asBool();
	      //std::cout << dep.str() << std::endl;
	      dep.compressName();
	      departures.push_back(std::dynamic_pointer_cast<Departure>(departure));
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


std::vector<std::shared_ptr<Departure>> Frammr::fetchDeparture(bool testMode)
{
    std::string content;
    if (testMode) {
        constexpr auto fileName = "response_raw.txt";
        // constexpr auto fileName = "response_json_formatted_2.txt";
	std::cout << "Reading cached response" << std::endl;
        std::ifstream file(fileName);
        if (!file) {
            std::cerr << "Error opening file." << std::endl;
            return {};
        } else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            content = buffer.str();
	}
    } else {
        std::string url = "https://reise.frammr.no/departures/NSR%3AStopPlace%3A40489?searchMode=now";
        std::cout << "Performing HTTP request...";
        content = httpRequest(url);
        std::cout << "done" << std::endl;
	std::cout << "HTTP response: BEGIN" << content << "END" << std::endl;
    }
    const auto responseSize = content.length();
    bool jsonFound = false;
    {
        const std::string startChunk = R"--(type="application/json">)--";
        const auto jsonStartIndex = content.find(startChunk);
        if (jsonStartIndex != std::string::npos) {
            content = content.substr(jsonStartIndex + startChunk.size());
            const auto jsonEndIndex = content.find("</script>");
            if (jsonEndIndex != std::string::npos) {
                content = content.substr(0, jsonEndIndex);
		jsonFound = true;
            }
        }
    }
    std::cout << "HTTP response was " << responseSize << " bytes" << std::endl;
    if (jsonFound) {
        std::cout << "JSON chunk found" << std::endl;
    } else {
        std::cout << "Failed to find JSON chunk! HTTP response: BEGIN" << content << "END" << std::endl;
	return {};
    }
    Json::Reader reader;
    Json::Value parsed;
    Json::StyledWriter styledWriter;
    std::vector<std::shared_ptr<Departure>> departures;
    if (reader.parse(content, parsed)) {
        std::cout << "JSON parse success" << std::endl;
    } else {
        std::cout << "Failed to parse JSON: " << content << std::endl;
	return {};
    }
    
    const auto& jsonProps = parsed["props"];
    const auto& jsonPageProps = jsonProps["pageProps"];
    const auto& jsonDepartures = jsonPageProps["departures"];
    const auto& jsonQuays = jsonDepartures["quays"];
    for (const auto& quay : jsonQuays) {
      const std::string quayName = quay["name"].asString();
      const std::string quayId = quay["id"].asString();
      for (const auto& jsonDeparture : quay["departures"]) {
	auto departure = std::make_shared<FrammrDeparture>();
	auto& dep = *departure;
	dep.id_ = jsonDeparture["id"].asString();
	dep.destinationDisplay_ = jsonDeparture["destinationDisplay"]["frontText"].asString();
	dep.lineNo_ = jsonDeparture["publicCode"].asString();
	dep.aimedDepartureTime_ = 123; // jsonDeparture["aimedDepartureTime"].asString();
	dep.expectedDepartureTime_ = 456; // jsonDeparture["expectedDepartureTime"].asString();
	dep.cancelled_ = jsonDeparture["cancelled"].asBool();
	dep.realtime_ = jsonDeparture["realtime"].asBool();
	std::cout << "Parsed departure: " << dep.str() << std::endl;
	departures.push_back(std::dynamic_pointer_cast<Departure>(departure));
      }
    }
    return departures;
}