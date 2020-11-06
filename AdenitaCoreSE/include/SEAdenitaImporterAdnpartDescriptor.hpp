/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaImporterAdnpart.hpp "SEAdenitaImporterAdnpart.hpp"
#include "SEAdenitaImporterAdnpart.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaImporterAdnpart);

	SB_CLASS_TYPE(SBCClass::Importer);
	SB_CLASS_DESCRIPTION("Adenita importer for Adenita parts format (.adnpart)");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaImporterAdnpart);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaImporterAdnpart);

