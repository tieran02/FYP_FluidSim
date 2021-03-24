#define MAX_LOCAL_SIZE 256

__kernel void SphereAABBCollisions(__global float3* positions, float3 lowerBound, float3 upperBound, float radius)
{
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    __local float3 local_positions[MAX_LOCAL_SIZE];

	event_t copy_events[1];
	copy_events[0] = async_work_group_copy(local_positions,positions+offset,wg,0);
    wait_group_events(1,copy_events);

    float3 boxSize = (upperBound-lowerBound) * 0.5f;
    float3 closestPointOnBox = clamp(local_positions[i], -boxSize+radius, boxSize-radius);
    float3 localPoint = local_positions[i] - closestPointOnBox;

    float dist = fast_length(localPoint);

    if(dist > 0.0f)
    {
        float3 intersectionNormal = -normalize(localPoint);
        local_positions[i] = clamp(local_positions[i] + (-intersectionNormal * radius),lowerBound+radius,upperBound-radius);
    }

    copy_events[0] = async_work_group_copy(positions+offset,local_positions,wg,0);
    wait_group_events(1,copy_events);
}