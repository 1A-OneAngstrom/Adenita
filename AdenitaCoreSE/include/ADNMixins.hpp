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

#if 0
class SB_EXPORT Positionable {

public:

    Positionable() = default;
    ~Positionable() = default;
    Positionable(const Positionable& other);

    Positionable& operator=(const Positionable& other);

    void SetPosition(const ublas::vector<double>& pos);
    [[nodiscard]] const ublas::vector<double>& GetPosition() const;

private:

    ublas::vector<double> position_;

};
#endif

class ADNAtom; // PositionableSB needs ADNAtom defined, hence defined here

class SB_EXPORT PositionableSB {

public:

    PositionableSB();
    ~PositionableSB() = default;
    PositionableSB(const PositionableSB& other);

    PositionableSB& operator=(const PositionableSB& other);

    void SetPosition(const SBPosition3& pos);
    [[nodiscard]] const SBPosition3& GetPosition() const;

    [[nodiscard]] SBPointer<ADNAtom> GetCenterAtom() const;
    void SetCenterAtom(SBPointer<ADNAtom> centerAtom);
    void HideCenterAtom();

private:

    SBPointer<ADNAtom> centerAtom_;

};

#if 0
class SB_EXPORT Identifiable {

public:

    Identifiable() = default;
    ~Identifiable() = default;
    Identifiable(const Identifiable& other);

    Identifiable& operator=(const Identifiable& other);

    void SetId(int id) noexcept;
    [[nodiscard]] int GetId() const noexcept;

private:

    int id_ = -1;

};
#endif

class SB_EXPORT Orientable {

public:

    Orientable();
    ~Orientable() = default;
    Orientable(const Orientable& other);

    Orientable& operator=(const Orientable& other);

    void SetE1(const ublas::vector<double>& e1);
    void SetE2(const ublas::vector<double>& e2);
    void SetE3(const ublas::vector<double>& e3);
    [[nodiscard]] const ublas::vector<double>& GetE1() const;
    [[nodiscard]] const ublas::vector<double>& GetE2() const;
    [[nodiscard]] const ublas::vector<double>& GetE3() const;

private:

    ublas::vector<double> e1_;
    ublas::vector<double> e2_;
    ublas::vector<double> e3_;

};

#if 0
template <class T>
class SB_EXPORT Collection {

public:

    Collection() = default;
    ~Collection() = default;
    Collection(const Collection<T>& other);

    Collection<T>& operator=(const Collection<T>& other);

    void AddElement(SBPointer<T> elem, int id = -1);
    [[nodiscard]] const SBPointerIndexer<T>& GetCollection() const;
    void DeleteElement(int id);
    [[nodiscard]] int GetLastKey() const;
    [[nodiscard]] SBPointer<T> GetElement(int id) const;

private:

    SBPointerIndexer<T> collection_;

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
inline void Collection<T>::AddElement(SBPointer<T> elem, int id) {

    if (id == -1) {

        id = 0;
        if (collection_.size() > 0) id = collection_.rbegin()->first + 1;

    }

    collection_.insert(std::make_pair(id, elem));

}

template<class T>
inline const SBPointerIndexer<T>& Collection<T>::GetCollection() const {

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
inline SBPointer<T> Collection<T>::GetElement(int id) const {

    return collection_.at(id);

}
#endif
