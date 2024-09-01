#pragma once

#include <vector>
#include <string>

class Departure {
public:
    void compressName();
    std::string str();
    std::string lineNo_;
    std::string destinationDisplay_;
    int expectedDepartureTime_;
    int etaSeconds_;
    std::string directionRef_;
    unsigned destinationRef_;
    bool inCongestion_;
};

std::string httpRequest(const std::string & url);

namespace Trafikanten {
    std::vector<Departure> fetchDeparture(const std::string & stopId);
    namespace Utils {
      int parseTime(std::string t);
    }
}
namespace Frammr {
    std::vector<Departure> fetchDeparture();
}
