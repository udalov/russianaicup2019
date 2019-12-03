#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include "Stream.hpp"
#include "model/CustomData.hpp"
#include <string>

class Debug {
public:
  Debug(OutputStream &outputStream);
  void draw(const CustomData &customData);
  void log(const std::string& message) {
    draw(CustomData::Log(message));
  }

private:
  OutputStream &outputStream;
};

#endif
