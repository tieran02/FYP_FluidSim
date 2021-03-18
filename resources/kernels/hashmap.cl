#define MAX_LOCAL_SIZE 256
#define MAX_NEIGHBORS 256

typedef struct _GpuHashPoint
{
	uint Hash;
	uint SourceIndex;
} GpuHashPoint;


bool pointInBounds(float4 point, float4 lowerBound, float4 upperBound)
{
    	return (point.x >= lowerBound.x && point.x <= upperBound.x) &&
		(point.y >= lowerBound.y && point.y <= upperBound.y) &&
		(point.z >= lowerBound.z && point.z <= upperBound.z);
}

float4 getCellPoint(float4 point, float4 lowerBound, float4 upperBound, int subdivisions)
{
    float4 cellPoint = ((point - lowerBound)/(upperBound-lowerBound)) * subdivisions;
    cellPoint.x = floor(cellPoint.x);
    cellPoint.y = floor(cellPoint.y);
    cellPoint.z = floor(cellPoint.z);
    return cellPoint;
}

uint getHash(float4 point, float4 lowerBound, float4 upperBound, int subdivisions)
{
    float4 cell = getCellPoint(point,lowerBound,upperBound,subdivisions);
    float subdiv2 = subdivisions * subdivisions;

    return cell.x * subdiv2 + cell.y * subdivisions + cell.z;
}

uint getHashFromCellPoint(float4 cellPoint, float4 lowerBound, float4 upperBound, int subdivisions)
{
    float subdiv2 = subdivisions * subdivisions;
    return cellPoint.x * subdiv2 + cellPoint.y * subdivisions + cellPoint.z;
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
    //TODO to speed this up we can use local memory and remove the need of the atomic methods
    //Atomics are slow due to accessing global memory
    
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    int cellIndex = points[i+offset].Hash;
    atomic_min(&cellStartIndex[cellIndex], i+offset);
    atomic_inc(&cellSize[cellIndex]);
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
    //Currently sorting on the host (CPU) to test the data structure, we can speed it up alot by doing the sorting on the GPU
}

/// 0: __global const float4* points: Input points
/// 1: __global const GpuHashPoint* sortedPoints: sorted points in a way that points from the same cell will be placed sequentially in memory.
/// 2: __global const uint* cellStartIndex: The start index in the sorted points for the points contained in the cell
/// 3: __global const uint* cellSize: The size (amount) of points contained in the cell
/// 4: __global uint* neighbours: An array containing all the neighbouring points within radius (MAX neighbours defined as 256)
/// 5: lowerBound: lower bound of the AABB grid (used to hash the point)
/// 6: upperBound: upper bound of the AABB grid (used to hash the point)
/// 7: subdivisions: number of subdivisions on each dimension of the grid
/// 8: float4 queryPoint : the point to find the nighbors for
/// 9: float radius : radius to find neighbors
//TODO querypoints should be a global buffer, then have a local worj group of one and global work group size of the query points (this way we can have the nn computed in parallel)
//TODO store largest distance pointer, if neighborCount == MAX_NEIGHBORS, check if the new distance is less than largest and replace if true, get new largest index
__kernel void GetNearestNeighbours(__global const float4* points, __global const GpuHashPoint* sortedPoints, __global const uint* cellStartIndex, __global const uint* cellSize, __global uint* neighbours, float4 lowerBound, float4 upperBound, int subdivisions, float4 queryPoint, float radius)
{
    //This should only be called by a single local thread

    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    float4 bucketSize = (upperBound-lowerBound)/subdivisions;
    float4 A = (float4)(bucketSize.x ,0.0f, 0.0f, 0.0f);
    float4 B = (float4)(0.0f, bucketSize.y ,0.0f, 0.0f);
    float4 C = (float4)(0.0f, 0.0f, bucketSize.z, 0.0f);

    int searchSize = ceil(radius/bucketSize.x) + 2;

    float4 queryCellPoint = getCellPoint(queryPoint,lowerBound,upperBound,subdivisions);
    queryCellPoint.x = floor(queryCellPoint.x) * bucketSize.x;
    queryCellPoint.y = floor(queryCellPoint.y) * bucketSize.y;
    queryCellPoint.z = floor(queryCellPoint.z) * bucketSize.z;

    float4 topLeft = queryCellPoint - (bucketSize * searchSize);
    float radius2 = radius * radius;
    uint localNeighbors[MAX_NEIGHBORS];
    uint neighborCount = 0;

    for(int x = 0; x <= searchSize; x++)
    {
        if(neighborCount == MAX_NEIGHBORS)
            break;

        for(int y = 0; y <= searchSize; y++)
        {
            if(neighborCount == MAX_NEIGHBORS)
                break;

            for(int z = 0; z <= searchSize; z++)
            {
                if(neighborCount == MAX_NEIGHBORS)
                    break;

                float4 point = (float4)(topLeft + A * (float)x + B * (float)y + C * (float)z);
                if(!pointInBounds(point,lowerBound,upperBound))
                    continue;

                uint cellHash = getHash(point,lowerBound,upperBound,subdivisions);

                // printf("point =%f,%f,%f hash=%d \n", point.x,point.y,point.z,cellHash);
                //check bounds of hash
                if(cellHash >= (subdivisions * subdivisions * subdivisions))
                    continue;

                //printf("%d \n", cellHash);

                uint startIndex = cellStartIndex[cellHash];
                uint cellPointCount = cellSize[cellHash];
                if(cellPointCount == 0)
                    continue;

                //printf("hash = %d startIndex =%d cellPointCount = %d neighborCount = %d\n", cellHash,startIndex,cellPointCount,neighborCount);

                //now loop through all the points in the cell and caluate the distance
                for(int s = 0; s < cellPointCount; s++)
                {
                    if(neighborCount >= MAX_NEIGHBORS)
                        break;

                    float4 cellPoint = points[sortedPoints[startIndex+s].SourceIndex];
                    float dist = fast_distance(queryPoint, cellPoint);
                    //printf("queryPoint =%f,%f,%f  cellPoint=%f,%f,%f  distance=%f \n", queryPoint.x,queryPoint.y,queryPoint.z,cellPoint.x,cellPoint.y,cellPoint.z,dist);
                    //printf("neighbour visited index=%d with distance=%f \n", sortedPoints[startIndex+s].SourceIndex, dist);
                    if(dist <= radius)
                    {
                        //printf("neighbour added index=%d with distance=%f \n", sortedPoints[startIndex+s].SourceIndex, dist);
                        localNeighbors[neighborCount++] = sortedPoints[startIndex+s].SourceIndex;
                        //atomic_inc(&neighborCount);
                    }
                }
            }
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    for(int n = 0; n < neighborCount; n++)
    {
        neighbours[n] = localNeighbors[n];
    }
    barrier(CLK_GLOBAL_MEM_FENCE);
}

__kernel void KNN(__global const float4* points,
    __global const GpuHashPoint* sortedPoints,
    __global const uint* cellStartIndex,  
    __global const uint* cellSize,
    __global const float4* queryPoints, 
    __global uint* neighbours,
    float4 lowerBound, 
    float4 upperBound, 
    int subdivisions, 
    int K,
    float radius)
{
    int gi = get_global_id(0); //index of workgroup
    int globalOffset = gi * K;

    float radiusPerBucket = length(upperBound - lowerBound) / subdivisions;
    int rangeRadius = 0;
    int t = 0;
    //get number of point in queryPointCell
    float4 queryPoint = queryPoints[gi];
    if(!pointInBounds(queryPoint,lowerBound,upperBound))
        return;

    float4 queryCellPoint = getCellPoint(queryPoint, lowerBound,upperBound,subdivisions);
    uint queryPointHash = getHash(queryPoint, lowerBound,upperBound,subdivisions);
    
    uint pHist = cellSize[queryPointHash];
    __local uint local_neighbors[MAX_NEIGHBORS];
    __local float local_distances[MAX_NEIGHBORS];
    uint vistedCellHashes[MAX_NEIGHBORS*2];
    uint vistedCellCount = 0;
    uint neighborCount = 0;

    while(pHist < K || t <= 2)
    {
        if(rangeRadius >= subdivisions)
            break;

        //check if the cell distance is too far away from point
        if((rangeRadius*radiusPerBucket)/2 > radius)
            break;

        rangeRadius++;

        for(int cellX = queryCellPoint.x-rangeRadius; cellX < queryCellPoint.x+rangeRadius; cellX++)
        {
            for(int cellY = queryCellPoint.y-rangeRadius; cellY < queryCellPoint.y+rangeRadius; cellY++)
            {
                for(int cellZ = queryCellPoint.z-rangeRadius; cellZ < queryCellPoint.z+rangeRadius; cellZ++)
                {

                    //check for out of bounds
                    if(cellX < 0 || cellX > subdivisions || cellY < 0 || cellY > subdivisions || cellZ < 0 || cellZ > subdivisions) 
                        continue;
                    

                    // if(abs(cellX) < rangeRadius+1 || abs(cellY) <= rangeRadius+1 || abs(cellZ) <= rangeRadius+1)
                    //     continue; //ignore inside area as we have already visted before.

                    uint cellPointHash = getHashFromCellPoint((float4)(cellX,cellY,cellZ,1.0f), lowerBound,upperBound,subdivisions);

                    //TODO optimise visted cell points, this is not optimal at all as it also has a limit on the number of points visted due to private memory restrictions
                    bool vistedCell = false;
                    if(vistedCellCount < MAX_NEIGHBORS*2)
                    {
                        for(int vistedIndex = 0; vistedIndex < vistedCellCount; vistedIndex++)
                        {
                            if(vistedCellHashes[vistedIndex] == cellPointHash)
                            {
                                vistedCell = true;
                                break;
                            }
                        }
                        if(vistedCell) continue;
                    }
                    else{
                        continue;
                    }

                    //printf("visted Cell: %d, %d, %d search radius:%d\n", cellX,cellY,cellZ, rangeRadius);
                    vistedCellHashes[vistedCellCount++] = cellPointHash;
                    uint pointWithinCell = cellSize[cellPointHash];
                    if(pointWithinCell > 0)
                    {
                        for(int cellIndex = 0; cellIndex < pointWithinCell; cellIndex++)
                        {
                            float4 point = points[sortedPoints[cellStartIndex[cellPointHash] + cellIndex].SourceIndex];
                            float dist = fast_distance(queryPoint, point);
                            if(dist > radius)
                                continue;

                            if(neighborCount < K)
                            {
                                local_distances[neighborCount] = dist;
                                local_neighbors[neighborCount++] = sortedPoints[cellStartIndex[cellPointHash] + cellIndex].SourceIndex;
                            }
                            // else
                            // {
                            //     //replace largest value with new value
                            //     uint largestIndex = 0;
                            //     float largestDistance = FLT_MIN;
                            //     for(int kIndex = 0; kIndex < K; kIndex++)
                            //     {
                            //         if(local_distances[kIndex] > largestDistance)
                            //         {
                            //             largestIndex = kIndex;
                            //             largestDistance = local_distances[kIndex];
                            //         }
                            //     }
                            //     local_distances[largestIndex] = dist;
                            //     local_neighbors[largestIndex] = sortedPoints[cellStartIndex[cellPointHash] + cellIndex].SourceIndex;
                            // }
                        }
                    }
                    pHist += pointWithinCell;
                } 
            }
        }

        if(pHist >= rangeRadius)
        {
            t++;
        }
    }

    // for(int n = 0; n < neighborCount ;n++)
    // {
    //     printf("Neighbors %d\n", local_neighbors[n]);
    //     //neighbours[n] = local_neighbors[n];
    // }

    event_t evt1 = async_work_group_copy(neighbours+(get_global_id(0)*K), local_neighbors, neighborCount, 0);
    wait_group_events(1,&evt1);
}