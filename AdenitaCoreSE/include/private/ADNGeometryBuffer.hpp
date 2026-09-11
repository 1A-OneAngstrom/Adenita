#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace ADNGeometryBuffer {

/// \brief Copies borrowed computation storage into an exclusively owned geometry buffer.
/// \param destination Current geometry allocation, borrowed until its owning setter replaces it.
/// \param oldSize Number of elements in the current allocation.
/// \param source Borrowed computation data; nullptr clears the geometry buffer.
/// \param size Number of elements to copy.
/// \return Reused buffer or a new[] allocation for immediate ownership transfer; nullptr when empty.
/// \throws std::bad_alloc If replacement allocation fails; the previous buffer remains owned by its geometry.
template<typename T>
T* copyGeometryBuffer(T* destination, std::size_t oldSize, const T* source, std::size_t size) {
	static_assert(std::is_trivially_copyable_v<T>, "Geometry snapshots require trivially copyable elements");


	if (!size || !source) return nullptr;
	T* replacement = destination && oldSize == size ? destination : new T[size];
	std::copy_n(source, size, replacement);
	return replacement;

}

}
