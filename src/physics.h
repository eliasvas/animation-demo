#ifndef PHYSICS_H
#define PHYSICS_H
typedef struct AABB
{
    vec3 min;
    vec3 max;
}AABB;

typedef struct AABB2D
{
    vec2 min;
    vec2 max;
}AABB2D;

static AABB2D
aabb2d(vec2 min, vec2 max)
{
    AABB2D res;
    res.min = min;
    res.max = max;
    return res;
}

static AABB
aabb(vec3 min, vec3 max)
{
    AABB res;
    res.min = min;
    res.max = max;
    return res;
}


static b32 
AABB2DvsAABB2D(AABB2D a, AABB2D b)
{
    //search for a separating axis
    if (a.max.x < b.min.x || a.min.x > b.max.x)return 0;
    if (a.max.y < b.min.y || a.min.y > b.max.y)return 0;

    //if not found, there must be a collision
    return 1;
}

static b32 
AABB2DvsAABB(AABB a, AABB b)
{
    //search for a separating axis
    if (a.max.x < b.min.x || a.min.x > b.max.x)return 0;
    if (a.max.y < b.min.y || a.min.y > b.max.y)return 0;
    if (a.max.z < b.min.z || a.min.z > b.max.z)return 0;

    //if not found, there must be a collision
    return 1;
}

typedef struct Circle2D
{
    f32 radius;
    vec2 position;
}Circle2D;

static f32 dist2d(vec2 a, vec2 b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}
static f32 dist(vec3 a, vec3 b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}


static b32 Circle2DvsCircle2D(Circle2D c1, Circle2D c2)
{
    f32 r = c1.radius + c2.radius;
    return (r < dist2d(c1.position, c2.position));
}

typedef struct Circle
{
    f32 radius;
    vec3 position;
}Circle;

static b32 CirclevsCircle(Circle c1, Circle c2)
{
    f32 r = c1.radius + c2.radius;
    return (r < dist(c1.position, c2.position));
}

typedef struct Ray
{
    vec3 origin;
    vec3 direction;
}Ray;

static b32 
RayvsCircle(Ray r1, Circle c1)
{
    return 0;    
}

#endif
