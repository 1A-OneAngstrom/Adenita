#pragma once

#include <cstddef>
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

    ADNArray(std::size_t dim, std::size_t num_elements) {
        dim_ = dim;
        num_elements_ = num_elements;
        std::size_t total = dim * num_elements;
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
        if (dim_ * idx + idy >= num_elements_ * dim_) {
            std::string pos = std::to_string(dim_ * idx + idy);
            std::string total = std::to_string(num_elements_ * dim_);
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx + idy];
    }

    const T& operator()(std::size_t idx, std::size_t idy) const {
        if (dim_ * idx + idy >= num_elements_ * dim_) {
            std::string pos = std::to_string(dim_ * idx + idy);
            std::string total = std::to_string(num_elements_ * dim_);
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx + idy];
    }

    T& operator()(std::size_t idx) {
        if (dim_ * idx >= num_elements_ * dim_) {
            std::string pos = std::to_string(dim_ * idx);
            std::string total = std::to_string(num_elements_ * dim_);
            throw ADNArray<T>::ERROR_OUT_OF_BOUNDS;
        }
        return array_[dim_ * idx];
    }

    const T& operator()(std::size_t idx) const {
        if (dim_ * idx >= num_elements_ * dim_) {
            std::string pos = std::to_string(dim_ * idx);
            std::string total = std::to_string(num_elements_ * dim_);
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

    /**
    * Returns a row as a 1-dim ANTArray
    * \return A 1-dim ANTArray with number of elements equal to this dim_
    */
    [[nodiscard]] ADNArray<T> GetRow(std::size_t row) const {
        ADNArray<T> arr(dim_);
        for (std::size_t i = 0; i < dim_; ++i) {
            arr(i) = this->operator ()(row, i);
        }
        return arr;
    }

    /**
    * Sets a row equal to the values contained in arr.
    * \param the row we want to set.
    * \param a 1-dim ANTArray with number of elements equal to the row length.
    */
    void SetRow(std::size_t row, const ADNArray<T>& arr) {
        if (arr.GetDim() != 1 || dim_ != arr.GetNumElements()) {
            throw ADNArray<T>::ERROR_DIMENSION_MISMATCH;
        }
        else {
            for (std::size_t i = 0; i < dim_; ++i) {
                this->operator ()(row, i) = arr(i);
            }
        }
    }

    /**
    * Concatenates two ANTArrays and returns one
    */
    [[nodiscard]] static ADNArray<T>* Concatenate(const ADNArray<T>& v, const ADNArray<T>& w) {
        if (v.GetDim() != w.GetDim()) {
            throw ADNArray<T>::ERROR_DIMENSION_MISMATCH;
        }

        auto* sol = new ADNArray<T>(v.GetDim(), v.GetNumElements() + w.GetNumElements());
        std::size_t i = 0;
        for (i = 0; i < v.GetNumElements(); ++i) {
            auto row = v.GetRow(i);
            sol->SetRow(i, row);
        }
        for (std::size_t j = 0; j < w.GetNumElements(); ++j) {
            auto row = w.GetRow(j);
            sol->SetRow(i + j, row);
        }
        return sol;
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
