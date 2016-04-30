//
//  beziertool.h
//  CG_HW_2.3
//
//  Created by Rui Lu on 4/26/16.
//  Copyright © 2016 Rui Lu. All rights reserved.
//

#ifndef beziertool_h
#define beziertool_h

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#endif

#include <vector>
#include <string>
#include <cstring>
#include "amath.h"

typedef amath::vec4  point4;
typedef amath::vec4  color4;

using namespace std;

class Triangle {
public:
    point4 v_1, v_2, v_3;
    vec4 n_1, n_2, n_3;
    
    Triangle(point4 v_1, point4 v_2, point4 v_3,
             vec4 n_1, vec4 n_2, vec4 n_3);
};

class Patch {
public:
    vector< vector< point4 > > points;
    int u_degree, v_degree;
    
    Patch();
    Patch(int u, int v);
    void setPoints(vector< vector< point4 > > points);
    
    void uvSampling(int NumSample, vector<Triangle> &triangles);
    void interpolate(double u, double v, point4 &pnt, vec4 &tangent);
};

class Model {
public:
    vector<Patch> patches;
    vector<Triangle> triangles;
    
    Model();
    void setPatches(vector<Patch> patches);
    void subDivide(int NumSample);
    void getDataFromTriangles(vector<point4> &v, vector<vec4> &n);
    void clearData();
};

void eval_bez(const vector<point4> controlpoints, int degree, const double t,
              point4 &pnt, vec4 &tangent);

#endif /* beziertool_h */
