/* FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2025-12-30
*/

#include "FlipperHTTP.hpp"

FlipperHTTP fhttp;

void setup()
{
  fhttp.setup();
}

void loop()
{
  fhttp.loop();
}