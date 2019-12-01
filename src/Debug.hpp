#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include "Stream.hpp"
#include "model/CustomData.hpp"
#include <memory>
#include <string>

class Debug {
public:
  Debug(const std::shared_ptr<OutputStream> &outputStream);
  void draw(const CustomData &customData);
  void log(const std::string& message) {
    draw(CustomData::Log(message));
  }

private:
  std::shared_ptr<OutputStream> outputStream;
};

#endif
