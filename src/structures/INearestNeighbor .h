#pragma once

template<size_t K>
using point_t = glm::vec<K, float, glm::defaultp>;

template<size_t K>
using pointi_t = glm::vec<K, int32_t, glm::defaultp>;

template<size_t K>
class INearestNeighbor
{
 public:
	virtual void Build(const std::vector<point_t<K>>& points) = 0;
	virtual bool FindNearestNeighbor(const point_t<K>& point, size_t& index) = 0;
	virtual bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<size_t>& indices) = 0;
};