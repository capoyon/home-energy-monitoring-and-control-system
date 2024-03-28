#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <ArduinoJson.h>

StaticJsonDocument<200> graphSensorReading(float sensor_data[], size_t data_size);

#endif //DATAHANDLER_H