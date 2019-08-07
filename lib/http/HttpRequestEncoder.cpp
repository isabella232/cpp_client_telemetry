// Copyright (c) Microsoft. All rights reserved.

#include "HttpRequestEncoder.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"

#include <memory>
#include <string>

#include "utils/Utils.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include "bond/generated/CsProtocol_readers.hpp"

namespace ARIASDK_NS_BEGIN {


    HttpRequestEncoder::HttpRequestEncoder(ITelemetrySystem& system, IHttpClient& httpClient)
        :
        m_system(system),
        m_httpClient(httpClient),
        m_config(system.getConfig())
    {
    }

    HttpRequestEncoder::~HttpRequestEncoder()
    {
    }

    bool HttpRequestEncoder::DispatchDataViewerEvent(StorageBlob dataPacket)
    {
        return m_system.getLogManager().DispatchDataViewerEvent(dataPacket);
    }

    bool HttpRequestEncoder::handleEncode(EventsUploadContextPtr const& ctx)
    {
        ctx->httpRequest = m_httpClient.CreateRequest();
        ctx->httpRequestId = ctx->httpRequest->GetId();

        ctx->httpRequest->SetMethod("POST");

        ctx->httpRequest->SetUrl(m_config.GetCollectorUrl());

        ctx->httpRequest->GetHeaders().set("Expect", "100-continue");
        ctx->httpRequest->GetHeaders().set("SDK-Version", PAL::getSdkVersion());
        ctx->httpRequest->GetHeaders().set("Client-Id", "NO_AUTH");
        ctx->httpRequest->GetHeaders().set("Content-Type", "application/bond-compact-binary");
        ctx->httpRequest->GetHeaders().set("Upload-Time", toString(PAL::getUtcSystemTimeMs()));


        if (GetAuthTokensController()->GetDeviceTokens().size() > 0)
        {
            std::map<TicketType, std::string>& map = GetAuthTokensController()->GetDeviceTokens();
            if (map.end() != map.find(TicketType::TicketType_MSA_Device))
            {
                ctx->httpRequest->GetHeaders().set("AuthMsaDeviceTicket", map[TicketType::TicketType_MSA_Device]);
            }

            if (map.end() != map.find(TicketType::TicketType_XAuth_Device))
            {
                ctx->httpRequest->GetHeaders().set("AuthXToken", map[TicketType::TicketType_XAuth_Device]);
            }

            if (map.end() != map.find(TicketType::TicketType_AAD))
            {
                ctx->httpRequest->GetHeaders().set("Aad-Token", map[TicketType::TicketType_AAD]);
            }

            if (map.end() != map.find(TicketType::TicketType_AAD_JWT))
            {
                ctx->httpRequest->GetHeaders().set("Aad-Jwt-Token", map[TicketType::TicketType_AAD_JWT]);
            }
        }

        if (GetAuthTokensController()->GetUserTokens().size() > 0)
        {  //create Ticket header
            std::map<TicketType, std::string>& map = GetAuthTokensController()->GetUserTokens();

            std::string ticketHeader;
            // We know that each ticket is about 1kb in size, so pre-reserve space for the appends
            ticketHeader.reserve(GetAuthTokensController()->GetUserTokens().size() * 1024);//

            if (map.end() != map.find(TicketType::TicketType_MSA_User))
            {
                ticketHeader.append("\"");
                ticketHeader.append(TICKETS_PREPEND_STRING + std::to_string(TicketType::TicketType_MSA_User));
                ticketHeader.append("\"=\"");
                ticketHeader.append("p:");
                ticketHeader.append(map[TicketType::TicketType_MSA_User]);
                ticketHeader.append("\"");
            }
            if (map.end() != map.find(TicketType::TicketType_XAuth_User))
            {
                if (!ticketHeader.empty())
                {
                    ticketHeader.append(";");
                }
                ticketHeader.append("\"");
                ticketHeader.append(TICKETS_PREPEND_STRING + std::to_string(TicketType::TicketType_XAuth_User));
                ticketHeader.append("\"=\"");
                ticketHeader.append("x:XBL3.0 x=");
                ticketHeader.append(map[TicketType::TicketType_XAuth_User]);
                ticketHeader.append("\"");
            }
            if (map.end() != map.find(TicketType::TicketType_AAD_User))
            {
                if (!ticketHeader.empty())
                {
                    ticketHeader.append(";");
                }
                ticketHeader.append("\"");
                ticketHeader.append(TICKETS_PREPEND_STRING + std::to_string(TicketType::TicketType_AAD_User));
                ticketHeader.append("\"=\"");
                ticketHeader.append("at:");
                ticketHeader.append(map[TicketType::TicketType_AAD_User]);
                ticketHeader.append("\"");
            }

            if (!ticketHeader.empty())
            {
                ctx->httpRequest->GetHeaders().set("Tickets", ticketHeader);
            }
        }
        //strict mode
        if (true == GetAuthTokensController()->GetStrictMode())
        {
            ctx->httpRequest->GetHeaders().set("Strict", "true");
        }

        std::string tenantTokens;
        tenantTokens.reserve(ctx->packageIds.size() * 75); // Tenants tokens are usually 74 chars long.
        for (auto const& item : ctx->packageIds) {
            if (!tenantTokens.empty()) {
                tenantTokens.push_back(',');
            }
            tenantTokens.append(item.first);
        }
        ctx->httpRequest->GetHeaders().set("APIKey", tenantTokens);

        if (ctx->compressed) {
            ctx->httpRequest->GetHeaders().add("Content-Encoding", "deflate");
        }


#if 0
        // XXX: [MG] - debug only
        CsProtocol::Record result;
        bond_lite::CompactBinaryProtocolReader reader(ctx->body);
        bond_lite::Deserialize(reader, result);
#endif

        ctx->httpRequest->SetBody(ctx->body);
        // IHttpRequest::SetBody() is free to swap the real body out, but better clear it anyway.
        ctx->body.clear();

        ctx->httpRequest->SetLatency(ctx->latency);

        DispatchDataViewerEvent(ctx->body);

        return true;
    }

} ARIASDK_NS_END
