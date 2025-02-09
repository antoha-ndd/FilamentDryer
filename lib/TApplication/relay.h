#pragma once
#include "outputdevice.h"

class TRelay : public TOutputDevice
{
public:
    TRelay(uint8_t _Pin) : TOutputDevice(_Pin){};
};