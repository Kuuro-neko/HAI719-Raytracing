#ifndef CONSTANTS_H
#define CONSTANTS_H

// Main constants
#define MULTI_THREADED 1 // 1 for multi-threading, 0 for single-threading
#define MONORAY 0 // 1 for monoray (for debugging purpose), 0 for normal ray tracing
#define DEFAULT_SELECTED_SCENE 2 // Default scene to be displayed

// Ray tracing constants
#define DEFAULT_NSAMPLES 20 // Default number of samples per pixel
#define MAXBOUNCES 6 // Number of bounces per ray
#define NB_ECH 10 // Number of shadow rays per light

// KDTree constants
#define KDTREE_MAX_DEPTH 100 // Maximum depth of the KDTree
#define KDTREE_TRIANGLES_PER_LEAF 40 // Maximum number of triangles per leaf

#define EPSILON 0.00001

#endif // CONSTANTS_H