#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H
#include <Arduino.h>
#include <atomic>

struct SensorData
{
    std::atomic<unsigned int> m_currTemperature{0};
    std::atomic<unsigned int> m_currHumidity{0};
    std::atomic<unsigned int> m_currVoltage{0};
};
#endif
