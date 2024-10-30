#pragma once

#include "ADNConstants.hpp"
#include "ADNVectorMath.hpp"
#include "ADNLogger.hpp"

#include "SBCHeapExport.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using LatticeType = ADNConstants::LatticeType;

namespace ublas = boost::numeric::ublas;

struct LatticeCell {
	double x_{ 0.0 };
	double y_{ 0.0 };
};

class SB_EXPORT DASLattice {

public:

	DASLattice() = default;
	DASLattice(LatticeType type, double edgeDistance, int maxRows, int maxCols);
	~DASLattice() = default;

	LatticeCell GetLatticeCell(unsigned int row, unsigned int column) const;

	size_t GetNumberRows() const;
	size_t GetNumberCols() const;

private:

	ublas::matrix<LatticeCell> mat_;

	void CreateSquareLattice(int maxRows, int maxCols);
	void CreateHoneycombLattice(int maxRows, int maxCols);

	double edgeDistance_{ 0.0 };

};
