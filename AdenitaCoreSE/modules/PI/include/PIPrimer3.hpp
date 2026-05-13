#pragma once

#include "PIBindingRegion.hpp"


class SB_EXPORT PIPrimer3 {

public:

	static PIPrimer3& GetInstance();
	~PIPrimer3() = default;

	void														UpdateBindingRegions(SBPointer<ADNPart> p);
	SBPointerIndexer<PIBindingRegion>								GetBindingRegions() const;
	SBPointerIndexer<PIBindingRegion>								GetBindingRegions(SBPointer<ADNPart> p) const;
	void														Calculate(SBPointer<ADNPart> p, int oligo_conc, int mv, int dv) const;
	void														DeleteBindingRegions(SBPointer<ADNPart> p);

private:

	PIPrimer3() = default;

	//double													GetMinGibbsFreeEnergy();
	//double													GetMaxGibbsFreeEnergy();
	//double													GetMinMeltingTemperature();
	//double													GetMaxMeltingTemperature();

	static ThermodynamicParameters								ExecuteNtthal(std::string leftSequence, std::string rightSequence, int oligo_conc, int mv, int dv);

	std::map<ADNPart*, SBPointerIndexer<PIBindingRegion>>			regionsMap_;

};
