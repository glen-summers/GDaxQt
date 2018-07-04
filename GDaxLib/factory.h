#ifndef FACTORY_H
#define FACTORY_H

#include "gdl.h"

namespace GDL
{
    class Factory : public GDL::IFactory
    {
        std::string streamUrl, requestUrl;

    public:
        Factory(const std::string & streamUrl, const std::string & requestUrl);

    private:
        GDL::StreamPtr CreateStream(GDL::IStreamCallbacks & callbacks, const Subscription & subscription, Auth * auth) const override;
        GDL::RequestPtr CreateRequest(const char * product, Auth * auth) const override;
    };
}

#endif // FACTORY_H
