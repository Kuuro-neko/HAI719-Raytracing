#include "KDTree.hpp"
#include <algorithm>
#include "Functions.h"
#include "Constants.h"

class KDTree::Node {
public:
    AABB aabb;
    Node *left;
    Node *right;
    const std::vector<MeshVertex>& vertices;

    std::vector<MeshTriangle> triangles; // Leaf
    AABBCuttingPlane plane; // Node

    Node(const AABB& aabb, const std::vector<MeshVertex>& vertices) : aabb(aabb), vertices(vertices), left(nullptr), right(nullptr) {

    }
    ~Node() {
        if (left) delete left;
        if (right) delete right;
        if (leaf()) triangles.clear();
    }

    bool leaf() const { return left == nullptr && right == nullptr;}

    void addTriangle(const MeshTriangle& triangle) {
        triangles.push_back(triangle);
    }

    inline RayTriangleIntersection intersect(const Ray& ray) const {
        if (!aabb.intersects(ray)) return RayTriangleIntersection();
        if (leaf()) {
            if (triangles.size() == 0) return RayTriangleIntersection();
            RayTriangleIntersection closestIntersection;
            closestIntersection.t = FLT_MAX;
            for (unsigned int i = 0; i < triangles.size(); i++) {
                Triangle triangle(vertices[triangles[i][0]].position * TRIANGLE_SCALING,
                                  vertices[triangles[i][1]].position * TRIANGLE_SCALING,
                                  vertices[triangles[i][2]].position * TRIANGLE_SCALING);
                RayTriangleIntersection intersection = triangle.getIntersection(ray);
                if (intersection.t < closestIntersection.t) {
                    closestIntersection = intersection;
                    closestIntersection.tIndex = triangles[i][3];
                }
            }
            return closestIntersection;
        } else {
            RayTriangleIntersection leftIntersection, rightIntersection;

            if (left) {
                leftIntersection = left->intersect(ray);
            } else {
                leftIntersection.t = FLT_MAX;
            }

            if (right) {
                rightIntersection = right->intersect(ray);
            } else {
                rightIntersection.t = FLT_MAX;
            }

            if (leftIntersection.t < rightIntersection.t) {
                return leftIntersection;
            } else {
                return rightIntersection;
            }
        }
    }
};

KDTree::KDTree(const std::vector<MeshTriangle>& triangles, const AABB& aabb, const std::vector<MeshVertex>& vertices) : vertices(vertices), aabb(aabb) {
    root = buildTree(triangles, aabb, 0);
}

KDTree::~KDTree() {
    if (root) delete root;
}

RayTriangleIntersection KDTree::intersect(const Ray& ray) const {
    if (!root) return RayTriangleIntersection();
    if (!aabb.intersects(ray)) return RayTriangleIntersection();
    RayTriangleIntersection intersection = root->intersect(ray);
    return intersection;
}

AABBCuttingPlane KDTree::cut(const std::vector<MeshTriangle>& triangles, int depth) const {
    AABBCuttingPlane plane = AABBCuttingPlane(depth % 3, 0);

    std::vector<float> min_list;
    for (unsigned int i = 0; i < triangles.size(); i++) {
        AABB aabb = Triangle(vertices[triangles[i][0]].position, vertices[triangles[i][1]].position, vertices[triangles[i][2]].position).getAABB();
        min_list.push_back( aabb.p0[plane.axis] );
    }
    std::sort(min_list.begin(), min_list.end());
    plane.position = min_list[min_list.size() / 2] + EPSILON;
    return plane;
}

KDTree::Node* KDTree::buildTree(const std::vector<MeshTriangle>& triangles, const AABB& aabb, unsigned int depth) {
    if (triangles.size() == 0 || depth > KDTREE_MAX_DEPTH) {
        return nullptr;
    }

    KDTree::Node* ret = new KDTree::Node(aabb, vertices);

    int nTris = triangles.size();
    if (nTris <= KDTREE_TRIANGLES_PER_LEAF) {
        for (const auto& triangle : triangles) ret->addTriangle(triangle);
        return ret;
    }
    std::pair<AABB, AABB> aabbs;
    AABBCuttingPlane bestPlane;
    float min_cost = FLT_MAX;

    bestPlane = cut(triangles, depth);

    try {
        aabbs = aabb.split(bestPlane);
    } catch (const std::exception& e) {
        for (const auto& triangle : triangles) ret->addTriangle(triangle);
        return ret;
    }

    ret->plane = bestPlane;

    std::vector<MeshTriangle> leftTriangles;
    std::vector<MeshTriangle> rightTriangles;

    for (const auto& triangle : triangles) {
        AABB triangleAABB = Triangle(vertices[triangle[0]].position, vertices[triangle[1]].position, vertices[triangle[2]].position).getAABB();
        if (triangleAABB.p1[bestPlane.axis] <= bestPlane.position - EPSILON) {
            leftTriangles.push_back(triangle);
        } else if (triangleAABB.p0[bestPlane.axis] >= bestPlane.position + EPSILON) {
            rightTriangles.push_back(triangle);
        } else {
            leftTriangles.push_back(triangle);
            rightTriangles.push_back(triangle);
        }
    }

    if (leftTriangles.size() == rightTriangles.size()) {
        for (const auto& triangle : triangles) ret->addTriangle(triangle);
        return ret;
    }

    ret->left = buildTree(leftTriangles, aabbs.first, depth + 1);
    ret->right = buildTree(rightTriangles, aabbs.second, depth + 1);

    return ret;
}

void KDTree::draw() const {
    if (root) {
        GLfloat material_color[4] = {1.0, 1.0, 1.0, 1.0};
        GLfloat material_specular[4] = {1.0, 1.0, 1.0, 1.0};
        GLfloat material_ambient[4] = {1.0, 1.0, 1.0, 1.0};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 16);

        glEnableClientState(GL_VERTEX_ARRAY);
        
        std::vector<Vec3> boxVertices;
        boxVertices.push_back(Vec3(aabb.p0[0], aabb.p0[1], aabb.p0[2]));
        boxVertices.push_back(Vec3(aabb.p1[0], aabb.p0[1], aabb.p0[2]));
        boxVertices.push_back(Vec3(aabb.p1[0], aabb.p1[1], aabb.p0[2]));
        boxVertices.push_back(Vec3(aabb.p0[0], aabb.p1[1], aabb.p0[2]));
        boxVertices.push_back(Vec3(aabb.p0[0], aabb.p0[1], aabb.p1[2]));
        boxVertices.push_back(Vec3(aabb.p1[0], aabb.p0[1], aabb.p1[2]));
        boxVertices.push_back(Vec3(aabb.p1[0], aabb.p1[1], aabb.p1[2]));
        boxVertices.push_back(Vec3(aabb.p0[0], aabb.p1[1], aabb.p1[2]));

        // draw only the edges
        unsigned int boxIndices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        glVertexPointer(3, GL_FLOAT, 0, boxVertices.data());
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boxIndices);

    }
}