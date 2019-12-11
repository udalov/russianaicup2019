#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

#include "Stream.hpp"
#include "model/CustomData.hpp"

class Debug {
public:
  Debug(OutputStream &outputStream);
  void draw(const CustomData &customData);

private:
  OutputStream &outputStream;
};

#endif
