// Copyright (c) Microsoft. All rights reserved.
#include <string.h>

#include "LogSessionDataDB.hpp"
 #include "IOfflineStorage.hpp"
#include "pal/PAL.hpp"
#include "DebugTrace.hpp"

#include "utils/Utils.hpp"
#include "utils/StringUtils.hpp"

#include <memory>

namespace ARIASDK_NS_BEGIN {

#define LOG_SESSION_FIRSTLAUNCHTIME_NAME "sessionfirstlaunchtime"
#define LOG_SESSION_SDKUID_NAME "sessionsdkuid"


    LogSessionDataDB::LogSessionDataDB(IOfflineStorage* offlineStorage):
        m_sessionSDKUid(""),
        m_sessionFirstTimeLaunch(0ull),
        m_isDBInitialized(false)
    {
        if (offlineStorage == nullptr)
        {
            LOG_WARN("offlineStorage was not set.");
            return;
        }
        m_offlineStorage = offlineStorage;
    }

    void LogSessionDataDB::Initialize()
    {
        m_sessionSDKUid = offlineStorage->GetSetting(LOG_SESSION_SDKUID_NAME);
        auto firsttimelaunch = offlineStorage->GetSetting(LOG_SESSION_FIRSTLAUNCHTIME_NAME); 
        validateAndSetSdkId(firsttimelaunch);
        if (!m_sessionFirstTimeLaunch || m_sessionSDKUid.empty())
        {
            m_sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
            m_sessionSDKUid = PAL::generateUuidString();
            setSessionData(offlineStorage);
        }
    }

    void LogSessionDataDB::validateAndSetSdkId(const std::string &firsttimelaunch)
    {
        try
        {
            m_sessionFirstTimeLaunch = std::stoull(firsttimelaunch);
        }
        catch (const std::invalid_argument&)
        {
            LOG_WARN("Non-integer data passed to std::stoull");
        }
        catch (const std::out_of_range&)
        {
            LOG_WARN("Value passed to std::stoull was larger than unsigned long long could represent");
        }
    }

    void LogSessionDataDB::setSessionData(IOfflineStorage* offlineStorage)
    {
        if (!offlineStorage->StoreSetting(SESSION_FIRSTLAUNCHTIME_NAME, std::to_string(m_sessionFirstTimeLaunch)))
        {
           LOG_WARN("Unable to save session analytics to DB for %s",SESSION_FIRSTLAUNCHTIME_NAME);
           return;
        }

        if (!offlineStorage->StoreSetting(SESSION_SDKUID_NAME, m_sessionSDKUid))
        {
            LOG_WARN("Unable to save session analytics to DB for %s",SESSION_SDKUID_NAME);
            return;
        }
    }

    unsigned long long LogSessionDataDB::getSessionFirstTime() const
    {
        if (!m_isDBInitialized)
        {
            Initialize();
        }
        return m_sessionFirstTimeLaunch;
    }

    std::string LogSessionDataDB::getSessionSDKUid() const
    {
        if (!m_isDBInitialized)
        {
            Initialize();
        }
        return m_sessionSDKUid;
    }
} ARIASDK_NS_END