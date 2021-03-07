#define MAX_LOCAL_SIZE 256

__kernel void FindNearestNeighbors(__global const float4* points, __global uint* neighbors,float3 queryPoint, uint size, uint maxNeighborCount, float radius)
{
    __local float4 blockA[MAX_LOCAL_SIZE];
    __local uint blockAnearestIndex[MAX_LOCAL_SIZE];

    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    //copy data from global to local
    event_t evt = async_work_group_copy(blockA,points+offset,wg,0);
    wait_group_events(1,&evt);
    float radius2 = radius * radius;

    for(int j=get_local_id(0); j < wg;j++)
    {
        for(int k=0;k < wg; k++)
        {
            if(j==k) continue;

            float4 diff = blockA[k] - blockA[j];
            float distance2 = dot(diff,diff);
            blockA[j].w = distance2;
            if(distance2 < radius2)
                blockAnearestIndex[j] = j+offset;
            else
                blockAnearestIndex[j] = UINT_MAX;
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    evt = async_work_group_copy(neighbors+offset,blockAnearestIndex,wg,0);
    wait_group_events(1,&evt);

    //due to the float3 points being 16 bytes and not 12 in opencl 1/4 of the distances are cut off
    //aligh glm vec3 to 16 bytes instead
}