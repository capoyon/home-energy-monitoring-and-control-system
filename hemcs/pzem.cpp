#include "pzem.h"

Pzem::Pzem(uint8_t rxPin, uint8_t txPin) : pzem(Serial2, rxPin, txPin) {
    // Constructor definition with initialization of PZEM004Tv30 object
}

float Pzem::voltage() {
    return pzem.voltage();
}

float Pzem::current() {
    return pzem.current();
}

float Pzem::power() {
    return pzem.power();
}

float Pzem::energy() {
    return pzem.energy();
}

float Pzem::frequency() {
    return pzem.frequency();
}

float Pzem::powerfactor() {
    return pzem.pf();
}
