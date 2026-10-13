#include "ConfigurationManager.h"


bool ConfigurationManager::Begin()
{
    //Try to open NVS
    if (!preferences_.begin(NVS_NAMESPACE, false))
    {
        reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
        initialized_ = false;
        return false;
    }

    //read the stored interval value
    //unsigned 16-bit value stored under NVS_KEY_INTERVAL.
    //the 2nd input param is default value (DEFAULT_REPORTING_INTERVAL_MIN)
    const uint16_t stored = preferences_.getUShort(
        NVS_KEY_INTERVAL,
        DEFAULT_REPORTING_INTERVAL_MIN
    );

    //Validate data
    //This prevents a corrupted or invalid configuration from producing an unreasonable reporting interval.
    if (stored > MAX_REPORTING_INTERVAL_MIN)
    {
        reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
    }
    else
    {
        reportingIntervalMinutes_ = stored;
    }

    // Serial.print("reportingIntervalMinutes_: ");
    // Serial.println(reportingIntervalMinutes_);

    #if DEBUG_SATELLITE_TX
        reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
    #endif

    // Serial.print("reportingIntervalMinutes_: ");
    // Serial.println(reportingIntervalMinutes_);

    initialized_ = true;
    return true;
}

uint16_t ConfigurationManager::GetReportingIntervalMinutes() const
{
    return reportingIntervalMinutes_;
}

uint32_t ConfigurationManager::GetReportingIntervalMs() const
{
    if (reportingIntervalMinutes_ == 0)
    {
        return 0;
    }

    return static_cast<uint32_t>(reportingIntervalMinutes_) * 60UL * 1000UL;
}

bool ConfigurationManager::SetReportingIntervalMinutes(uint16_t minutes)
{
    if (minutes < MIN_REPORTING_INTERVAL_MIN ||
        minutes > MAX_REPORTING_INTERVAL_MIN)
    {
        return false;
    }

    reportingIntervalMinutes_ = minutes;

    if (!initialized_)
    {
        return false;
    }

    //if write success and number of byte written is uint16_t
    //putUShort() returns the number of bytes written.
    return preferences_.putUShort(
        NVS_KEY_INTERVAL,
        reportingIntervalMinutes_
    ) == sizeof(uint16_t);
}
