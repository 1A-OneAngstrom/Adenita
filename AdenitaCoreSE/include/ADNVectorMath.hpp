#pragma once

#include <vector>

#include <cmath>

#include "SBVector3.hpp"
#include "ADNConstants.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/lu.hpp>


namespace ublas = boost::numeric::ublas;

namespace ADNVectorMath {

	//using namespace ublas;

	SB_EXPORT ublas::vector<double> CreateBoostVector(const std::vector<double>& vec);
	SB_EXPORT std::vector<double> CreateStdVector(const ublas::vector<double>& vec);
	SB_EXPORT ublas::matrix<double> CreateBoostMatrix(const std::vector<std::vector<double>>& vecovec);
	SB_EXPORT ublas::vector<double> CalculateCM(const ublas::matrix<double>& positions);
	SB_EXPORT ublas::vector<double> CalculateCM(const ublas::matrix<double>& weightedPositions, double totalMass);
	SB_EXPORT ublas::vector<double> CrossProduct(const ublas::vector<double>& v, const ublas::vector<double>& w);
	SB_EXPORT ublas::vector<double> DirectionVector(const ublas::vector<double>& p, const ublas::vector<double>& q);
	SB_EXPORT double DegToRad(double degree);
	SB_EXPORT ublas::matrix<double> MakeRotationMatrix(const ublas::vector<double>& dir, double angle);
	SB_EXPORT ublas::matrix<double> SkewMatrix(const ublas::vector<double>& v);
	SB_EXPORT ublas::vector<double> InitializeVector(size_t size);  // deprecated, use constructor
	SB_EXPORT ublas::matrix<double> InitializeMatrix(size_t sz_r, size_t sz_c);  // deprecated, use constructor
	SB_EXPORT ublas::matrix<double> InitializeMatrix(size_t sz);  // deprecated, use constructor
	SB_EXPORT ublas::matrix<double> Translate(const ublas::matrix<double>& input, const ublas::vector<double>& t_vector);
	SB_EXPORT ublas::matrix<double> Rotate(const ublas::matrix<double>& input, const ublas::matrix<double>& rot_matrix);  // deprecated, use ApplyTransformation
	SB_EXPORT ublas::matrix<double> CenterSystem(const ublas::matrix<double>& input);
	SB_EXPORT void AddRowToMatrix(ublas::matrix<double> &input, ublas::vector<double> r);
	SB_EXPORT ublas::vector<double> CalculatePlane(const ublas::matrix<double>& mat);
	SB_EXPORT ublas::matrix<double> FindOrthogonalSubspace(ublas::vector<double> z);
	SB_EXPORT ublas::matrix<double> InvertMatrix(const ublas::matrix<double>& input);
	SB_EXPORT double Determinant(ublas::matrix<double> mat);
	SB_EXPORT bool IsNearlyZero(double n, double tol = 0.000000001);
	SB_EXPORT double CalculateVectorNorm(const ublas::vector<double>& v);
	/*!
	* Applies the transformation given by t_mat to a set of points
	* \param the transformation matrix
	* \param a matrix holding coordinates of points
	* \return a matrix with the coordinates after the transformation
	*/
	SB_EXPORT ublas::matrix<double> ApplyTransformation(const ublas::matrix<double>& t_mat, const ublas::matrix<double>& points);

	SB_EXPORT ublas::vector<double> Spherical2Cartesian(const ublas::vector<double>& spher);

	// SAMSON types operations
	SB_EXPORT SBVector3 SBCrossProduct(const SBVector3& v, const SBVector3& w);
	SB_EXPORT double SBInnerProduct(const SBVector3& v, const SBVector3& w);

	//! Calculation of parameters of dna nanotubes
	SB_EXPORT SBQuantity::length CalculateNanotubeRadius(int numDs);
	SB_EXPORT int CalculateNanotubeDoubleStrands(const SBQuantity::length& radius);

	//! Bezier curves
	//! Calculates the length of a quadratic Bezier curve
	SB_EXPORT SBQuantity::length LengthQuadraticBezier(const SBPosition3& P0, const SBPosition3& P1, const SBPosition3& P2);
	SB_EXPORT SBPosition3 QuadraticBezierPoint(const SBPosition3& P0, const SBPosition3& P1, const SBPosition3& P2, double t);
	SB_EXPORT SBVector3 DerivativeQuadraticBezier(const SBPosition3& P0, const SBPosition3& P1, const SBPosition3& P2, double t);

};