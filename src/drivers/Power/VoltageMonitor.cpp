#include "VoltageMonitor.h"
#include "../../config/PinMap.h"

namespace
{
    constexpr float ADC_GAIN = 0.00417f;
    constexpr float ADC_OFFSET = 0.53f;

    constexpr float USB_THRESHOLD_V = 5.0f;
}

void VoltageMonitor::Begin()
{
    pinMode(PinMap::VOLTAGE_PSU, INPUT);
    pinMode(PinMap::VOLTAGE_SYSTEM, INPUT);
    pinMode(PinMap::VOLTAGE_BATTERY, INPUT);

    // The calibration formula must match this ADC configuration.
    analogReadResolution(12);

    analogSetPinAttenuation(PinMap::VOLTAGE_PSU, ADC_11db);
    analogSetPinAttenuation(PinMap::VOLTAGE_SYSTEM, ADC_11db);
    analogSetPinAttenuation(PinMap::VOLTAGE_BATTERY, ADC_11db);

    data_ = VoltageData{};

    // Take the first sample immediately.
    Update();
}

void VoltageMonitor::Update()
{
    const uint32_t now = millis();

    if (data_.valid &&
        now - data_.lastUpdate < SAMPLE_INTERVAL_MS)
    {
        return;
    }

    data_.psu = ReadVoltage(PinMap::VOLTAGE_PSU);
    data_.system = ReadVoltage(PinMap::VOLTAGE_SYSTEM);
    data_.battery = ReadVoltage(PinMap::VOLTAGE_BATTERY);

    data_.noAcError = data_.psu < data_.system;
    data_.batteryOff = data_.battery < data_.system;

    data_.source = DetectSource();

    data_.lastUpdate = now;
    data_.valid = true;
}

float VoltageMonitor::ReadVoltage(uint8_t pin) const
{
    // Discard one reading after switching ADC channels.
    analogRead(pin);

    uint32_t total = 0;

    for (uint8_t i = 0; i < ADC_SAMPLES; ++i)
    {
        total += analogRead(pin);
    }

    const float average =
        static_cast<float>(total) / ADC_SAMPLES;

    return average * ADC_GAIN + ADC_OFFSET;
}
PowerSource VoltageMonitor::DetectSource() const
{
    // USB takes priority when system voltage is below 5 V.
    if (data_.system < USB_THRESHOLD_V)
    {
        return PowerSource::USB;
    }

    // PSU is available when its voltage is at least system voltage.
    if (data_.psu >= data_.system)
    {
        return PowerSource::PSU;
    }

    // Battery is available when its voltage is at least system voltage.
    if (data_.battery >= data_.system)
    {
        return PowerSource::BATTERY;
    }

    return PowerSource::UNKNOWN;
}

const VoltageData& VoltageMonitor::GetData() const
{
    return data_;
}

const char* VoltageMonitor::SourceName(PowerSource source)
{
    switch (source)
    {
        case PowerSource::PSU:
            return "PSU";

        case PowerSource::BATTERY:
            return "BATTERY";

        case PowerSource::USB:
            return "USB";

        default:
            return "UNKNOWN";
    }
}