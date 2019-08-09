// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/LogManagerImpl.hpp"

using namespace testing;
using namespace MAT;

class TestLogManagerImpl : public LogManagerImpl
{
public:
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient)
      : TestLogManagerImpl(configuration, httpClient, false) { }
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient, bool deferSystemStart)
      : LogManagerImpl(configuration, httpClient, deferSystemStart) { }

   using LogManagerImpl::m_httpClient;
   using LogManagerImpl::m_ownHttpClient;
};

class TestHttpClient : public IHttpClient
{
   virtual IHttpRequest* CreateRequest() override { return nullptr; }
   virtual void SendRequestAsync(IHttpRequest*, IHttpResponseCallback*) override { }
   virtual void CancelRequestAsync(std::string const&) override { }
};

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_ConstructsOwnHttpClient)
{
   ILogConfiguration configuration;
   TestLogManagerImpl logManager { configuration, nullptr };
   ASSERT_NE(logManager.m_ownHttpClient, nullptr);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_HttpClientAndOwnHttpClientAreSame)
{
   ILogConfiguration configuration;
   TestLogManagerImpl logManager { configuration, nullptr };
   ASSERT_EQ(logManager.m_ownHttpClient.get(), logManager.m_httpClient);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_DoesNotConstructOwnHttpClient)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, true };
   ASSERT_EQ(logManager.m_ownHttpClient, nullptr);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_HttpClientIsSet)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, true };
   ASSERT_EQ(logManager.m_httpClient, &httpClient);
}

TEST(LogManagerImplTests, Constructor_DataViewerCollectionIsNotNullptr_DataViewerCollectionIsSet)
{
    ILogConfiguration configuration;
    TestHttpClient httpClient;
    TestLogManagerImpl logManager { configuration, &httpClient, true };
    ASSERT_NE(logManager.GetDataViewerCollection(), nullptr);
}