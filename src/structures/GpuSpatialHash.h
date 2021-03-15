#pragma once
#include <opencl/OpenCLContext.h>
#include <vector>
#include <common.hpp>
#include <memory>
#include <math/AABB.h>
#include "INearestNeighbor .h"

struct GpuHashPoint
{
	cl_uint Hash;
	cl_uint	SourceIndex;
};

class GpuSpatialHash: public INearestNeighbor<4>
{
 public:
	GpuSpatialHash(const std::vector<point4_t>& points, const AABB& aabb, OpenCLContext& context);

	/// Build uploads the point data to the GPU
	/// \param points
	void Build(const std::vector<point4_t>& points) override;

	bool FindNearestNeighbor(const point4_t& point, uint32_t& index) override;
	bool FindNearestNeighbors(const point4_t& point,
	                          float radius,
	                          std::vector<uint32_t>& indices) override;

	bool KNN(const std::vector<glm::vec4>& points,const std::vector<glm::vec4>& queryPoints, const AABB& aabb, uint32_t K, float radius, std::vector<uint32_t>& indices);
 private:
	AABB m_aabb;

	OpenCLContext& m_openCLContext;
	std::unique_ptr<cl::Buffer> m_pointBuffer;
	std::unique_ptr<cl::Buffer> m_sortedPointBuffer;
	std::unique_ptr<cl::Buffer> m_cellStartIndexBuffer;
	std::unique_ptr<cl::Buffer> m_cellSizeIndexBuffer;
	cl_uint m_pointCount;
	cl_uint m_subdivisions;
};