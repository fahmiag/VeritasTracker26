#pragma once

#include <Arduino.h>
#include <Preferences.h>

class ConfigurationManager
{
public:
    #if DEBUG_SATELLITE_TX //1 is debug
        static constexpr uint16_t DEFAULT_REPORTING_INTERVAL_MIN = 1;
    #else
        static constexpr uint16_t DEFAULT_REPORTING_INTERVAL_MIN = 5;
    #endif

    static constexpr uint16_t MIN_REPORTING_INTERVAL_MIN = 0;
    static constexpr uint16_t MAX_REPORTING_INTERVAL_MIN = 1440;

    bool Begin();

    uint16_t GetReportingIntervalMinutes() const;
    uint32_t GetReportingIntervalMs() const;

    // 0 = telemetry OFF
    bool SetReportingIntervalMinutes(uint16_t minutes);

private:
    static constexpr const char* NVS_NAMESPACE = "vts_config";
    static constexpr const char* NVS_KEY_INTERVAL = "tx_interval";

    Preferences preferences_;
    uint16_t reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
    bool initialized_ = false;
};
