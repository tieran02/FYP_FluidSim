#pragma once

template<size_t K>
using point_t = glm::vec<K, float, glm::defaultp>;

template<size_t K, typename ElementType>
class INearestNeighbor
{
 public:
	virtual bool FindNearestNeighbor(const point_t<K>& point, ElementType& element) = 0;
	virtual bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<ElementType*>& elements) = 0;
};