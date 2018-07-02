#include "factory.h"

#include "websocketstream.h"
#include "restprovider.h"

GDL::Factory::Factory(const std::string & streamUrl, const std::string & requestUrl)
    : streamUrl(streamUrl)
    , requestUrl(requestUrl)
{
}

GDL::StreamPtr GDL::Factory::CreateStream(GDL::IStreamCallbacks & callbacks, const Subscription & subscription) const
{
    return GDL::StreamPtr(Utils::QMake<WebSocketStream>("webSocketStream", streamUrl.c_str(), subscription, callbacks));
}

GDL::RequestPtr GDL::Factory::CreateRequest(const char * product) const
{
    return GDL::RequestPtr(Utils::QMake<RestProvider>("RestProvider", requestUrl.c_str(), product));
}
