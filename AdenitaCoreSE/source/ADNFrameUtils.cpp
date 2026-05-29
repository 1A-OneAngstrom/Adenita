#include "ADNFrameUtils.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace ADNFrameUtils {

namespace {

[[nodiscard]] Vec3 perpendicularComponent(const Vec3& v, const Vec3& axis) noexcept {

	return v - dot(v, axis) * axis;

}

[[nodiscard]] Vec3 firstValidPerpendicular(const Vec3& axis, double eps) noexcept {

	const std::array<Vec3, 3> candidates{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 }
	};

	Vec3 best = candidates[0];
	double bestAlignment = std::abs(dot(axis, candidates[0]));
	for (const Vec3& candidate : candidates) {

		const double alignment = std::abs(dot(axis, candidate));
		if (alignment < bestAlignment) {

			best = candidate;
			bestAlignment = alignment;

		}

	}

	const Vec3 perpendicular = normalized(perpendicularComponent(best, axis), eps);
	if (!isNearlyZero(perpendicular, eps)) return perpendicular;

	return Vec3{ 0.0, 1.0, 0.0 };

}

[[nodiscard]] Vec3 normalizedFallback(const Vec3& candidate,
	const Vec3& fallback,
	const Vec3& lastResort,
	double eps) noexcept {

	Vec3 value = normalized(candidate, eps);
	if (!isNearlyZero(value, eps)) return value;

	value = normalized(fallback, eps);
	if (!isNearlyZero(value, eps)) return value;

	value = normalized(lastResort, eps);
	if (!isNearlyZero(value, eps)) return value;

	return Vec3{ 1.0, 0.0, 0.0 };

}

} // namespace

Vec3 operator+(const Vec3& a, const Vec3& b) noexcept {

	return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };

}

Vec3 operator-(const Vec3& a, const Vec3& b) noexcept {

	return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z };

}

Vec3 operator-(const Vec3& v) noexcept {

	return Vec3{ -v.x, -v.y, -v.z };

}

Vec3 operator*(const Vec3& v, double scalar) noexcept {

	return Vec3{ v.x * scalar, v.y * scalar, v.z * scalar };

}

Vec3 operator*(double scalar, const Vec3& v) noexcept {

	return v * scalar;

}

Vec3 operator/(const Vec3& v, double scalar) noexcept {

	if (std::abs(scalar) <= std::numeric_limits<double>::min()) return Vec3{};
	return Vec3{ v.x / scalar, v.y / scalar, v.z / scalar };

}

Frame identityFrame() noexcept {

	return Frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 }
	};

}

double dot(const Vec3& a, const Vec3& b) noexcept {

	return a.x * b.x + a.y * b.y + a.z * b.z;

}

Vec3 cross(const Vec3& a, const Vec3& b) noexcept {

	return Vec3{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};

}

double squaredNorm(const Vec3& v) noexcept {

	return dot(v, v);

}

double norm(const Vec3& v) noexcept {

	return std::sqrt(squaredNorm(v));

}

Vec3 normalized(const Vec3& v, double eps) noexcept {

	if (!isFinite(v)) return Vec3{};

	const double length = norm(v);
	if (!std::isfinite(length) || length <= eps) return Vec3{};

	return v / length;

}

bool isNearlyZero(const Vec3& v, double eps) noexcept {

	return squaredNorm(v) <= eps * eps;

}

Vec3 rotated(const Mat3& r, const Vec3& v) noexcept {

	return Vec3{
		r.m[0][0] * v.x + r.m[0][1] * v.y + r.m[0][2] * v.z,
		r.m[1][0] * v.x + r.m[1][1] * v.y + r.m[1][2] * v.z,
		r.m[2][0] * v.x + r.m[2][1] * v.y + r.m[2][2] * v.z
	};

}

double determinant(const Frame& f) noexcept {

	return dot(f.e1, cross(f.e2, f.e3));

}

bool isFinite(const Vec3& v) noexcept {

	return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);

}

bool isNearlyUnit(const Vec3& v, double eps) noexcept {

	return isFinite(v) && std::abs(norm(v) - 1.0) <= eps;

}

bool isOrthonormalRightHanded(const Frame& f, double eps) noexcept {

	return isNearlyUnit(f.e1, eps) &&
		isNearlyUnit(f.e2, eps) &&
		isNearlyUnit(f.e3, eps) &&
		std::abs(dot(f.e1, f.e2)) <= eps &&
		std::abs(dot(f.e1, f.e3)) <= eps &&
		std::abs(dot(f.e2, f.e3)) <= eps &&
		determinant(f) > 0.0;

}

Frame rotated(const Mat3& r, const Frame& f) noexcept {

	return Frame{
		rotated(r, f.e1),
		rotated(r, f.e2),
		rotated(r, f.e3)
	};

}

Frame orthonormalized(const Frame& f, const Frame& fallback, double eps) noexcept {

	const Frame identity = identityFrame();

	const Vec3 e1 = normalizedFallback(f.e1, fallback.e1, identity.e1, eps);

	Vec3 e2Candidate = perpendicularComponent(f.e2, e1);
	Vec3 e2Fallback = perpendicularComponent(fallback.e2, e1);
	Vec3 e2 = normalizedFallback(e2Candidate, e2Fallback, firstValidPerpendicular(e1, eps), eps);

	Vec3 e3 = normalized(cross(e1, e2), eps);
	if (isNearlyZero(e3, eps)) {

		e2 = firstValidPerpendicular(e1, eps);
		e3 = normalized(cross(e1, e2), eps);

	}

	e2 = normalized(cross(e3, e1), eps);

	Frame repaired{ e1, e2, e3 };
	if (!isOrthonormalRightHanded(repaired, 1.0e-6)) {

		repaired.e3 = -repaired.e3;
		repaired.e2 = normalized(cross(repaired.e3, repaired.e1), eps);

	}

	if (!isOrthonormalRightHanded(repaired, 1.0e-6))
		return identity;

	return repaired;

}

Frame frameFromE2AndTangent(const Vec3& e2,
	const Vec3& tangent,
	const Frame& fallback,
	double eps) noexcept {

	const Vec3 normalizedE2 = normalizedFallback(e2, fallback.e2, Vec3{ 0.0, 1.0, 0.0 }, eps);
	Vec3 tangentPerpendicular = perpendicularComponent(tangent, normalizedE2);
	Vec3 normalizedE3 = normalized(tangentPerpendicular, eps);
	if (isNearlyZero(normalizedE3, eps)) {

		tangentPerpendicular = perpendicularComponent(fallback.e3, normalizedE2);
		normalizedE3 = normalized(tangentPerpendicular, eps);

	}
	if (isNearlyZero(normalizedE3, eps))
		normalizedE3 = firstValidPerpendicular(normalizedE2, eps);

	const Vec3 e1 = normalized(cross(normalizedE2, normalizedE3), eps);
	return orthonormalized(Frame{ e1, normalizedE2, normalizedE3 }, fallback, eps);

}

Mat3 rotationZ(double radians) noexcept {

	const double c = std::cos(radians);
	const double s = std::sin(radians);

	return Mat3{
		{
			{ c, -s, 0.0 },
			{ s, c, 0.0 },
			{ 0.0, 0.0, 1.0 }
		}
	};

}

Mat3 rotationAroundAxis(const Vec3& axis, double radians) noexcept {

	const Vec3 normalizedAxis = normalized(axis);
	if (isNearlyZero(normalizedAxis)) return rotationZ(0.0);

	const double x = normalizedAxis.x;
	const double y = normalizedAxis.y;
	const double z = normalizedAxis.z;
	const double c = std::cos(radians);
	const double s = std::sin(radians);
	const double oneMinusC = 1.0 - c;

	return Mat3{
		{
			{ c + x * x * oneMinusC, x * y * oneMinusC - z * s, x * z * oneMinusC + y * s },
			{ y * x * oneMinusC + z * s, c + y * y * oneMinusC, y * z * oneMinusC - x * s },
			{ z * x * oneMinusC - y * s, z * y * oneMinusC + x * s, c + z * z * oneMinusC }
		}
	};

}

} // namespace ADNFrameUtils
