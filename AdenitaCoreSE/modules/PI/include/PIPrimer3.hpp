#pragma once

#include "PIBindingRegion.hpp"


class SB_EXPORT PIPrimer3 {

public:

	static PIPrimer3& GetInstance();
	~PIPrimer3() = default;

	void														UpdateBindingRegions(ADNPointer<ADNPart> p);
	CollectionMap<PIBindingRegion>								GetBindingRegions() const;
	CollectionMap<PIBindingRegion>								GetBindingRegions(ADNPointer<ADNPart> p) const;
	void														Calculate(ADNPointer<ADNPart> p, int oligo_conc, int mv, int dv) const;
	void														DeleteBindingRegions(ADNPointer<ADNPart> p);

private:

	PIPrimer3() = default;

	//double													GetMinGibbsFreeEnergy();
	//double													GetMaxGibbsFreeEnergy();
	//double													GetMinMeltingTemperature();
	//double													GetMaxMeltingTemperature();

	static ThermodynamicParameters								ExecuteNtthal(std::string leftSequence, std::string rightSequence, int oligo_conc, int mv, int dv);

	std::map<ADNPart*, CollectionMap<PIBindingRegion>>			regionsMap_;

};
