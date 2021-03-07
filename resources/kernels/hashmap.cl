typedef struct st_hash_pair
{
    int key;
    int points[256];
    int pointsInBucket;
} hash_pair;


uint getHashKey(__global const float3* point, float radius, int numberOfBuckets)
{
    int ix = floor(point->x / radius);
    int iy = floor(point->y / radius);
    int iz = floor(point->z / radius);

    int hash = ((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % numberOfBuckets;
    //uint hash = ((uint)(point->x * 73856093) ^ (uint)(point->y * 19349663) ^ (uint)(point->z * 83492791)) % gridSize;
    return hash;
}

__kernel void build_hashmap(__global const float3* points, __global hash_pair* pairs, const int size, float radius)
{
    const int numberOfBuckets = 125; //80% of max point count (size/ 256*0.8)^3

    // Get the index of the current element to be processed
    int i = get_global_id(0);

    long hashkey = getHashKey(&points[i], radius , numberOfBuckets);
    int row = hashkey;

    pairs[i].key = hashkey;
    pairs[i].points[pairs[i].pointsInBucket++] = i;
}

__kernel void hash(__global const float3* points, const int size)
{
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    
    int offset = get_group_id(0) * wg;

    
}