#pragma once

#include "SBCHeapExport.hpp"

#include <cmath>

/*! \file ADNFrameUtils.hpp */

namespace ADNFrameUtils {

/// \brief Three-dimensional vector used by the frame utility layer.
struct Vec3 {
	double x{}; ///< X coordinate.
	double y{}; ///< Y coordinate.
	double z{}; ///< Z coordinate.
};

/// \brief Dense 3x3 rotation matrix.
struct Mat3 {
	double m[3][3]{}; ///< Matrix coefficients in row-major order.
};

/// \brief Orthonormal right-handed frame represented by three basis vectors.
struct Frame {
	Vec3 e1{}; ///< First basis vector.
	Vec3 e2{}; ///< Second basis vector.
	Vec3 e3{}; ///< Third basis vector.
};

/// \name Vector arithmetic
//@{
/// \brief Add two vectors component-wise.
SB_EXPORT [[nodiscard]] Vec3 operator+(const Vec3& a, const Vec3& b) noexcept;
/// \brief Subtract two vectors component-wise.
SB_EXPORT [[nodiscard]] Vec3 operator-(const Vec3& a, const Vec3& b) noexcept;
/// \brief Negate a vector.
SB_EXPORT [[nodiscard]] Vec3 operator-(const Vec3& v) noexcept;
/// \brief Multiply a vector by a scalar.
SB_EXPORT [[nodiscard]] Vec3 operator*(const Vec3& v, double scalar) noexcept;
/// \brief Multiply a vector by a scalar.
SB_EXPORT [[nodiscard]] Vec3 operator*(double scalar, const Vec3& v) noexcept;
/// \brief Divide a vector by a scalar.
SB_EXPORT [[nodiscard]] Vec3 operator/(const Vec3& v, double scalar) noexcept;
//@}

/// \brief Return the canonical Cartesian frame.
SB_EXPORT [[nodiscard]] Frame identityFrame() noexcept;

/// \name Vector algebra
//@{
/// \brief Return the dot product of two vectors.
SB_EXPORT [[nodiscard]] double dot(const Vec3& a, const Vec3& b) noexcept;
/// \brief Return the cross product of two vectors.
SB_EXPORT [[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) noexcept;
/// \brief Return the squared Euclidean norm of a vector.
SB_EXPORT [[nodiscard]] double squaredNorm(const Vec3& v) noexcept;
/// \brief Return the Euclidean norm of a vector.
SB_EXPORT [[nodiscard]] double norm(const Vec3& v) noexcept;
/// \brief Return a normalized copy of the vector when its norm is large enough.
SB_EXPORT [[nodiscard]] Vec3 normalized(const Vec3& v, double eps = 1.0e-12) noexcept;
/// \brief Test whether a vector norm is below the given tolerance.
SB_EXPORT [[nodiscard]] bool isNearlyZero(const Vec3& v, double eps = 1.0e-12) noexcept;
/// \brief Rotate a vector with the given rotation matrix.
SB_EXPORT [[nodiscard]] Vec3 rotated(const Mat3& r, const Vec3& v) noexcept;
//@}

/// \brief Return the determinant of the frame basis matrix.
SB_EXPORT [[nodiscard]] double determinant(const Frame& f) noexcept;

/// \name Frame validation and repair
//@{
/// \brief Test whether every coordinate of the vector is finite.
SB_EXPORT [[nodiscard]] bool isFinite(const Vec3& v) noexcept;
/// \brief Test whether a vector is unit-length within the given tolerance.
SB_EXPORT [[nodiscard]] bool isNearlyUnit(const Vec3& v, double eps = 1.0e-6) noexcept;
/// \brief Test whether a frame is orthonormal and right-handed.
SB_EXPORT [[nodiscard]] bool isOrthonormalRightHanded(const Frame& f, double eps = 1.0e-6) noexcept;

/// \brief Rotate every basis vector of a frame with the given matrix.
SB_EXPORT [[nodiscard]] Frame rotated(const Mat3& r, const Frame& f) noexcept;
/// \brief Return a repaired orthonormal right-handed copy of a frame.
///
/// The fallback frame is used when the input does not contain enough valid
/// information to reconstruct a stable basis.
SB_EXPORT [[nodiscard]] Frame orthonormalized(const Frame& f,
	const Frame& fallback = Frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 } },
	double eps = 1.0e-10) noexcept;
//@}

/// \name Frame construction helpers
//@{
/// \brief Build a frame from the nucleotide normal \p e2 and a strand tangent.
///
/// The returned frame is orthonormal and right-handed. The fallback frame is
/// used when the inputs are degenerate or nearly collinear.
SB_EXPORT [[nodiscard]] Frame frameFromE2AndTangent(const Vec3& e2,
	const Vec3& tangent,
	const Frame& fallback = Frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 } },
	double eps = 1.0e-10) noexcept;

/// \brief Return the rotation matrix for a rotation about the Z axis.
SB_EXPORT [[nodiscard]] Mat3 rotationZ(double radians) noexcept;
/// \brief Return the rotation matrix for a rotation about an arbitrary axis.
///
/// If \p axis is degenerate, the identity rotation is returned.
SB_EXPORT [[nodiscard]] Mat3 rotationAroundAxis(const Vec3& axis, double radians) noexcept;
//@}

} // namespace ADNFrameUtils
