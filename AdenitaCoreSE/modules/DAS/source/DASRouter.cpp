#include "DASRouter.hpp"

#include "ADNPart.hpp"

DASRouter* DASRouter::GetRouter(RoutingType t) {

    DASRouter* router = nullptr;

    if (t == RoutingType::None) {
        router = new DASRouterNone();
    }

    return router;

}
