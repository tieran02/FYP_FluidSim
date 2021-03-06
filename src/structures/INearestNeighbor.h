#pragma once
#include "glm.hpp"

template<size_t K>
using point_t = glm::vec<K, float, glm::defaultp>;

using point4_t = glm::vec<4, float, glm::defaultp>;

template<size_t K>
using pointi_t = glm::vec<K, int32_t, glm::defaultp>;

template<size_t K>
class INearestNeighbor
{
 public:
	virtual void Build(const std::vector<point_t<K>>& points) = 0;
	virtual bool FindNearestNeighbor(const point_t<K>& point, uint32_t& index) = 0;
	virtual bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<uint32_t>& indices) = 0;
	virtual bool FindAllNearestNeighbors(const std::vector<point_t<K>>& points, float radius, std::vector<std::vector<uint32_t>>& indices) = 0;
};