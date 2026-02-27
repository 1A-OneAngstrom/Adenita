#pragma once

#include "SBProxy.hpp"
#include "ADNModel.hpp"
#include "ADNAtom.hpp"
#include "ADNBackbone.hpp"
#include "ADNSidechain.hpp"
#include "ADNNucleotide.hpp"
#include "ADNSingleStrand.hpp"
#include "ADNDoubleStrand.hpp"
#include "ADNLoop.hpp"
#include "ADNBaseSegment.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(ADNAtom);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNAtom");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_ONLY(SBNode*, Nucleotide, "Nucleotide", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBPosition3 const&, Position, "Position", "Adenita");

	SB_ATTRIBUTE_READ_WRITE(SBElement::Type, ElementType, "Element", "Element");
	SB_ATTRIBUTE_READ_ONLY(std::string, ElementName, "Element name", "Element");
	SB_ATTRIBUTE_READ_ONLY(std::string, ElementSymbol, "Element symbol", "Element");
	SB_ATTRIBUTE_READ_ONLY(std::string, Period, "Period", "Element");
	SB_ATTRIBUTE_READ_ONLY(unsigned int, Group, "Group", "Element");
    SB_ATTRIBUTE_READ_ONLY(std::string, Block, "Block", "Element");
	SB_ATTRIBUTE_READ_ONLY(SBQuantity::mass, AtomicWeight, "Atomic weight", "Element");
    SB_ATTRIBUTE_READ_WRITE(SBAtom::Hybridization const&, Hybridization, "Hybridization", "Element");
    SB_ATTRIBUTE_READ_WRITE(int const&, OxidationState, "Oxydation state", "Element");
    SB_ATTRIBUTE_READ_WRITE(bool const&, WaterFlag, "Water flag", "Element");
	SB_ATTRIBUTE_READ_ONLY(std::string, MetalSubcategoryString, "Subcategory", "Element");

	SB_ATTRIBUTE_READ_WRITE(bool const&, Resonance, "Resonance", "Chemistry");
	SB_ATTRIBUTE_READ_WRITE(bool const&, Aromaticity, "Aromaticity", "Chemistry");
	SB_ATTRIBUTE_READ_ONLY(SBQuantity::dimensionless, Electronegativity, "Electronegativity", "Chemistry");
	
	SB_ATTRIBUTE_READ_ONLY(SBQuantity::length, CovalentRadius, "Covalent radius", "Structure");
	SB_ATTRIBUTE_READ_ONLY(SBQuantity::length, VanDerWaalsRadius, "Van der Waals radius", "Structure");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(SBAtom::Geometry const&, Geometry, "Geometry", "Structure");
	SB_ATTRIBUTE_READ_WRITE(SBPosition3 const&, Position, "Position", "Structure");
	SB_ATTRIBUTE_READ_WRITE(bool, MobilityFlag, "Mobile", "Structure");

	SB_ATTRIBUTE_READ_WRITE_CLEAR(const std::string&, Name, "Name", "Identity");
	SB_ATTRIBUTE_READ_ONLY(std::string, MoleculeName, "Molecule name", "Identity");
	
	SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
	SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");
	SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection flag", "Node");
	SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility flag", "Node");
	SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
	SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
	//SB_ATTRIBUTE_READ_ONLY(SBNode*, ADNAtom, NextNode, "Next", "Node");
	//SB_ATTRIBUTE_READ_ONLY(SBNode*, ADNAtom, PreviousNode, "Previous", "Node");

    SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, Comment, "Comment", "Other");
    SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, StatusBit, "Status bit", "Other");

	SB_ATTRIBUTE_READ_WRITE_CLEAR(const char&, AltLocation, "Alt. location", "Protein");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(const char&, InsertionCode, "Insertion code", "Protein");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(const int&, SerialNumber, "Serial number", "Protein");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(const SBQuantity::dimensionless&, Occupancy, "Occupancy", "Protein");

	SB_ATTRIBUTE_READ_WRITE_CLEAR(double const&, TemperatureFactor, "Temperature factor", "Protein");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(float const&, PartialCharge, "Partial charge", "Protein");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(int const&, FormalCharge, "Formal charge", "Protein");

	SB_ATTRIBUTE_READ_ONLY(std::string, SubstructureName, "Substructure name", "Protein");
	SB_ATTRIBUTE_READ_ONLY(std::string, SubstructureSequenceNumberString, "Substructure sequence number", "Protein");

	SB_ATTRIBUTE_READ_ONLY(std::string, ChainIDString, "Chain ID", "Protein");
	SB_CONST_FUNCTION_0(int, getChainID);
	SB_ATTRIBUTE_READ_ONLY(std::string, ChainName, "Chain name", "Protein");

	SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, RecordType, "Record type", "Protein");

	SB_ATTRIBUTE_READ_ONLY(std::string, ResidueName, "Residue name", "Protein");
	SB_ATTRIBUTE_READ_ONLY(std::string, ResidueTypeString, "Residue type", "Protein");
	SB_ATTRIBUTE_READ_ONLY(std::string, ResidueSequenceNumberString, "Residue sequence number", "Protein");
	SB_CONST_FUNCTION_0(int, getResidueSequenceNumber);

	SB_ATTRIBUTE_READ_ONLY(std::string, SegmentName, "Segment name", "Protein");

	SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, Comment, "Comment", "Other");
	SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, StatusBit, "Status bit", "Other");

	SB_ATTRIBUTE_READ_WRITE_CLEAR_ARRAY(char* const&, SYBYLType, "SYBYL type", "Typization");
	SB_ATTRIBUTE_READ_WRITE_CLEAR(int const&, CustomType, "Custom type", "Typization");
	
  SB_INTERFACE_END;

SB_CLASS_END(ADNAtom);


SB_CLASS_BEGIN(ADNBackbone);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNBackbone");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_ONLY(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_ONLY(int, NumberOfAtoms, "Number of atoms", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNBackbone);


SB_CLASS_BEGIN(ADNSidechain);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNSidechain");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_ONLY(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_ONLY(int, NumberOfAtoms, "Number of atoms", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNSidechain);


SB_CLASS_BEGIN(ADNNucleotide);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNNucleotide");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");
    SB_ATTRIBUTE_READ_WRITE_CLEAR(const int&, StructuralID, "Residue sequence number", "Identity");
    SB_ATTRIBUTE_READ_ONLY(DNABlocks, NucleotideType, "Residue type", "Identity");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, SingleStrand, "Single strand", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, BaseSegment, "Base segment", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Next, "Next nucleotide", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Prev, "Previous nucleotide", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Pair, "Pair nucleotide", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(std::string, BaseSegmentTypeString, "Base segment type", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(std::string, EndTypeString, "End type", "Adenita");
    SB_ATTRIBUTE_READ_WRITE(std::string, Tag, "Tag", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNNucleotide);


SB_CLASS_BEGIN(ADNSingleStrand);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNSingleStrand");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_WRITE(bool, ScaffoldFlag, "Is scaffold", "Adenita");
    SB_ATTRIBUTE_READ_WRITE(bool, CircularFlag, "Is circular", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(int, NumberOfNucleotides, "Nucleotides", "Adenita");
    SB_ATTRIBUTE_READ_WRITE(std::string, Sequence, "Sequence", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(double, GCContent, "GC %", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, FivePrime, "5'", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThreePrime, "3'", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNSingleStrand);


SB_CLASS_BEGIN(ADNCell);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNCell");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

  SB_INTERFACE_END;

SB_CLASS_END(ADNCell);


SB_CLASS_BEGIN(ADNBasePair);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNBasePair");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");
    SB_ATTRIBUTE_READ_WRITE_CLEAR(const int&, StructuralID, "Structural group ID", "Identity");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, LeftNucleotide, "Left", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, RightNucleotide, "Right", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNBasePair);


SB_CLASS_BEGIN(ADNSkipPair);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNSkipPair");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

  SB_INTERFACE_END;

SB_CLASS_END(ADNSkipPair);


SB_CLASS_BEGIN(ADNLoopPair);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNLoopPair");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, LeftLoop, "Left", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, RightLoop, "Right", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNLoopPair);


SB_CLASS_BEGIN(ADNLoop);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNLoop");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_ONLY(int, NumberOfNucleotides, "Number of nucleotides", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(std::string, LoopSequence, "Sequence", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, StartNucleotide, "Start nucleotide", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, EndNucleotide, "End nucleotide", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNLoop);


SB_CLASS_BEGIN(ADNBaseSegment);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNBaseSegment");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const&, Name, "Name", "Identity");
    SB_ATTRIBUTE_READ_WRITE_CLEAR(const int&, StructuralID, "Structural group ID", "Identity");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, DoubleStrand, "Double strand", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(int, Number, "Number", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(std::string, CellTypeString, "Contains", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNBaseSegment);


SB_CLASS_BEGIN(ADNDoubleStrand);

  SB_CLASS_TYPE(SBCClass::Custom);
  SB_CLASS_DESCRIPTION("ADNDoubleStrand");

  SB_FACTORY_BEGIN;

    SB_CONSTRUCTOR_0();

  SB_FACTORY_END;

  SB_INTERFACE_BEGIN;

    SB_ATTRIBUTE_READ_WRITE(std::string const &, Name, "Name", "Identity");

    SB_ATTRIBUTE_READ_ONLY(int, Length, "Length", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(double, InitialTwistAngle, "Helical twist offset", "Adenita");
    SB_ATTRIBUTE_READ_WRITE(bool, CircularFlag, "Is circular", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, FirstBaseSegment, "First base segment", "Adenita");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, LastBaseSegment, "Last base segment", "Adenita");

    SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
    SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection Flag", "Node");
    SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility Flag", "Node");
    SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");

  SB_INTERFACE_END;

SB_CLASS_END(ADNDoubleStrand);
