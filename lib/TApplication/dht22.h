#pragma once
#include "Objects.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

class Tdht22 : public TControl
{
private:
    uint8_t Pin;
    DHT_Unified *dht;
    float TemperatureValue{0};  
    float HumidityValue{0};  
    sensor_t sensor;
public:
    
    Tdht22(uint8_t _Pin) : TControl()
    {
        Pin = _Pin;
        dht = new DHT_Unified(Pin,DHT22 );
        dht->begin();
        dht->temperature().getSensor(&sensor);
        dht->humidity().getSensor(&sensor);
    };

    void UpdateValues()
    {
        sensors_event_t event;
        dht->temperature().getEvent(&event);
        TemperatureValue = event.temperature;

        dht->humidity().getEvent(&event);
        HumidityValue = event.relative_humidity;
    }


    float Temperature(bool Update = false){
        if(Update)
            UpdateValues();
        return TemperatureValue;
    }

    float Humidity(bool Update = false){
        if(Update)
            UpdateValues();
        return HumidityValue;
    }

    ~Tdht22();
};

Tdht22::~Tdht22()
{
    delete dht;
}
