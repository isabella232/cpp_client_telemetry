#pragma once
#include "Version.hpp"
#include "Enums.hpp"
#include "INetworkInformation.hpp"
#include "InformationProviderimpl.hpp"


namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

    class NetworkInformationImpl : public INetworkInformation
    {
    public:
        static INetworkInformation* Create();

        // IInformationProvider API
        virtual int  RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback) { m_registredCount++; return m_info_helper.RegisterInformationChangedCallback(pCallback); }
        virtual void UnRegisterInformationChangedCallback(int callbackToken) { --m_registredCount;  m_info_helper.UnRegisterInformationChangedCallback(callbackToken); }

        // INetworkInformation API
        virtual std::string const& GetNetworkProvider() { return m_provider; };
        virtual NetworkType GetNetworkType() { return m_type; }
        virtual NetworkCost GetNetworkCost() { return m_cost; }

        virtual bool IsEthernetAvailable();
        virtual bool IsWifiAvailable();
        virtual bool IsWwanAvailable();

        NetworkInformationImpl();
        virtual ~NetworkInformationImpl() { };

    protected:
        std::string m_provider;
        NetworkType m_type;
        NetworkCost m_cost;
        int         m_registredCount;

        InformatonProviderImpl m_info_helper;

        // Disable copy constructor and assignment operator.
        NetworkInformationImpl(NetworkInformationImpl const& other) = delete;
        NetworkInformationImpl& operator=(NetworkInformationImpl const& other) = delete;
    };

} // PlatformAbstraction
}}}