#define MAX_LOCAL_SIZE 256

typedef struct _GpuHashPoint
{
	uint Hash;
	uint SourceIndex;
} GpuHashPoint;


float4 getCellPoint(float4 point, float4 lowerBound, float4 upperBound, int subdivisions)
{
    return ((point - lowerBound)/(upperBound-lowerBound)) * subdivisions;
}

uint getHash(float4 point, float4 lowerBound, float4 upperBound, int subdivisions)
{
    float4 cell = getCellPoint(point,lowerBound,upperBound,subdivisions);
    float subdiv2 = subdivisions * subdivisions;

    return floor(cell.x) * subdiv2 + floor(cell.y) * subdivisions + floor(cell.z);
}

/// 0: __global GpuHashPoint* points: points to sort in a way that points from the same cell will be placed sequentially in memory.
/// 1: __global uint* cellStartIndex: The start index in the sorted points for the points contained in the cell
/// 2: __global uint* cellSize: The size (amount) of points contained in the cell
void sort(__global GpuHashPoint* points, __global uint* cellStartIndex, __global uint* cellSize)
{
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    barrier(CLK_LOCAL_MEM_FENCE);
}


__kernel void GetStartSize(const __global GpuHashPoint* points, __global uint* cellStartIndex, __global uint* cellSize)
{
    __local GpuHashPoint localPoints[MAX_LOCAL_SIZE];
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    localPoints[i] = points[i+offset];
    barrier(CLK_LOCAL_MEM_FENCE);

    // // event_t events[3];
    // event_t events[3];
    // events[0] = async_work_group_copy( (__local uint2*)localPoints, (__global uint2*)points+offset, wg, 0);
    // events[1] = async_work_group_copy(startIndex, cellStartIndex+offset,wg,0);
    // events[2] = async_work_group_copy(size, cellSize+offset,wg,0);
    // wait_group_events(3,events);

    int cellIndex = localPoints[i].Hash;
    atomic_min(&cellStartIndex[cellIndex], i+offset);
    barrier(CLK_GLOBAL_MEM_FENCE);
}

/// 0: __global const float4* points: Input points
/// 1: __global GpuHashPoint* sortedPoints: sorted point in a way that points from the same cell will be placed sequentially in memory.
/// 2: __global uint* cellStartIndex: The start index in the sorted points for the points contained in the cell
/// 3: __global uint* cellSize: The size (amount) of points contained in the cell
/// 4: lowerBound: lower bound of the AABB grid (used to hash the point)
/// 5: upperBound: upper bound of the AABB grid (used to hash the point)
/// 6: subdivisions: number of subdivisions on each dimension of the grid
__kernel void Build(__global const float4* points, __global GpuHashPoint* sortedPoints, __global uint* cellStartIndex, __global uint* cellSize, float4 lowerBound, float4 upperBound, int subdivisions)
{
    __local float4 blockA[MAX_LOCAL_SIZE];
    __local GpuHashPoint localPoints[MAX_LOCAL_SIZE];

    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    //copy data from global to local
    event_t evt = async_work_group_copy(blockA,points+offset,wg,0);
    wait_group_events(1,&evt);

    localPoints[i].Hash = getHash(blockA[i],lowerBound,upperBound,subdivisions);
    localPoints[i].SourceIndex = i+offset;
    barrier(CLK_LOCAL_MEM_FENCE);

    //copy local points into sorted (Note: the points are not currently sorted at this stage)
    // __global char* dst = (global char *)sortedPoints+offset;
    // local char* src = (local char *)localPoints;
    // size_t size = sizeof(GpuHashPoint) * wg;
    // event_t evt1 = async_work_group_copy(dst,src,size,0);
    event_t evt1 = async_work_group_copy((__global uint2*)sortedPoints+offset, (__local uint2*)localPoints, wg, 0);
    wait_group_events(1,&evt1);

    //Since we have all the points p∈P hashed through the getHash(p) function,
    //we sort them in a way that points from the same cell will be placed sequentially in memory.
    //Radix sort would be ideal here as values are unsigned ints
}