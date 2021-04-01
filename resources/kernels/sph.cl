#define MAX_LOCAL_SIZE 256
#define K 256

__constant float KERNEL_RADIUS  = 0.4f;
__constant float KERNEL_RADIUS2 = 0.4f * 0.4f;
__constant float KERNEL_RADIUS3 = 0.4f * 0.4f * 0.4f;
__constant float KERNEL_RADIUS4 = 0.4f * 0.4f * 0.4f * 0.4f;
__constant float KERNEL_RADIUS5 = 0.4f * 0.4f * 0.4f * 0.4f * 0.4f;
__constant float MASS = 2.0f;
__constant float MASS2 = 2.0f * 2.0f;
__constant float VISCOSITY_COEFFICIENT = 0.0074f;

float SmoothedKernelValue(float distance)
{
	if(distance * distance >= KERNEL_RADIUS2)
		return 0.0f;
	
	float x = 1.0f - distance * distance / KERNEL_RADIUS2;
	return 315.0f / (64.0f * M_PI * KERNEL_RADIUS3) * x * x * x;
}

float SmoothedKernelFirstDerivative(float distance)
{
	if(distance >= KERNEL_RADIUS)
		return 0.0f;
	
	float x = 1.0f - distance * distance / KERNEL_RADIUS2;
	return -945.0f / (32.0f * M_PI * KERNEL_RADIUS5) * distance * x * x;
}

float SpikedKernelValue(float distance)
{
	if(distance >= KERNEL_RADIUS)
		return 0.0f;
	
	float x = 1.0f - distance / KERNEL_RADIUS;
	return 15.0f / (M_PI * KERNEL_RADIUS3) * x * x * x;
}

float SpikedKernelFirstDerivative(float distance)
{
	if(distance >= KERNEL_RADIUS)
		return 0.0f;
	
	float x = 1.0f - distance / KERNEL_RADIUS;
	return -45.0f / (M_PI * KERNEL_RADIUS4) * x * x;
}

float SpikedKernelSecondDerivative(float distance)
{
	if(distance >= KERNEL_RADIUS)
		return 0.0f;
	
	float x = 1.0f - distance / KERNEL_RADIUS;
	return 90.0f / (M_PI * KERNEL_RADIUS5) * x;
}

float3 SpikedKernelGradiant(float distance, float3 direction)
{
	return -SpikedKernelFirstDerivative(distance) * direction;
}

__kernel void SumOfKernel(__global const float3* positions, __global const uint* neighbors, __global float* kernelSum)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory
	uint neighbourOffset = (i + offset) * K;

	__local float3 local_pos[MAX_LOCAL_SIZE];
	__local float local_kernelSum[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[1];
 	copy_events[0] = async_work_group_copy(local_pos,positions+offset,wg,0);
    wait_group_events(1,copy_events);

	float sum = 0.0f;
	for(int n = 0; n < K; n++)
	{
		uint neighborIndex = neighbors[n+neighbourOffset];

		if(i+offset == neighborIndex || neighborIndex == UINT_MAX)
			continue;

		float dist = distance(local_pos[i],positions[neighborIndex]);
		float weight = SmoothedKernelValue(dist);
		sum += weight;
	}

	local_kernelSum[i] = sum;
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(kernelSum+offset,local_kernelSum,wg,0);
    wait_group_events(1,copy_events);
}

__kernel void ComputeDensitites(__global const float* kernelSum, __global float* densities)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

	__local float local_den[MAX_LOCAL_SIZE];
	__local float local_sum[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[2];
	copy_events[0] = async_work_group_copy(local_den,densities+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_sum,kernelSum+offset,wg,0);
    wait_group_events(2,copy_events);

	local_den[i] = MASS * local_sum[i];
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(densities+offset,local_den,wg,0);
    wait_group_events(1,copy_events);
}

__kernel void ApplyNonPressureForces(__global const uint* neighbors, __global const float3* positions, __global const float3* velocities,  __global const float* densities, __global float3* forces)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory
	uint neighbourOffset = (i + offset) * K;

	__local float3 local_force[MAX_LOCAL_SIZE];
	__local float3 local_positions[MAX_LOCAL_SIZE];
	__local float3 local_velocities[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[3];
	copy_events[0] = async_work_group_copy(local_force,forces+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_positions,positions+offset,wg,0);
	copy_events[2] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
    wait_group_events(3,copy_events);

	//Apply Gravity
	local_force[i] = (float3)(0.0f,-9.81f,0.0f);
	barrier(CLK_LOCAL_MEM_FENCE);

	// //Apply Viscosity forces
	// for(int n = 0; n < K; n++)
	// {
	// 	uint neighborIndex = neighbors[n+neighbourOffset];

	// 	if(i+offset == neighborIndex || neighborIndex == UINT_MAX)
	// 		continue;


	// 	float dist = distance(local_positions[i],positions[neighborIndex]);
		
	// 	local_force[i] += VISCOSITY_COEFFICIENT * MASS2
	// 		* (velocities[neighborIndex] - local_velocities[i]) / densities[neighborIndex]
	// 		* SpikedKernelSecondDerivative(dist);
	// }
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(forces+offset,local_force,wg,0);
    wait_group_events(1,copy_events);

	//printf("point =%f,%f,%f\n", local_force[i].x,local_force[i].y,local_force[i].z);

}

__kernel void PredictPositionVelocity(__global const float3* positions,
										__global const float3* velocities,
										__global const float3* forces,
										__global const float3* pressureForces,
										__global float3* tempPositions,
										__global float3* tempVelocities,
										float timeStep)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

	__local float3 local_positions[MAX_LOCAL_SIZE];
	__local float3 local_velocities[MAX_LOCAL_SIZE];
	__local float3 local_force[MAX_LOCAL_SIZE];
	__local float3 local_pressureForces[MAX_LOCAL_SIZE];
	__local float3 local_tempVelocities[MAX_LOCAL_SIZE];
	__local float3 local_tempPositions[MAX_LOCAL_SIZE];
	
	//copy data from global to local
	event_t copy_events[4];
	copy_events[0] = async_work_group_copy(local_positions,positions+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
	copy_events[2] = async_work_group_copy(local_force,forces+offset,wg,0);
	copy_events[3] = async_work_group_copy(local_pressureForces,pressureForces+offset,wg,0);
    wait_group_events(4,copy_events);

	//predict velocity and pos
	local_tempVelocities[i] = local_velocities[i] 
								+ timeStep / MASS
								* (local_force[i] + local_pressureForces[i]);
	local_tempPositions[i] = local_positions[i] + timeStep * local_tempVelocities[i];
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(tempPositions+offset,local_tempPositions,wg,0);
	copy_events[1] = async_work_group_copy(tempVelocities+offset,local_tempVelocities,wg,0);
    wait_group_events(2,copy_events);
}

__kernel void CalculatePressure(__global const uint* neighbors,
									__global const float3* positions,
									__global const float3* velocities,
									__global float* pressures,
									__global float* densityErrors,
									__global float* estimateDensity,
									__global float3* pressureForces,
									float targetDensity,
									float deltaDensitity,
									float negativePressureScale)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int gid = get_group_id(0);
    int offset = gid * wg; //offset from global points memory
	uint neighbourOffset = (i + offset) * K;
	barrier(CLK_LOCAL_MEM_FENCE);

	__local float3 local_positions[MAX_LOCAL_SIZE];
	__local float3 local_velocities[MAX_LOCAL_SIZE];
	__local float local_pressure[MAX_LOCAL_SIZE];
	__local float local_densityErrors[MAX_LOCAL_SIZE];
	__local float local_estimateDensity[MAX_LOCAL_SIZE];
	__local float local_weightSums[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[5];
	copy_events[0] = async_work_group_copy(local_positions,positions+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
	copy_events[2] = async_work_group_copy(local_pressure,pressures+offset,wg,0);
	copy_events[3] = async_work_group_copy(local_densityErrors,densityErrors+offset,wg,0);
	copy_events[4] = async_work_group_copy(local_estimateDensity,estimateDensity+offset,wg,0);
    wait_group_events(5,copy_events);
	barrier(CLK_LOCAL_MEM_FENCE);

	//calculate pressure from densitiy error		
	for(int n = 0; n < K; n++)
	{
		uint neighborIndex = neighbors[n+neighbourOffset];

		if(neighborIndex == UINT_MAX)
			continue;


		float dist = distance(local_positions[i],positions[neighborIndex]);
		float kernelValue = SpikedKernelValue(dist);
		local_weightSums[i] += kernelValue;

		// printf("%d, %d, %d, %d, %f:%f:%f, %f:%f:%f, %f, %f, %f\n",
		// 		gid,
		// 		i,
		// 		i+offset,
		// 		neighborIndex,
		// 		local_positions[i].x, local_positions[i].y, local_positions[i].z,
		// 		positions[neighborIndex].x, positions[neighborIndex].y, positions[neighborIndex].z,
		// 		dist,
		// 		kernelValue,
		// 		local_weightSums[i]);
	}	
	local_weightSums[i] += SpikedKernelValue(0.0f);
	barrier(CLK_LOCAL_MEM_FENCE);		

	float density = MASS * (isnan(local_weightSums[i]) ? 0.0f: local_weightSums[i]);
	float densityError = (density - targetDensity);
	float pressure = deltaDensitity * densityError;

	if(pressure < 0.0f)
	{
		pressure *= negativePressureScale;
		densityError *= negativePressureScale;
	}
	else{
		//printf("Pressure %f\n", pressure);
	}

	local_pressure[i] += pressure;
	local_estimateDensity[i] = density;
	local_densityErrors[i] = densityError;
	barrier(CLK_LOCAL_MEM_FENCE);	
	
		

	copy_events[0] = async_work_group_copy(pressures+offset,local_pressure,wg,0);
	copy_events[1] = async_work_group_copy(densityErrors+offset,local_densityErrors,wg,0);
	copy_events[2] = async_work_group_copy(estimateDensity+offset,local_estimateDensity,wg,0);
    wait_group_events(3,copy_events);
}

__kernel void AccumlatePressureForces(__global const uint* neighbors,
									__global const float3* positions,
									__global const float3* velocities,  
									__global const float* estimateDensity,
									__global const float* pressures, 
									__global float3* pressureForces)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
	int gid = get_group_id(0);
    int offset = gid * wg; //offset from global points memory
	uint neighbourOffset = (i + offset) * K;

	__local float3 local_positions[MAX_LOCAL_SIZE];
	__local float3 local_velocities[MAX_LOCAL_SIZE];
	__local float local_estimateDensity[MAX_LOCAL_SIZE];
	__local float local_pressure[MAX_LOCAL_SIZE];
	__local float3 local_pressureForces[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[5];
	copy_events[0] = async_work_group_copy(local_positions,positions+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
	copy_events[2] = async_work_group_copy(local_estimateDensity,estimateDensity+offset,wg,0);
	copy_events[3] = async_work_group_copy(local_pressure,pressures+offset,wg,0);
	copy_events[4] = async_work_group_copy(local_pressureForces,pressureForces+offset,wg,0);
    wait_group_events(4,copy_events);
	

	for(int n = 0; n < K; n++)
	{
		uint neighborIndex = neighbors[n+neighbourOffset];

		if(neighborIndex == UINT_MAX)
			break;
		else if(i+offset == neighborIndex)
			continue;

		float dist = distance(local_positions[i],positions[neighborIndex]);
		if(dist > 0.0f)
		{
			float3 direction = (positions[neighborIndex] - local_positions[i]) / dist;
			
			float3 force = MASS2
				* (local_pressure[i] / (local_estimateDensity[i] * local_estimateDensity[i])
					+ pressures[neighborIndex] / (estimateDensity[neighborIndex] * estimateDensity[neighborIndex]))
				* SpikedKernelGradiant(dist, direction);

			local_pressureForces[i] -= force;


			// printf("%d, %d, %d, %f:%f:%f, %f:%f:%f, %f:%f:%f, %f, %f, %f, %f, %d\n",
			// 	gid,
			// 	i,
			// 	i+offset,
			// 	local_positions[i].x, local_positions[i].y, local_positions[i].z,
			// 	positions[neighborIndex].x, positions[neighborIndex].y, positions[neighborIndex].z,
			// 	local_pressureForces[i].x, local_pressureForces[i].y, local_pressureForces[i].z,
			// 	local_estimateDensity[i],
			// 	estimateDensity[neighborIndex],
			// 	local_pressure[i], 
			// 	pressures[neighborIndex],
			// 	neighborIndex);
		}
	}	
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(pressureForces+offset,local_pressureForces,wg,0);
    wait_group_events(1,copy_events);
}

__kernel void AccumlateForces(__global const float3* additionalForces, __global float3* forces)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

	__local float3 local_additionalForces[MAX_LOCAL_SIZE];
	__local float3 local_forces[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_events[2];
	copy_events[0] = async_work_group_copy(local_additionalForces,additionalForces+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_forces,forces+offset,wg,0);
    wait_group_events(2,copy_events);

	if(fast_length(local_additionalForces[i]) >= 0.00001f)
		local_forces[i] += local_additionalForces[i];
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(forces+offset,local_forces,wg,0);
    wait_group_events(1,copy_events);
}

__kernel void integrate(__global const float3* forces, __global float3* positions, __global float3* velocities, float timeStep)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

	__local float3 local_forces[MAX_LOCAL_SIZE];
	__local float3 local_positions[MAX_LOCAL_SIZE];
	__local float3 local_velocities[MAX_LOCAL_SIZE];

	event_t copy_events[3];
	copy_events[0] = async_work_group_copy(local_forces,forces+offset,wg,0);
	copy_events[1] = async_work_group_copy(local_positions,positions+offset,wg,0);
	copy_events[2] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
    wait_group_events(3,copy_events);

	local_velocities[i] +=  timeStep * local_forces[i];
	local_positions[i] += timeStep * local_velocities[i];
	barrier(CLK_LOCAL_MEM_FENCE);

	copy_events[0] = async_work_group_copy(positions+offset,local_positions,wg,0);
	copy_events[1] = async_work_group_copy(velocities+offset,local_velocities,wg,0);
    wait_group_events(2,copy_events);
}