#include "Debug.hpp"
#include "model/PlayerMessageGame.hpp"

Debug::Debug(OutputStream& outputStream) : outputStream(outputStream) {}

void Debug::draw(const CustomData &customData) {
    outputStream.write(0);
    customData.writeTo(outputStream);
    outputStream.flush();
}
