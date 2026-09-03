#include "ConfigurationManager.h"

bool ConfigurationManager::Begin()
{
    if (!preferences_.begin(NVS_NAMESPACE, false))
    {
        reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
        initialized_ = false;
        return false;
    }

    const uint16_t stored = preferences_.getUShort(
        NVS_KEY_INTERVAL,
        DEFAULT_REPORTING_INTERVAL_MIN
    );

    if (stored > MAX_REPORTING_INTERVAL_MIN)
    {
        reportingIntervalMinutes_ = DEFAULT_REPORTING_INTERVAL_MIN;
    }
    else
    {
        reportingIntervalMinutes_ = stored;
    }

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

    return preferences_.putUShort(
        NVS_KEY_INTERVAL,
        reportingIntervalMinutes_
    ) == sizeof(uint16_t);
}
