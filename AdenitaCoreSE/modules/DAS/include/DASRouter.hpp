#pragma once

#include "ADNMixins.hpp"

enum class RoutingType {
	None = 0,
	Seamless = 1,
	Tiles = 2,
	NonSeamless = 3
};

class ADNPart;

class SB_EXPORT DASRouter {

public:

	DASRouter() = default;
	~DASRouter() = default;

	static DASRouter* GetRouter(RoutingType t);
	virtual void Route(SBPointer<ADNPart> part) = 0;

};

class SB_EXPORT DASRouterNone : public DASRouter {

	void Route(SBPointer<ADNPart> part) {};

};

#if 0
class SB_EXPORT DASRouterNanotubeTiles : public DASRouter {

	void Route(SBPointer<ADNPart> part);

};

class SB_EXPORT DASRouterNanotubeSeamless : public DASRouter {

	void Route(SBPointer<ADNPart> part);

};

class SB_EXPORT DASRouterNanotubeNonSeamless : public DASRouter {

	void Route(SBPointer<ADNPart> part);

};
#endif
