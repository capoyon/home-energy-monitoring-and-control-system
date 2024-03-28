#ifndef PZEM_H
#define PZEM_H

#include <Arduino.h>
#include <PZEM004Tv30.h>

void readPzem();

extern float voltage;
extern float current;
extern float power;
extern float energy;
extern float frequency;
extern float pf;

#endif //PZEM_H