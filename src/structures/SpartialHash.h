#pragma once
#include <glm.hpp>
#include <memory>
#include <vector>
#include "INearestNeighbor .h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

template<size_t K>
class SpartialHash : public INearestNeighbor<K>
{
 public:
	SpartialHash(const std::vector<point_t<K>>& points, uint32_t bucketSize);

	void Build(const std::vector<point_t<K>>& points) override;
	bool FindNearestNeighbor(const point_t<K>& point, size_t & index) override;
	bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<size_t>& indices) override;
 private:
	const uint32_t m_bucketSize;
	std::unordered_map<size_t, std::vector<size_t>> m_buckets;

	size_t getHashKey(point_t<K> point) const;
};

template<size_t K>
SpartialHash<K>::SpartialHash(const std::vector<point_t<K>>& points, uint32_t bucketSize) : m_bucketSize(bucketSize)
{
	Build(points);
}

template<size_t K>
void SpartialHash<K>::Build(const std::vector<point_t<K>>& points)
{
	for (int i = 0; i < points.size(); ++i)
	{
		//get hash from point
		size_t hash = getHashKey(points[i]);
		m_buckets[hash].push_back(i);

	}
}

template<size_t K>
bool SpartialHash<K>::FindNearestNeighbor(const point_t<K>& point, size_t & index)
{
	return false;
}

template<size_t K>
bool SpartialHash<K>::FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<size_t>& indices)
{
	return false;
}
template<size_t K>
size_t SpartialHash<K>::getHashKey(point_t<K> point) const
{
	//cast to int
	pointi_t<K> pointI;
	for (int i = 0; i < K; ++i)
	{
		pointI[i] = (int)floor(point[i] / m_bucketSize);
	}

	size_t hash = std::hash<pointi_t<K>>{}(pointI);
	return hash;
}
