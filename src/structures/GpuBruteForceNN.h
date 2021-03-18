#pragma once
#include <vector>
#include <common.hpp>
#include <opencl/OpenCLContext.h>
#include <memory>
#include "INearestNeighbor.h"

class GpuBruteForceNN : public INearestNeighbor<4>
{
 public:
	GpuBruteForceNN(const std::vector<point4_t>& points, OpenCLContext& context);

	/// Build uploads the point data to the GPU
	/// \param points
	void Build(const std::vector<point4_t>& points) override;

	bool FindNearestNeighbor(const point4_t& point, uint32_t& index) override;
	bool FindNearestNeighbors(const point4_t& point,
	                          float radius,
	                          std::vector<uint32_t>& indices) override;

	bool FindAllNearestNeighbors(const std::vector<point4_t>& points,
		float radius,
		std::vector<std::vector<uint32_t>>& indices) override;
 private:
	OpenCLContext& m_openCLContext;
	std::unique_ptr<cl::Buffer> m_pointBuffer;
	std::unique_ptr<cl::Buffer> m_neighboursBuffer;
	cl_uint m_pointCount;
};
