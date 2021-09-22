#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNMixins.hpp"

class ADNBaseSegment;

class ADNDoubleStrand : public SBStructuralGroup {

	SB_CLASS

public:

	ADNDoubleStrand() : SBStructuralGroup() {}
	~ADNDoubleStrand() = default;
	ADNDoubleStrand(const ADNDoubleStrand& other);

	ADNDoubleStrand& operator=(const ADNDoubleStrand& other);

	void serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const;										///< Serializes the node
	void unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0));											///< Unserializes the node

	void SetInitialTwistAngle(double angle);
	double GetInitialTwistAngle() const;
	double getInitialTwistAngle() const;

	int GetLength() const;
	int getLength() const;

	void IsCircular(bool c);
	bool IsCircular() const;
	bool getIsCircular() const;
	void setIsCircular(bool b);

	CollectionMap<ADNBaseSegment> GetBaseSegments() const;
	ADNPointer<ADNBaseSegment> GetNthBaseSegment(int n);  // return the base segment by position in the double strand

	ADNPointer<ADNBaseSegment> GetFirstBaseSegment() const;
	SBNode* getFirstBaseSegment() const;
	void SetStart(ADNPointer<ADNBaseSegment> bs);
	ADNPointer<ADNBaseSegment> GetLastBaseSegment() const;
	SBNode* getLastBaseSegment() const;
	void SetEnd(ADNPointer<ADNBaseSegment> bs);

	void AddBaseSegmentBeginning(ADNPointer<ADNBaseSegment> bs);
	void AddBaseSegmentEnd(ADNPointer<ADNBaseSegment> bs);

private:

	bool isCircular_ = false;
	ADNPointer<ADNBaseSegment> start_ = nullptr;
	ADNPointer<ADNBaseSegment> end_ = nullptr;
	double initialTwistAngle_ = 0.0;

};

SB_REGISTER_TARGET_TYPE(ADNDoubleStrand, "ADNDoubleStrand", "D87CDA71-C36A-EA63-9B20-24DD78A4BC4C");
SB_DECLARE_BASE_TYPE(ADNDoubleStrand, SBStructuralGroup);
