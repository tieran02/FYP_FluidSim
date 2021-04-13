#define MAX_LOCAL_SIZE 256

__constant float RestitutionCoefficient  = 0.2f;
__constant float frictionCoeffient = 0.1f;

typedef struct _collisionPoint
{
	float3 Point;
    float3 Normal;
} CollisionPoint;

bool GetIntersection(float3 point, float3 velocity, float3 lowerBound, float3 upperBound, float radius, CollisionPoint* collisionPoint)
{
    float3 boxSize = (upperBound-lowerBound) * 0.5f;

    float3 boxWorldTransform = (float3)(0.0f,0.0f,0.0f);
	float3 sphereWorldTransform = point + velocity;
    float3 delta = sphereWorldTransform - boxWorldTransform;
    float3 closestPointOnBox = clamp(delta, -boxSize+radius, boxSize-radius);

    float3 localPoint = delta - closestPointOnBox;
	float distance = length(localPoint);

    if(fabs(distance) >= FLT_EPSILON) // colliding
	{
		collisionPoint->Normal = -normalize(localPoint);
		collisionPoint->Point = closestPointOnBox;
        return true;
	}

    return false;
}

__kernel void SphereAABBCollisions(__global float3* positions, __global float3* velocities, float3 lowerBound, float3 upperBound, float radius, float timeStep)
{
    int i = get_local_id(0); //index of workgroup
    int wg = get_local_size(0); //get workgroup size
    int offset = get_group_id(0) * wg; //offset from global points memory

    __local float3 local_positions[MAX_LOCAL_SIZE];
    __local float3 local_velocities[MAX_LOCAL_SIZE];

	event_t copy_events[2];
	copy_events[0] = async_work_group_copy(local_positions,positions+offset,wg,0);
    copy_events[1] = async_work_group_copy(local_velocities,velocities+offset,wg,0);
    wait_group_events(2,copy_events);
    barrier(CLK_LOCAL_MEM_FENCE);

    CollisionPoint collisionPoint;
    if(GetIntersection(local_positions[i], local_velocities[i] * timeStep, lowerBound, upperBound, radius, &collisionPoint))
    {
        float normalDotRelativeVelocity = dot(collisionPoint.Normal, local_velocities[i]);
        float3 relativeVelocityNormal = normalDotRelativeVelocity * collisionPoint.Normal;
		float3 relativeVelocityT = local_velocities[i] - relativeVelocityNormal;

        // Check if the velocity is facing opposite direction of the surface
        if (normalDotRelativeVelocity < 0.0f)
        {
            float3 deltaRelativeVelocityNormal = (-RestitutionCoefficient - 1.0f) * relativeVelocityNormal;
            relativeVelocityNormal *= -RestitutionCoefficient;

            // Apply friction to the tangential component of the velocity
            if (length(relativeVelocityT) > 0.0f)
            {
                float frictionScale = max(1.0f - frictionCoeffient *
                    length(deltaRelativeVelocityNormal) / length(relativeVelocityT), 0.0f);
                relativeVelocityT *= frictionScale;
            }

            local_velocities[i] = relativeVelocityNormal + relativeVelocityT;
        }
        local_positions[i] = collisionPoint.Point;
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    

    copy_events[0] = async_work_group_copy(positions+offset,local_positions,wg,0);
    copy_events[1] = async_work_group_copy(velocities+offset,local_velocities,wg,0);
    wait_group_events(2,copy_events);
    
}