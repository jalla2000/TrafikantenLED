#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <memory>

class Departure {
public:
    virtual std::string str() const
    {
      std::ostringstream oss;
      oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds_.count();
      return oss.str();
    }
    void compressName();
    std::chrono::seconds etaSeconds() const { return etaSeconds_; }
    const std::string& lineNo() const { return lineNo_; }
    const std::string& destinationDisplay() const { return destinationDisplay_; }
    std::chrono::seconds expectedDepartureTime() const { return expectedDepartureTime_; }
    std::string expectedDepartureTimeString() const { return expectedDepartureTimeString_; }
// private:
    std::string lineNo_;
    std::string destinationDisplay_;
    std::string expectedDepartureTimeString_;
    std::chrono::seconds expectedDepartureTime_{0};
    std::chrono::seconds etaSeconds_{0};
};

class TrafikantenDeparture : public Departure {
public:
    virtual std::string str() const
    {
      std::ostringstream oss;
      oss << lineNo_ << " " << destinationDisplay_ << " " << etaSeconds().count()
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
    << " DepartureString=" << expectedDepartureTimeString()
    << " DepartureTime=" << expectedDepartureTime().count()
	  << " EtaSeconds=" << etaSeconds().count()
	  << (cancelled_ ? " Cancelled=TRUE" : "");
      return oss.str();
    }
    std::string id_;
    std::string aimedDepartureTimeString_;
    std::chrono::seconds aimedDepartureTime_{0};
    bool cancelled_ = false;
    bool realtime_ = true;
};

std::string httpRequest(const std::string & url);

namespace Trafikanten {
    std::vector<std::shared_ptr<Departure>> fetchDeparture(const std::string& stopId);
    namespace Utils {
      int parseTime(std::string t);
    }
}
namespace Frammr {
    std::vector<std::shared_ptr<Departure>> fetchDeparture(const std::string& inputFilePath);
}
