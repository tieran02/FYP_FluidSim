#define MAX_LOCAL_SIZE 256
#define MAX_NEIGHBOR_COUNT 256

__kernel void calculateDistances(__global const float4* points, __global uint* neighbors, float4 queryPoint, float radius2)
{
    __local float4 blockA[MAX_LOCAL_SIZE];
    __local uint blockAnearestIndex[MAX_LOCAL_SIZE];

    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    //copy data from global to local
    event_t evt = async_work_group_copy(blockA,points+offset,wg,0);
    wait_group_events(1,&evt);

    for(int j=get_local_id(0); j < wg;j++)
    {
        for(int k=0;k < wg; k++)
        {
            if(j==k) continue;

            float3 diff = blockA[k].xyz - blockA[j].xyz;
            float distance2 = dot(diff, diff);
            blockA[j].w = distance2;
            if (distance2 <= radius2)
            {
                blockAnearestIndex[j] = j + offset;
            }
            else
            {
                blockAnearestIndex[j] = UINT_MAX;
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    evt = async_work_group_copy(neighbors+offset, blockAnearestIndex, wg, 0);
    wait_group_events(1, &evt);

    //Now sort blockAnearestIndex
    //Copy first 256 over to neighbours
}

//Find the neighest neighbours of the query point (Note this kernel method should only be used to get neighbors of a single point only)
//Do not call it for every point from the CPU as it requires the GPU to write to the CPU each time, making it really slow for all particles neighbours
__kernel void FindNearestNeighbors(__global const float4* points, __global uint* neighbors, float4 queryPoint, uint size, uint maxNeighborCount, float radius)
{
    __local float4 blockA[MAX_LOCAL_SIZE];

    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory
    float radius2 = radius * radius;

    calculateDistances(points, neighbors, queryPoint, radius2);

    barrier(CLK_LOCAL_MEM_FENCE);
}


//__kernel void FindNearestNeighbors(__global const float4* points, __global uint* neighbors, float4 queryPoint, uint size, uint maxNeighborCount, float radius)
//{
//    __local float4 blockA[MAX_LOCAL_SIZE];
//
//    int i = get_local_id(0); //index of workgroup
//    int wg = get_local_size(0); //get workgroup size
//    int offset = get_group_id(0) * wg; //offset from global points memory
//
//    //copy data from global to local
//    event_t evt = async_work_group_copy(blockA, points + offset, wg, 0);
//    wait_group_events(1, &evt);
//
//    float radius2 = radius * radius;
//
//    for (int j = get_local_id(0); j < wg; j++)
//    {
//        calculateDistances(points, blockA[j], radius2);
//    }
//    barrier(CLK_LOCAL_MEM_FENCE);
//}
