#pragma once

#include "SBStructuralGroup.hpp"

#include "ADNMixins.hpp"

class ADNBaseSegment;
class ADNPart;

class SB_EXPORT ADNDoubleStrand : public SBStructuralGroup {

	SB_CLASS

public:

	ADNDoubleStrand() : SBStructuralGroup() {}
	~ADNDoubleStrand() = default;
	ADNDoubleStrand(const ADNDoubleStrand& other);

	ADNDoubleStrand& operator=(const ADNDoubleStrand& other);

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	ADNPointer<ADNPart>                                         GetPart() const;                                                        ///< Returns a pointer to the part to which this double strand belongs
	
	void														SetInitialTwistAngle(double angle);
	double														GetInitialTwistAngle() const;
	double														getInitialTwistAngle() const;

	int															GetLength() const;
	int															getLength() const;

	bool														IsCircular() const;
	bool														getCircularFlag() const;
	void														setCircularFlag(bool b);

	CollectionMap<ADNBaseSegment>								GetBaseSegments() const;
	ADNPointer<ADNBaseSegment>									GetNthBaseSegment(int n) const;											///< Returns the base segment by position in the double strand

	ADNPointer<ADNBaseSegment>									GetFirstBaseSegment() const;
	SBNode*														getFirstBaseSegment() const;
	void														SetStart(ADNPointer<ADNBaseSegment> baseSegment);
	ADNPointer<ADNBaseSegment>									GetLastBaseSegment() const;
	SBNode*														getLastBaseSegment() const;
	void														SetEnd(ADNPointer<ADNBaseSegment> baseSegment);

	void														AddBaseSegmentBeginning(ADNPointer<ADNBaseSegment> baseSegment);
	void														AddBaseSegmentEnd(ADNPointer<ADNBaseSegment> baseSegment);

private:

	bool														circularFlag{ false };

	ADNPointer<ADNBaseSegment>									startBaseSegment = nullptr;
	ADNPointer<ADNBaseSegment>									endBaseSegment = nullptr;

	double														initialTwistAngle{ 0.0 };

};

SB_REGISTER_TARGET_TYPE(ADNDoubleStrand, "ADNDoubleStrand", "D87CDA71-C36A-EA63-9B20-24DD78A4BC4C");
SB_DECLARE_BASE_TYPE(ADNDoubleStrand, SBStructuralGroup);
