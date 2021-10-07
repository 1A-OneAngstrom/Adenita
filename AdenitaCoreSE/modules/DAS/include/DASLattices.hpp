#pragma once

#include "ADNConstants.hpp"
#include "ADNVectorMath.hpp"
#include "ADNLogger.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using LatticeType = ADNConstants::LatticeType;

namespace ublas = boost::numeric::ublas;

struct LatticeCell {
	double x_;
	double y_;
};

class SB_EXPORT DASLattice {

public:

	DASLattice() = default;
	DASLattice(LatticeType type, double edgeDistance, int maxRows, int maxCols);
	~DASLattice() = default;

	LatticeCell GetLatticeCell(unsigned int row, unsigned int column);

	size_t GetNumberRows();
	size_t GetNumberCols();

private:

	ublas::matrix<LatticeCell> mat_;

	void CreateSquareLattice(int maxRows, int maxCols);
	void CreateHoneycombLattice(int maxRows, int maxCols);

	double edgeDistance_;

};
