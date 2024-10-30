#pragma once

#include <string>
#include <memory>
#include <map>

#include "SBResidue.hpp"
#include "SBCReferencePointer.hpp"
#include "SBCReferencePointerIndexer.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector.hpp>


namespace ublas = boost::numeric::ublas;

/* Using smart pointers */
//template <typename T>
//using ADNPointer = std::shared_ptr<T>;

template <typename T>
using ADNPointer = SBPointer<T>;

//template <typename T>
//using ADNWeakPointer = std::weak_ptr<T>;

template <typename T>
using ADNWeakPointer = SBPointer<T>;

//template<typename T>
//using CollectionMap = std::map<int, ADNPointer<T>>;

template<typename T>
using CollectionMap = SBPointerIndexer<T>;

//using Position3D = ublas::vector<double>;
using Position3D = SBPosition3;

using DNABlocks = SBResidue::ResidueType;

using NucleotideGroup = SBNode::Type;

#if 0
// used as a base class of the ADNNanorobot class, while there was no need for naming
class SB_EXPORT Nameable {

public:

    Nameable() = default;
    ~Nameable() = default;
    Nameable(const Nameable& other);

    Nameable& operator=(const Nameable& other);

    void SetName(std::string name);
    std::string GetName() const;

private:

    std::string name_;

};
#endif

class SB_EXPORT Positionable {

public:

    Positionable() = default;
    ~Positionable() = default;
    Positionable(const Positionable& other);

    Positionable& operator=(const Positionable& other);

    void SetPosition(const ublas::vector<double>& pos);
    const ublas::vector<double>& GetPosition() const;

private:

    ublas::vector<double> position_;

};

class ADNAtom; // PositionableSB needs ADNAtom defined, hence defined here

class SB_EXPORT PositionableSB {

public:

    PositionableSB();
    ~PositionableSB() = default;
    PositionableSB(const PositionableSB& other);

    PositionableSB& operator=(const PositionableSB& other);

    void SetPosition(const Position3D& pos);
    const Position3D& GetPosition() const;

    ADNPointer<ADNAtom> GetCenterAtom() const;
    void SetCenterAtom(ADNPointer<ADNAtom> centerAtom);
    void HideCenterAtom();

private:

    ADNPointer<ADNAtom> centerAtom_;

};

class SB_EXPORT Identifiable {

public:

    Identifiable() = default;
    ~Identifiable() = default;
    Identifiable(const Identifiable& other);

    Identifiable& operator=(const Identifiable& other);

    void SetId(int id) noexcept;
    int GetId() const noexcept;

private:

    int id_ = -1;

};

class SB_EXPORT Orientable {

public:

    Orientable();
    ~Orientable() = default;
    Orientable(const Orientable& other);

    Orientable& operator=(const Orientable& other);

    void SetE1(const ublas::vector<double>& e1);
    void SetE2(const ublas::vector<double>& e2);
    void SetE3(const ublas::vector<double>& e3);
    const ublas::vector<double>& GetE1() const;
    const ublas::vector<double>& GetE2() const;
    const ublas::vector<double>& GetE3() const;

private:

    ublas::vector<double> e1_;
    ublas::vector<double> e2_;
    ublas::vector<double> e3_;

};

template <class T>
class SB_EXPORT Collection {

public:

    Collection() = default;
    ~Collection() = default;
    Collection(const Collection<T>& other);

    Collection<T>& operator=(const Collection<T>& other);

    void AddElement(ADNPointer<T> elem, int id = -1);
    const CollectionMap<T>& GetCollection() const;
    void DeleteElement(int id);
    int GetLastKey() const;
    ADNPointer<T> GetElement(int id) const;

private:

    CollectionMap<T> collection_;

};

template<class T>
inline Collection<T>::Collection(const Collection<T>& other) {

    *this = other;

}

template<class T>
inline Collection<T>& Collection<T>::operator=(const Collection<T>& other) {

    if (&other == this)
        return *this;

    collection_ = other.GetCollection();

    return *this;

}

template<class T>
inline void Collection<T>::AddElement(ADNPointer<T> elem, int id) {

    if (id == -1) {

        id = 0;
        if (collection_.size() > 0) id = collection_.rbegin()->first + 1;

    }

    collection_.insert(std::make_pair(id, elem));

}

template<class T>
inline const CollectionMap<T>& Collection<T>::GetCollection() const {

    return collection_;

}

template<class T>
inline void Collection<T>::DeleteElement(int id) {

    collection_.erase(id);

}

template<class T>
inline int Collection<T>::GetLastKey() const {

    int id = -1;
    if (collection_.size() > 0) {
        id = collection_.rbegin()->first;
    }

    return id;

}

template<class T>
inline ADNPointer<T> Collection<T>::GetElement(int id) const {

    return collection_.at(id);

}
