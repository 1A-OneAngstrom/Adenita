#pragma once

#include "SBProxy.hpp"
#include "PIBindingRegion.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(PIBindingRegion);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("PIBindingRegion");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_ONLY(unsigned int, Size, "Number of nodes", "Properties");
    SB_ATTRIBUTE_READ_ONLY(double, Entropy, "Entropy (cal/mol)", "Thermodynamic Parameters");
    SB_ATTRIBUTE_READ_ONLY(double, Enthalpy, "Enthalpy (cal/mol)", "Thermodynamic Parameters");
    SB_ATTRIBUTE_READ_ONLY(double, Gibbs, "Gibbs Free Energy (cal/mol)", "Thermodynamic Parameters");
    SB_ATTRIBUTE_READ_ONLY(double, Temp, "Melting Temperature (C)", "Thermodynamic Parameters");
    SB_ATTRIBUTE_READ_ONLY(std::string, StatusString, "Status", "Thermodynamic Parameters");

  SB_INTERFACE_END;

SB_CLASS_END(PIBindingRegion);