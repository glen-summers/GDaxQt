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
        GDL::StreamPtr CreateStream(GDL::IStreamCallbacks & callbacks) const override;
        GDL::RequestPtr CreateRequest() const override;
    };
}

#endif // FACTORY_H
