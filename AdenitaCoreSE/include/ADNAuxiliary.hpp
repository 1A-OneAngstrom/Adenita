#pragma once

#include <string>

#include <QString>
#include <QTextStream>
#include <QFile>

#include <SAMSON.hpp>

#include <ADNConfig.hpp>

#undef foreach
#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>


namespace ublas = boost::numeric::ublas;

using Position3D = SBPosition3;

namespace ADNAuxiliary {

    //using namespace ublas;

    //enum SelectionFlag {
    //    Unselected = 0,
    //    Selected = 1
    //};

    //enum VisibilityFlag {
    //    Invisible = 0,
    //    Visible = 1
    //};

    enum class SortingType {
        StrandID = 0,
        Length = 1,
        GCCont = 2,
        MeltTemp = 3,
        Gibbs = 4
    };

    enum class OverlayType{
        NoOverlay = 0,
        Crossovers = 1,
        PossibleCrossovers = 2
    };

    enum class ScaffoldSeq {
        m13mp18 = 0,
        p7249 = 1,
        Custom = 2,
    };


    struct SphericalCoord {
        double phi;
        double theta;
        double r;
    };

    enum class OxDNAInteractionType {
        DNA = 0,
        DNA2 = 1,
        Patchy = 2,
        LJ = 3,
    };

    enum class OxDNASimulationType {
        MD = 0,
        MC = 1,
        VMMC = 2,
    };

    enum class OxDNABackend {
        CPU = 0,
        CUDA = 1,
    };

    struct OxDNAOptions {
        OxDNAInteractionType interactionType_;
        OxDNASimulationType simType_;
        OxDNABackend backend_;
        bool debug_;
        unsigned int steps_;
        std::string temp_;
        double saltConcentration_ = 0.0;
        double mdTimeStep_ = 0.0;
        double mcDeltaTranslation_ = 0.0;
        double mcDeltaRotation_ = 0.0;
        double boxSizeX_ = 0.0;  // in nm
        double boxSizeY_ = 0.0;
        double boxSizeZ_ = 0.0;
    };

    SB_EXPORT double mapRange(double input, double input_start, double input_end, double output_start, double output_end);
    //SB_EXPORT char getBaseSymbol(SBNode* node);
    SB_EXPORT char getBaseSymbol(std::string name);
    SB_EXPORT char getBaseSymbol(size_t numAtoms); 
    SB_EXPORT void getHeatMapColor(double val, double min, double max, int* color);

    SB_EXPORT std::string UblasVectorToString(ublas::vector<double> vec);
    SB_EXPORT std::string VectorToString(std::vector<int> vec);
    SB_EXPORT std::string SBPositionToString(Position3D pos);
    SB_EXPORT std::string SBVector3ToString(SBVector3 vec);

    SB_EXPORT ublas::vector<double> StringToUblasVector(std::string vec);
    SB_EXPORT Position3D StringToSBPosition(std::string vec);
    SB_EXPORT SBVector3 StringToSBVector(std::string vec);
    SB_EXPORT std::vector<int> StringToVector(std::string vec);

    SB_EXPORT Position3D UblasVectorToSBPosition(ublas::vector<double> pos);
    SB_EXPORT SBVector3 UblasVectorToSBVector(ublas::vector<double> pos);
    SB_EXPORT ublas::vector<double> SBVectorToUblasVector(SBVector3 v);
    SB_EXPORT ublas::vector<double> SBPositionToUblas(Position3D pos);
    SB_EXPORT std::vector<double> SBPositionToVector(Position3D pos);

    /*! Converts a 3D std::vector<double> to SBPosition3
        (in nanometers)
    */
    SB_EXPORT SBPosition3 VectorToSBPosition(std::vector<double> v);
    /*! Converts a 3D std::vector<double> to SBVector3
    */
    SB_EXPORT SBVector3 VectorToSBVector(std::vector<double> v);
  
    SB_EXPORT bool ValidateSequence(std::string seq);

    SB_EXPORT QString AdenitaCitation();

};
