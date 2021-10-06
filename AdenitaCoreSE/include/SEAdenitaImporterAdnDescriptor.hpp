/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaImporterAdn.hpp "SEAdenitaImporterAdn.hpp"
#include "SEAdenitaImporterAdn.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaImporterAdn);

	SB_CLASS_TYPE(SBCClass::Importer);
	SB_CLASS_DESCRIPTION("Adenita importer (.adn)");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaImporterAdn);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaImporterAdn);

