#pragma once

#include <stdexcept>
#include <string>

#include "ADNCell.hpp"

class ADNNodeValidation final {

public:

	static bool IsAdenitaNode(SBNode* node, const std::string& className) {

		if (node == nullptr) return false;
		auto proxy = node->getProxy();
		return proxy != nullptr &&
			proxy->getName() == className &&
			proxy->getElementUUID() == SBUUID(SB_ELEMENT_UUID);

	}

	static bool IsAdenitaCellNode(SBNode* node) {

		if (node == nullptr) return false;
		auto proxy = node->getProxy();
		if (proxy == nullptr || proxy->getElementUUID() != SBUUID(SB_ELEMENT_UUID)) return false;

		const std::string className = proxy->getName();
		return className == "ADNCell" ||
			className == "ADNBasePair" ||
			className == "ADNSkipPair" ||
			className == "ADNLoopPair";

	}

	static SBNode* GetSerializedNode(const SBNodeIndexer& nodeIndexer, unsigned int index, const std::string& description) {

		if (index == InvalidNodeIndex()) return nullptr;
		if (index >= nodeIndexer.size())
			throw std::runtime_error("Invalid Adenita serialized " + description + " node index.");

		SBNode* node = nodeIndexer.getNode(index);
		if (node == nullptr)
			throw std::runtime_error("Missing Adenita serialized " + description + " node.");

		return node;

	}

	template <typename T>
	static SBPointer<T> GetSerializedAdenitaNode(const SBNodeIndexer& nodeIndexer, unsigned int index, const std::string& className) {

		SBNode* node = GetSerializedNode(nodeIndexer, index, className);
		if (node == nullptr) return nullptr;

		if (!IsAdenitaNode(node, className))
			throw std::runtime_error("Unexpected Adenita serialized node type for " + className + ".");

		T* typedNode = dynamic_cast<T*>(node);
		if (typedNode == nullptr)
			throw std::runtime_error("Failed to cast Adenita serialized node to " + className + ".");

		return typedNode;

	}

	static SBPointer<ADNCell> GetSerializedAdenitaCell(const SBNodeIndexer& nodeIndexer, unsigned int index) {

		SBNode* node = GetSerializedNode(nodeIndexer, index, "ADNCell");
		if (node == nullptr) return nullptr;

		if (!IsAdenitaCellNode(node))
			throw std::runtime_error("Unexpected Adenita serialized node type for ADNCell.");

		ADNCell* cell = dynamic_cast<ADNCell*>(node);
		if (cell == nullptr)
			throw std::runtime_error("Failed to cast Adenita serialized node to ADNCell.");

		return cell;

	}

private:

	static unsigned int InvalidNodeIndex() noexcept {

		return static_cast<unsigned int>(-1);

	}

};
