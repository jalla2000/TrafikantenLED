#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

class Departure {
public:
    virtual std::string str() const
    {
      std::ostringstream oss;
      oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds_;
      return oss.str();
    }
    void compressName();
    int etaSeconds() const { return etaSeconds_; }
    const std::string& lineNo() const { return lineNo_; }
    const std::string& destinationDisplay() const { return destinationDisplay_; }
    int expectedDepartureTime() const { return expectedDepartureTime_; }
// private:
    std::string lineNo_;
    std::string destinationDisplay_;
    int expectedDepartureTime_ = 0;
    int etaSeconds_ = 0;
};

class TrafikantenDeparture : public Departure {
public:
    virtual std::string str() const
    {
      std::ostringstream oss;
      oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds()
          << " dir=" << directionRef_ << (inCongestion_ ? " CONGESTION" : "");
      return oss.str();
    }
// private:
    std::string directionRef_;
    unsigned destinationRef_;
    bool inCongestion_ = false;
};

class FrammrDeparture : public Departure {
public:
    virtual std::string str() const override {
      std::ostringstream oss;
      oss << "LineNo=" << lineNo()
	  << " Dest=" << destinationDisplay()
	  << " EtaSeconds=" << etaSeconds()
	  << " Cancelled=" << cancelled_;
      return oss.str();
    }
    std::string id_;
    int aimedDepartureTime_ = 0;
    bool cancelled_ = false;
    bool realtime_ = true;
};

std::string httpRequest(const std::string & url);

namespace Trafikanten {
    std::vector<std::shared_ptr<Departure>> fetchDeparture(const std::string & stopId);
    namespace Utils {
      int parseTime(std::string t);
    }
}
namespace Frammr {
    std::vector<std::shared_ptr<Departure>> fetchDeparture(bool testMode);
}
