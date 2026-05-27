#pragma once

#ifndef BACKTOTHEATOM_H
#define BACKTOTHEATOM_H

#include <vector>
#include <string>
#include <map>
#include <stdlib.h>

#include "ADNNanorobot.hpp"
#include "ADNVectorMath.hpp"
#include "ADNBasicOperations.hpp"
#include "DASPolyhedron.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

typedef std::map<int, std::vector<double>> BasePositions;
typedef std::map<DASEdge*, BasePositions> SequencePositions;
typedef std::pair<SBPointer<ADNNucleotide>, SBPointer<ADNNucleotide>> NtPair;

class SB_EXPORT DASBackToTheAtom {

public:

    DASBackToTheAtom();
    ~DASBackToTheAtom();

    /**
     * Sets the nucleotides backbone, sidechain and center of mass positions for an entire
     * double strand using the BaseSegment position information.
     * \param The double strand whose nucleotides need to be set.
     */
    void SetDoubleStrandPositions(SBPointer<ADNDoubleStrand> ds);
    void SetNucleotidesPositions(SBPointer<ADNPart> part);

    static void SetPositionLoopNucleotides(SBPointer<ADNBaseSegment> bs);

    void GenerateAllAtomModel(SBPointer<ADNPart> origami, bool createFlag = false);

    void CheckDistances(SBPointer<ADNPart> part) const;

    void PopulateWithMockAtoms(SBPointer<ADNPart> origami, bool positionsFromNucleotide = false, bool createAtoms = false);

    //! Set the backbone and sidechain position of the nucleotides of a base segment
    /*!
      \param The nucleotide we want to untwist
      \param Whether to also untwist the pair
    */
    void SetNucleotidePosition(SBPointer<ADNBaseSegment> bs, bool set_pair = false);
    //! Untwist the nucleotides of a base segment (remove the helix turn)
    /*!
      \param The base segment
      \param Whether to also untwist the pair
    */
    void UntwistNucleotidesPosition(SBPointer<ADNBaseSegment> bs);

    //! Sets the positions of a collection of nucleotides, meant to be called after modifications
    void SetPositionsForNewNucleotides(SBPointer<ADNPart> part, SBPointerIndexer<ADNNucleotide> nts);

    //void SetAllAtomsPositions(SBPointer<ADNPart> origami);
    // for cadnano
    /*void SetAllAtomsPositions2D(SBPointer<ADNPart> origami);
    void SetAllAtomsPositions1D(SBPointer<ADNPart> origami);*/

    /**
     * Rotates a nucleotide along base-pair plane (defined by nt->e3_).
     * \param the nucleotide you want to rotate
     * \param the amount of degrees
     * \param wether to also rotate the pair
     */
     //void RotateNucleotide(SBPointer<ADNNucleotide> nt, double angle, bool set_pair);

     /**
      * Loads a nucleotide or base pair into SAMSON for testing purposes
      */
      //void DisplayDNABlock(std::string block);

private:

    SBPointer<ADNNucleotide> da_;
    SBPointer<ADNNucleotide> dt_;
    SBPointer<ADNNucleotide> dg_;
    SBPointer<ADNNucleotide> dc_;
    NtPair da_dt_;
    NtPair dt_da_;
    NtPair dc_dg_;
    NtPair dg_dc_;
    /** Loads the four types of nucleotide as members
    */
    void LoadNucleotides();
    /** Loads the four base pairs as members
    */
    void LoadNtPairs();

    /** Parses a nucleotide PDB file.
     *  \param a string with the location of the PDB.
     *  \return a nucleotide object.
     */
    static SBPointer<ADNNucleotide> ParsePDB(const std::string& source);

    /** Parses a base pair PDB file.
     *  \param a string with the location of the PDB.
     *  \return a pair of nucleotide objects.
     */
    static NtPair ParseBasePairPDB(const std::string& source);

    static void PositionLoopNucleotides(SBPointer<ADNLoop> loop, SBPosition3 bsPositionPrev, SBPosition3 bsPositionNext);
    static void PositionLoopNucleotidesQBezier(SBPointer<ADNLoop> loop, SBPosition3 bsPositionPrev, SBPosition3 bsPositionNext, SBVector3 bsPrevE3, SBVector3 bsNextE3);

    void PopulateNucleotideWithAllAtoms(SBPointer<ADNPart> origami, SBPointer<ADNNucleotide> nt, bool createFlag = false);
    void PrepareFramesForAtomicModel(SBPointer<ADNPart> origami);
#ifndef NDEBUG
    bool ValidateGeneratedBasePairPlanes(SBPointer<ADNPart> part) const;
#endif
    static void CreateBonds(SBPointer<ADNPart> origami, bool createFlag = false);
    void FindAtomsPositions(SBPointer<ADNNucleotide> nt);

    //! Untwist the nucleotide (remove the helix turn)
    /*!
    \param The nucleotide we want to untwist
    \param Whether to also untwist the pair
    */
    void UntwistNucleotidePosition(SBPointer<ADNNucleotide> nt);

    //! Create positions matrix from a NtPair, every row in the matrix is the position of one atom
    static ublas::matrix<double> CreatePositionsMatrix(NtPair pair);

    //! Select the ideal NtPair corresponding to a pair
    NtPair GetIdealBasePairNucleotides(SBPointer<ADNNucleotide> nt_l, SBPointer<ADNNucleotide> nt_r) const;
    NtPair GetIdealBasePairNucleotides(DNABlocks nt_l, DNABlocks nt_r) const;

    //! If not defined, set a local basis for the base segment and return it as a matrix
    static ublas::matrix<double> CalculateBaseSegmentBasis(SBPointer<ADNBaseSegment> bs);
    /*!
     * Calculate the center of mass of backbone and sidechain w.r.t. atom coordinates
     *  \param the nucleotide
     *  \return tuple with center values. first is total c.o.m., second is backbone, third is sidechain.
     */
    static std::tuple<SBPosition3, SBPosition3, SBPosition3> CalculateCenters(SBPointer<ADNNucleotide> nt);
    static std::tuple<SBPosition3, SBPosition3, SBPosition3> CalculateCentersOfMass(SBPointer<ADNNucleotide> nt);
    /*!
     * Generate a SBPosition from a ublas vector assuming picometers -> angstrom conversion
     */
    static SBPosition3 UblasToSBPosition(ublas::vector<double> vec);
    /*!
     * Sets the reference frame for the pair to the standard basis
     */
    static void SetReferenceFrame(NtPair pair);
    /** Sets the positions of a list of atoms according to a matrix positions
    */
    static int SetAtomsPositions(SBPointerIndexer<ADNAtom> atoms, ublas::matrix<double> new_positions, int r_id);

};

#endif
