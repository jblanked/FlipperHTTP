/* FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-24
Updated: 2024-10-26
*/

#include "FlipperHTTPPico.h"

FlipperHTTP fhttp;

void setup()
{
    fhttp.setup();
}

void loop()
{
    fhttp.loop();
}