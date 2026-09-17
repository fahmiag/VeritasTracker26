#include "VoltageMonitor.h"
#include "../../config/PinMap.h"

namespace
{
    constexpr float ADC_GAIN = 0.00417f;
    constexpr float ADC_OFFSET = 0.53f;

    // Adjust these thresholds after checking actual voltages.
    constexpr float PSU_ON_V = 10.0f;
    constexpr float PSU_OFF_V = 9.5f;

    constexpr float BATTERY_AVAILABLE_V = 10.0f;

    constexpr float SYSTEM_HIGH_V = 5.5f;
    constexpr float SYSTEM_LOW_V = 5.0f;

    // Reject an unpowered input, including the calibration offset.
    constexpr float SYSTEM_POWERED_V = 1.0f;
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

    data_.source = DetectSource();

    // Here, "no AC" means the measured PSU voltage is below
    // its operating threshold. It does not directly measure mains.
    const float psuThreshold =
        data_.noAcError ? PSU_ON_V : PSU_OFF_V;

    data_.noAcError = data_.psu < psuThreshold;

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
    if (data_.system < SYSTEM_POWERED_V)
    {
        return PowerSource::UNKNOWN;
    }

    // Hysteresis prevents repeated switching near 10 V.
    const float psuThreshold =
        data_.source == PowerSource::PSU
            ? PSU_OFF_V
            : PSU_ON_V;

    if (data_.psu >= psuThreshold)
    {
        return PowerSource::PSU;
    }

    const float batterySystemThreshold =
        data_.source == PowerSource::BATTERY
            ? SYSTEM_LOW_V
            : SYSTEM_HIGH_V;

    if (data_.battery >= BATTERY_AVAILABLE_V &&
        data_.system >= batterySystemThreshold)
    {
        return PowerSource::BATTERY;
    }

    const float usbSystemThreshold =
        data_.source == PowerSource::USB
            ? SYSTEM_HIGH_V
            : SYSTEM_LOW_V;

    if (data_.system <= usbSystemThreshold)
    {
        return PowerSource::USB;
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