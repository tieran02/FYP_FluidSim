#pragma once
#include <vector>
#include <common.hpp>
#include <opencl/OpenCLContext.h>
#include <memory>
#include "INearestNeighbor .h"

class GpuBruteForceNN : public INearestNeighbor<4>
{
 public:
	GpuBruteForceNN(const std::vector<point4_t>& points, OpenCLContext& context);

	/// Build uploads the point data to the GPU
	/// \param points
	void Build(const std::vector<point4_t>& points) override;

	bool FindNearestNeighbor(const point4_t& point, size_t& index) override;
	bool FindNearestNeighbors(const point4_t& point,
		float radius,
		std::vector<size_t>& indices) override;
 private:
	OpenCLContext& m_openCLContext;
	std::unique_ptr<cl::Buffer> m_pointBuffer;
	std::unique_ptr<cl::Buffer> m_neighboursBuffer;
	cl_uint m_pointCount;
};
