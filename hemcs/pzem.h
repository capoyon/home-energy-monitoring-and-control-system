// pzem.h
#ifndef PZEM_H_INCLUDED
#define PZEM_H_INCLUDED

#include <Arduino.h>
#include <PZEM004Tv30.h>

class Pzem {
public:
    Pzem(uint8_t rxPin, uint8_t txPin); // Constructor declaration with pin parameters
    float voltage();
    float current();
    float power();
    float energy();
    float frequency();
    float powerfactor();
    
private:
    PZEM004Tv30 pzem;
};

#endif // PZEM_H_INCLUDED
