#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Loads colored or uncolored mesh from OFF file
// Uncolored vertices line : x y z
// Colored vertices line : x y z r g b rgbmax
void Mesh::loadOFF(const std::string & filename) {
    
    std::ifstream in(filename.c_str());
    if (!in)
        exit(EXIT_FAILURE);

    std::string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    vertices.resize(sizeV);
    triangles.resize(sizeT);
    
    colorType = (offString == "COFF") ? ColorType_Vertex : ColorType_None;
    
    if (colorType == ColorType_Vertex) {
        vertColors.resize(sizeV);
        for (unsigned int i = 0; i < sizeV; i++) {
            in >> vertices[i].position >> vertColors[i] >> tmp;
            vertColors[i] /= 255.0;
        }
    } else {
        for (unsigned int i = 0; i < sizeV; i++) {
            in >> vertices[i].position;
        }
    }

    std::string line;
    std::getline(in, line);

    int s;
    std::getline(in, line);
    std::istringstream lineStream(line);
    lineStream >> s;
    // Check 1ere ligne de triangles pour savoir si on a des couleurs de face ou non
    for (unsigned int j = 0; j < 3; j++) {
        lineStream >> triangles[0].v[j];
    }
    if (!(lineStream >> std::ws).eof()) {
        colorType = ColorType_Face;
        faceColors.resize(sizeT);
        lineStream >> faceColors[0][0] >> faceColors[0][1] >> faceColors[0][2];
        faceColors[0] /= 255.0f;
    }
    
    // Lignes suivantes
    for (unsigned int i = 1; i < sizeT; i++) {
        std::getline(in, line);
        std::istringstream lineStream(line);
        lineStream >> s;

        for (unsigned int j = 0; j < 3; j++) {
            lineStream >> triangles[i].v[j];
        }

        if (colorType == ColorType_Face) {
            lineStream >> faceColors[i][0] >> faceColors[i][1] >> faceColors[i][2];
            faceColors[i] /= 255.0f;
        }
    }

    in.close();

    for (unsigned int i = 0; i < triangles.size(); i++) {
        triangles[i].v[3] = i;
    }
}


void Mesh::recomputeNormals () {
    for (unsigned int i = 0; i < vertices.size (); i++)
        vertices[i].normal = Vec3 (0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < triangles.size (); i++) {
        Vec3 e01 = vertices[triangles[i].v[1]].position -  vertices[triangles[i].v[0]].position;
        Vec3 e02 = vertices[triangles[i].v[2]].position -  vertices[triangles[i].v[0]].position;
        Vec3 n = Vec3::cross (e01, e02);
        n.normalize ();
        for (unsigned int j = 0; j < 3; j++)
            vertices[triangles[i].v[j]].normal += n;
    }
    for (unsigned int i = 0; i < vertices.size (); i++)
        vertices[i].normal.normalize ();
}

void Mesh::centerAndScaleToUnit () {
    Vec3 c(0,0,0);
    for  (unsigned int i = 0; i < vertices.size (); i++)
        c += vertices[i].position;
    c /= vertices.size ();
    float maxD = (vertices[0].position - c).length();
    for (unsigned int i = 0; i < vertices.size (); i++){
        float m = (vertices[i].position - c).length();
        if (m > maxD)
            maxD = m;
    }
    for  (unsigned int i = 0; i < vertices.size (); i++)
        vertices[i].position = (vertices[i].position - c) / maxD;
}

void Mesh::computeKDTree() {
    computeAABB();
    kdtree = new KDTree(triangles, aabb, vertices);
}

RayTriangleIntersection Mesh::intersect( Ray const & ray ) const {
        if( kdtree == NULL )  return intersectOld( ray );
        RayTriangleIntersection intersection = kdtree->intersect(ray);
        return intersection;

    }