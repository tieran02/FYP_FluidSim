#define MAX_LOCAL_SIZE 256
#define K 256

__constant float KERNEL_RADIUS  = 0.4f;
__constant float KERNEL_RADIUS2 = 0.4f * 0.4f;
__constant float KERNEL_RADIUS3 = 0.4f * 0.4f * 0.4f;
__constant float KERNEL_RADIUS4 = 0.4f * 0.4f * 0.4f * 0.4f;
__constant float KERNEL_RADIUS5 = 0.4f * 0.4f * 0.4f * 0.4f * 0.4f;
__constant float MASS = 1.0f;

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

__kernel void SumOfKernel(__global const float3* positions, __global const uint* neighbors, __global float* kernelSum)
{
	int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory
	int neighbourOffset = i + offset * K;

	__local float3 local_pos[MAX_LOCAL_SIZE];

	//copy data from global to local
	event_t copy_event = async_work_group_copy(local_pos,positions+offset,wg,0);
    wait_group_events(1,&copy_event);

	float sum = 0.0f;
	for(int n = 0; n < K; n++)
	{
		uint neighborIndex = neighbors[n+neighbourOffset];

		if(i+offset == neighborIndex || neighborIndex == UINT_MAX)
			continue;

		float dist = fast_distance(local_pos[i],positions[neighborIndex]);
		float weight = SmoothedKernelValue(dist);
		sum += weight;
	}
	kernelSum[i+offset] = sum;
	barrier(CLK_GLOBAL_MEM_FENCE);
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