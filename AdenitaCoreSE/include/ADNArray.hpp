#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "SBCHeapExport.hpp"

#define _CRTDBG_MAP_ALLOC

template <class T> class SB_EXPORT ADNArray {
public:
    ADNArray() = default;

    ADNArray(const ADNArray& obj) : ADNArray(obj.GetDim(), obj.GetNumElements()) {
        std::size_t total = dim_ * num_elements_;
        std::size_t i = 0;
        auto arr = obj.GetArray();
        for (i = 0; i < total; ++i) {
            array_[i] = arr[i];
        }
    }

    ADNArray(ADNArray&& other) noexcept {
        swap(other);
    }

    /// \brief Allocates an array with checked row and column counts.
    /// \param dim Number of columns per row.
    /// \param num_elements Number of rows.
    /// \throws std::length_error If the shape or storage size cannot be represented.
    ADNArray(std::size_t dim, std::size_t num_elements) {
        const auto maximum = std::numeric_limits<std::size_t>::max();
        if (dim != 0 && num_elements > maximum / dim)
            throw std::length_error("ADNArray shape is too large");
        const std::size_t total = dim * num_elements;
        if (total > maximum / sizeof(T))
            throw std::length_error("ADNArray storage is too large");
        dim_ = dim;
        num_elements_ = num_elements;
        array_ = new T[total];
    }

    // For C++11: 
    ADNArray(std::size_t num_elements) : ADNArray(1, num_elements) {}

    ~ADNArray() {
        delete[] array_;
    }

    ADNArray<T>& operator=(ADNArray<T> other) noexcept {
        swap(other);
        return *this;
    }

    T& operator()(std::size_t idx, std::size_t idy) {
        // Check each coordinate before flattening: an invalid column can otherwise alias the next row.
        if (idx >= num_elements_ || idy >= dim_) {
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx + idy];
    }

    const T& operator()(std::size_t idx, std::size_t idy) const {
        if (idx >= num_elements_ || idy >= dim_) {
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx + idy];
    }

    T& operator()(std::size_t idx) {
        if (idx >= num_elements_ || dim_ == 0) {
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx];
    }

    const T& operator()(std::size_t idx) const {
        if (idx >= num_elements_ || dim_ == 0) {
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx];
    }

    [[nodiscard]] T* GetArray() const { return array_; }
    [[nodiscard]] std::size_t GetDim() const noexcept { return dim_; }
    [[nodiscard]] std::size_t GetNumElements() const noexcept { return num_elements_; }

    void swap(ADNArray<T>& other) noexcept {
        using std::swap;
        swap(array_, other.array_);
        swap(dim_, other.dim_);
        swap(num_elements_, other.num_elements_);
    }

    /// \brief Copies one row into a one-column array.
    /// \param row Zero-based row index, also checked for zero-width arrays.
    /// \return An independent array containing the row values.
    /// \throws int Error 30 if the row is out of bounds.
    [[nodiscard]] ADNArray<T> GetRow(std::size_t row) const {
        if (row >= num_elements_) throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        ADNArray<T> arr(dim_);
        for (std::size_t i = 0; i < dim_; ++i) {
            arr(i) = this->operator ()(row, i);
        }
        return arr;
    }

    /// \brief Replaces one row with the supplied values.
    /// \param row Zero-based destination row index.
    /// \param arr One-column array containing exactly one row's values.
    /// \throws int Error 31 for incompatible dimensions, or 30 for an invalid row.
    void SetRow(std::size_t row, const ADNArray<T>& arr) {
        if (arr.GetDim() != 1 || dim_ != arr.GetNumElements()) {
            throw ADNArray<T>::ERROR_DIMENSION_MISMATCH;
        }
        else {
            if (row >= num_elements_) throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
            for (std::size_t i = 0; i < dim_; ++i) {
                this->operator ()(row, i) = arr(i);
            }
        }
    }

    /// \brief Concatenates arrays with equal column counts.
    /// \param v Rows to place first.
    /// \param w Rows to append.
    /// \return A new array owned by the caller.
    /// \throws int Error 31 if the column counts differ.
    /// \throws std::length_error If the resulting shape or storage size is too large.
    [[nodiscard]] static ADNArray<T>* Concatenate(const ADNArray<T>& v, const ADNArray<T>& w) {
        if (v.GetDim() != w.GetDim()) {
            throw ADNArray<T>::ERROR_DIMENSION_MISMATCH;
        }

        if (w.GetNumElements() > std::numeric_limits<std::size_t>::max() - v.GetNumElements())
            throw std::length_error("ADNArray concatenation is too large");
        // Keep the result owned until all element copies have succeeded.
        auto sol = std::make_unique<ADNArray<T>>(v.GetDim(), v.GetNumElements() + w.GetNumElements());
        if (v.GetDim() == 0) return sol.release();
        std::size_t i = 0;
        for (i = 0; i < v.GetNumElements(); ++i) {
            auto row = v.GetRow(i);
            sol->SetRow(i, row);
        }
        for (std::size_t j = 0; j < w.GetNumElements(); ++j) {
            auto row = w.GetRow(j);
            sol->SetRow(i + j, row);
        }
        return sol.release();
    }

private:

  T* array_{ nullptr };
  std::size_t dim_{ 0 };
  std::size_t num_elements_{ 0 };
  static int ERROR_OUT_OF_BOUNDS;
  static int ERROR_DIMENSION_MISMATCH;

};

template<typename T>
int ADNArray<T>::ERROR_OUT_OF_BOUNDS = 30;

template<typename T>
int ADNArray<T>::ERROR_DIMENSION_MISMATCH = 31;
