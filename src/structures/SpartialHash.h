#pragma once
#include <glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include "INearestNeighbor.h"
#include "gtx/string_cast.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <omp.h>
#include <unordered_set>


template<size_t K>
class SpartialHash : public INearestNeighbor<K>
{
 public:
	SpartialHash(uint32_t bucketSize);
	SpartialHash(const std::vector<point_t<K>>& points, uint32_t bucketSize);

	void Build(const std::vector<point_t<K>>& points) override;

	/// Find the Nearest neighbour from the given point (Note: this will only search the bucket that contains the point)
	/// \param point
	/// \param index ref that gets assigned the closest index
	bool FindNearestNeighbor(const point_t<K>& point, uint32_t& index) override;

	/// Find a list of the nearest neighbours from the given point that fit within the radius
	/// \param point
	/// \param radius of the search from point
	/// \param indices ref that gets assigned the closest indices
	bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<uint32_t>& indices) override;

	bool FindAllNearestNeighbors(const std::vector<point_t<K>>& points,
		float radius,
		std::vector<std::vector<uint32_t>>& indices) override;
 private:
	typedef std::pair<point_t<K>,size_t> pair;
	const uint32_t m_bucketSize;
	std::unordered_map<size_t, std::vector<pair>> m_buckets;

	const int P1 = 73856093;
	const int P2 = 19349663;
	const int P3 = 83492791;

	size_t getHashKey(point_t<K> point) const;
	void getAllIndicesFromHashkey(size_t hashKey, std::vector<pair>& indices);
	std::vector<glm::vec3> getAllBucketPointsWithinRange(const point_t<K>& origin, float radius) const;
};

template<size_t K>
inline SpartialHash<K>::SpartialHash(uint32_t bucketSize) : m_bucketSize(bucketSize)
{
}

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
		m_buckets[hash].push_back(std::make_pair(points[i],i));
	}
}

template<size_t K>
bool SpartialHash<K>::FindNearestNeighbor(const point_t<K>& point, uint32_t& index)
{
	//hash point
	size_t pointHashkey = getHashKey(point);
	std::vector<pair> neighbors{};
	getAllIndicesFromHashkey(pointHashkey,neighbors);

	if(neighbors.empty())
	{
		return false;
	}
	else if(neighbors.size() == 1)
	{
		index = neighbors[0].second;
		return true;
	}

	float closestDistance = std::numeric_limits<float>::infinity();
	size_t closestIndex = 0;

	for (int i = 0; i < neighbors.size(); ++i)
	{
		float distance = glm::distance2(neighbors[i].first,point);
		if(distance < closestDistance)
		{
			closestDistance = distance;
			closestIndex = i;
		}
	}

	index = closestIndex;
	return true;
}

template<size_t K>
bool SpartialHash<K>::FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<uint32_t>& indices)
{
	float radius2 = radius * radius;
	auto searchBuckets = getAllBucketPointsWithinRange(point,radius2);
	std::unordered_set<size_t> visitedPoints;

	for(const auto& bucket : searchBuckets)
	{
		size_t hashKey = getHashKey(bucket);

		std::vector<pair> neighbors{};
		getAllIndicesFromHashkey(hashKey,neighbors);

		for (int i = 0; i < neighbors.size(); i++)
		{
			if(visitedPoints.find(neighbors[i].second) != visitedPoints.end())
				continue;

			visitedPoints.insert(neighbors[i].second);
			//calc distance
			float distance2 = glm::distance2(point, neighbors[i].first);
			if (distance2 <= radius2)
			{
				indices.push_back(neighbors[i].second);
			}
		}
	}

	return !indices.empty();
}

template<size_t K>
size_t SpartialHash<K>::getHashKey(point_t<K> point) const
{
	//cast to int
	pointi_t<K> pointI;
	for (int i = 0; i < K; ++i)
	{
		pointI[i] = static_cast<int>(floor(point[i] / m_bucketSize));
	}


	//return (pointI.x * 73856093 ^ pointI.y * 19349663 ^ pointI.z * 83492791) % 1000;
	return std::hash<pointi_t<K>>{}(pointI);
}

template<size_t K>
void SpartialHash<K>::getAllIndicesFromHashkey(size_t hashKey, std::vector<pair>& indices)
{
	if(m_buckets.find(hashKey) != m_buckets.end())
	{
		indices = m_buckets[hashKey];
	}
}

template<size_t K>
std::vector<glm::vec3> SpartialHash<K>::getAllBucketPointsWithinRange(const point_t<K>& origin, float radius) const
{
	const glm::vec3 A{m_bucketSize,0.0f,0.0f};
	const glm::vec3 B{0.0f,m_bucketSize,0.0f};
	const glm::vec3 C{0.0f,0.0f,m_bucketSize};

	uint32_t bucketSize2 = m_bucketSize * m_bucketSize;
	int bucketRadius2 = static_cast<int>(ceil(bucketSize2));

	int searchRadius = ceil(radius/bucketRadius2);
	const glm::vec3 topLeft = origin - (float)(m_bucketSize*searchRadius);

	int searchSize = searchRadius+2;
	std::vector<glm::vec3> result;
	result.reserve(searchSize * searchSize * searchSize);

	for (int i = 0; i < searchSize; ++i)
	{
		for (int j = 0; j < searchSize; ++j)
		{
			for (int k = 0; k < searchSize; ++k)
			{
				result.emplace_back(topLeft + A * (float)i + B * (float)j + C * (float)k);
			}
		}
	}

	return result;
}

template<size_t K>
bool SpartialHash<K>::FindAllNearestNeighbors(const std::vector<point_t<K>>& points,
	float radius,
	std::vector<std::vector<uint32_t>>& indices)
{
	return false;
}
