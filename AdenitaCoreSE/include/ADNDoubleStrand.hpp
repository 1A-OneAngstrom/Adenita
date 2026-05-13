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

	[[nodiscard]] SBPointer<ADNPart>                           GetPart() const;                                                        ///< Returns a pointer to the part to which this double strand belongs
	
	void														SetInitialTwistAngle(double angle);
	[[nodiscard]] double										GetInitialTwistAngle() const;
	[[nodiscard]] double										getInitialTwistAngle() const;

	[[nodiscard]] int											GetLength() const;
	[[nodiscard]] int											getLength() const;

	[[nodiscard]] bool											IsCircular() const;
	[[nodiscard]] bool											getCircularFlag() const;
	void														setCircularFlag(bool b);

	[[nodiscard]] SBPointerIndexer<ADNBaseSegment>					GetBaseSegments() const;
	[[nodiscard]] SBPointer<ADNBaseSegment>					GetNthBaseSegment(int n) const;											///< Returns the base segment by position in the double strand

	[[nodiscard]] SBPointer<ADNBaseSegment>					GetFirstBaseSegment() const;
	[[nodiscard]] SBNode*										getFirstBaseSegment() const;
	void														SetStart(SBPointer<ADNBaseSegment> baseSegment);
	[[nodiscard]] SBPointer<ADNBaseSegment>					GetLastBaseSegment() const;
	[[nodiscard]] SBNode*										getLastBaseSegment() const;
	void														SetEnd(SBPointer<ADNBaseSegment> baseSegment);

	void														AddBaseSegmentBeginning(SBPointer<ADNBaseSegment> baseSegment);
	void														AddBaseSegmentEnd(SBPointer<ADNBaseSegment> baseSegment);

private:

	bool														circularFlag{ false };

	SBPointer<ADNBaseSegment>									startBaseSegment = nullptr;
	SBPointer<ADNBaseSegment>									endBaseSegment = nullptr;

	double														initialTwistAngle{ 0.0 };

};

SB_REGISTER_TARGET_TYPE(ADNDoubleStrand, "ADNDoubleStrand", "D87CDA71-C36A-EA63-9B20-24DD78A4BC4C");
SB_DECLARE_BASE_TYPE(ADNDoubleStrand, SBStructuralGroup);
