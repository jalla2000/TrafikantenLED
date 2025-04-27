#include "tests.hpp"
#include "http_fetchers.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

namespace tests {

bool testParseJson()
{
  std::cout << "Reading test JSON" << std::endl;
  std::ifstream file("testjson1.json");
  if (!file) {
    std::cerr << "Error opening file." << std::endl;
    return false;
  } else {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    auto departures = Frammr::jsonToDepartures(content, true);
    std::cout << "Number of departures found: " << departures.size() << std::endl;
  }
  return false;
}

}
