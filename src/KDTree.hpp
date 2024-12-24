#ifndef KDTREE_H
#define KDTREE_H

#include <vector>
#include "Mesh.h"
#include "AABB.h"
#include "Ray.h"
#include "Triangle.h"
#include "Vec3.h"

#include "Constants.h"

struct MeshVertex;
struct MeshTriangle;

class KDTree {
public:
    struct Node;

    const std::vector<MeshVertex>& vertices;

    Node* root;

    AABB aabb;

    KDTree(const std::vector<MeshTriangle>& triangles, const AABB& aabb, const std::vector<MeshVertex>& vertices);
    ~KDTree();

    RayTriangleIntersection intersect(const Ray& ray) const;
    void draw() const;

private:
    Node* buildTree(const std::vector<MeshTriangle>& triangles, const AABB& aabb, unsigned int depth);
    AABBCuttingPlane cut(const std::vector<MeshTriangle>& triangles, int depth) const;
};

#endif // KDTREE_H