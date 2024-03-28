#include "datahandler.h"


StaticJsonDocument<200> graphSensorReading(float sensor_data[], size_t data_size) {
    const uint8_t cmd = 2;
    StaticJsonDocument<200> root;
    root["cmd"] = cmd;
    JsonArray arr = root.createNestedArray("data");
    for (size_t i = 0; i < data_size; i++) {
        arr.add(sensor_data[i]);
    }
    return root;
}