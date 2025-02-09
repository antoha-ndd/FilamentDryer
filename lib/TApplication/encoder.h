#pragma once
#include "Objects.h"

#include <GyverEncoder.h>
//Encoder enc1(CLK, DT);      // для работы без кнопки


#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class TEncoder;
void TEncoderEmptryEvent(TEncoder *Encoder){};

class TEncoder : public TControl,public Encoder
{
private:

 int value{0};

public:


    TEncoder(int CLK, int DT, int SW) : TControl(),Encoder(CLK, DT, SW)
    {
        setType(TYPE2);
        OnRight = TEncoderEmptryEvent;
        OnLeft = TEncoderEmptryEvent;
        OnHoldedRight = TEncoderEmptryEvent;
        OnHoldedLeft = TEncoderEmptryEvent;
        OnPress = TEncoderEmptryEvent;
        OnRelease = TEncoderEmptryEvent;
        OnHolded = TEncoderEmptryEvent;
    };   

    virtual void Idle(){
        tick();

        if (isRight()) OnRight(this);
        if (isLeft()) OnLeft(this);
        
        if (isRightH()) OnHoldedRight(this);
        if (isLeftH()) OnHoldedLeft(this);
        
        if (isPress()) OnPress(this);
        if (isRelease()) OnRelease(this);
        if (isHolded()) OnHolded(this);

    }

    void (*OnLeft)(TEncoder *Encoder);
    void (*OnRight)(TEncoder *Encoder);
    void (*OnHoldedLeft)(TEncoder *Encoder);
    void (*OnHoldedRight)(TEncoder *Encoder);
    void (*OnPress)(TEncoder *Encoder);
    void (*OnRelease)(TEncoder *Encoder);
    void (*OnHolded)(TEncoder *Encoder);

    ~TEncoder();
};

TEncoder::~TEncoder()
{

}
