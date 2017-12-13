#include "Version.hpp"
#include "pal\InformationProviderImpl.hpp"

#include <vector>
#ifdef _WIN32
// Add SEH handling to callbacks to avoid crashes on Windows 10 Phone as a temporary
// solution to cover unknown Windows 10 Phone scenarios, where app goes to background,
// being shutdown or unloaded. And callback (power or network) fires at this time,
// resulting in invalid pointer read from OnChanged. Worst case is instead of a crash
// we would not refresh the actual network 
#include <exception>
#include <excpt.h>
#endif

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

InformatonProviderImpl::InformatonProviderImpl():m_registredCount(0)
{
}

InformatonProviderImpl::~InformatonProviderImpl()
{
}

// IInformationProvider API
int InformatonProviderImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
{
	std::lock_guard<std::mutex> lock(m_lock);
    // Use (index + 1) as the token.
    m_callbacks.push_back(pCallback);
    int token = (int)m_callbacks.size();
    m_registredCount++;
    return token;
}

void InformatonProviderImpl::UnRegisterInformationChangedCallback(int callbackToken)
{
    size_t index = (size_t)(callbackToken - 1);

	std::lock_guard<std::mutex> lock(m_lock);
    size_t count = m_callbacks.size();
    if (index > 0 && index < count)
    {
        // Don't ever delete the item from the vector.
        // Just set it to NULL.
        m_callbacks[index] = NULL;
        --m_registredCount;
    }
}

void InformatonProviderImpl::OnChanged(std::string const& propertyName, std::string const& propertyValue)
{
    try {
        // OnChange shouldn't block new callbacks to be registered.
        // However, those newly registered callbacks are not called by
        // this current notification.
        //
        // OnChange shouldn't block unregistration of callbacks.  However,
        // those callbacks will be called by this current notification?
        // TODO: This means that after calling UnRegisterInformationChangedCallback
        // the client code is not sure if the IPropertyChangedCallback can
        // be deleted.  The current design is that IPropertyChangedCallback is
        // not refcount'ed.  Should we refcount it?

        if (m_registredCount > 0)
        {
            std::vector<IPropertyChangedCallback*> local_callbacks;

            {
                std::lock_guard<std::mutex> lock(m_lock);
                local_callbacks.insert(local_callbacks.end(), m_callbacks.begin(), m_callbacks.end());
            }

            size_t count = local_callbacks.size();
            for (size_t index = 0; index < count; ++index)
            {
                IPropertyChangedCallback* cur_callback = local_callbacks[index];
                if (cur_callback != NULL)
                {
                    cur_callback->OnChanged(propertyName, propertyValue);
                }
            }
        }
    }
    catch (...)
    {

    }
}

} // PlatformAbstraction
}}}