#pragma once

#include "ADNConfig.hpp"
#include "ADNArray.hpp"
#include "ADNMixins.hpp"

#include "SBNodeGroup.hpp"
#include "SBFolder.hpp"


class ADNNucleotide;
class ADNPart;

struct ThermodynamicParameters {

	double dS_{ 0.0 };
	double dH_{ 0.0 };
	double T_{ 0.0 };
	double dG_{ 0.0 };
	bool isValid{ false };

};

class SB_EXPORT PIBindingRegion : public SBNodeGroup {

	SB_CLASS

public:

	PIBindingRegion() : SBNodeGroup() {};
	PIBindingRegion(const std::string& name, const SBNodeIndexer& indexer) : SBNodeGroup(name, indexer) {};
	PIBindingRegion(const PIBindingRegion& other) : SBNodeGroup(other) {};
	~PIBindingRegion() = default;

	PIBindingRegion& operator=(const PIBindingRegion& other);

	double getEntropy() const;
	double getEnthalpy() const;
	double getGibbs() const;
	double getTemp() const;
	std::string getStatusString() const;

	void SetLastNt(ADNPointer<ADNNucleotide> nt);
	void SetFirstNt(ADNPointer<ADNNucleotide> nt);
	ADNPointer<ADNPart> GetPart();
	void SetPart(ADNPointer<ADNPart> part);
	void SetThermodynamicParameters(ThermodynamicParameters res);

	void RegisterBindingRegion(SBFolder* folder = nullptr);
	void UnregisterBindingRegion();
	std::pair<std::string, std::string> GetSequences();

private:

	ThermodynamicParameters thermodynamicParameters_;
	ADNPointer<ADNNucleotide> firstNt_;
	ADNPointer<ADNNucleotide> lastNt_;
	ADNPointer<ADNPart> part_;

};

SB_REGISTER_TARGET_TYPE(PIBindingRegion, "PIBindingRegion", "171D8E12-FCB6-4770-A026-1AADC94BE325");
SB_DECLARE_BASE_TYPE(PIBindingRegion, SBNodeGroup);
