#pragma once

#include <Arduino.h>

enum class PowerSource : uint8_t
{
    PSU = 0,
    BATTERY = 1,
    USB = 2,
    UNKNOWN = 3
};

struct VoltageData
{
    float psu = 0.0f;
    float system = 0.0f;
    float battery = 0.0f;

    PowerSource source = PowerSource::UNKNOWN;

    bool valid = false;
    bool noAcError = false;  // PSU voltage < system voltage
    bool batteryOff = false; // Battery voltage < system voltage

    uint32_t lastUpdate = 0;
};

class VoltageMonitor
{
public:
    void Begin();
    void Update();

    const VoltageData& GetData() const;
    static const char* SourceName(PowerSource source);

private:
    static constexpr uint32_t SAMPLE_INTERVAL_MS = 200;
    static constexpr uint8_t ADC_SAMPLES = 8;

    VoltageData data_;

    float ReadVoltage(uint8_t pin) const;
    PowerSource DetectSource() const;
};