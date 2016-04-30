//
//  beziertool.cpp
//  CG_HW_2.3
//
//  Created by Rui Lu on 4/26/16.
//  Copyright © 2016 Rui Lu. All rights reserved.
//

#include "beziertool.h"

/*
 *  Tools
 */
void eval_bez(const vector<point4> controlpoints, int degree, const double t,
              point4 &pnt, vec4 &tangent) {
    vector<point4> cps;
    cps.resize(degree);
    vec4 derivative;
    
    for (int i = 0; i < degree; ++i) {
        cps[i] = controlpoints[i] * (1.0 - t);
        cps[i] = controlpoints[i + 1] * t + cps[i];
    }
    
    for (int i = 0; i < degree - 1; ++i) {
        for (int j = 0; j < degree - 1 - i; ++j) {
            
            // compute the derivative
            if (i == degree - 2) {
                derivative = (cps[1] - cps[0]) * degree * 1.0;
            }
            
            cps[j] = cps[j] * (1.0 - t);
            cps[j] = cps[j + 1] * t + cps[j];
        }
    }
    
    pnt = cps[0];
    tangent = derivative;
}

/*
 *  Triangle
 */
Triangle::Triangle(point4 v_1, point4 v_2, point4 v_3,
                   vec4 n_1, vec4 n_2, vec4 n_3) {
    this->v_1 = v_1;
    this->v_2 = v_2;
    this->v_3 = v_3;
    this->n_1 = n_1;
    this->n_2 = n_2;
    this->n_3 = n_3;
}

/*
 *  Patch
 */
Patch::Patch() {}

Patch::Patch(int u, int v) {
    this->u_degree = u;
    this->v_degree = v;
}

void Patch::setPoints(vector< vector< point4 > > points) {
    this->points = points;
}

void Patch::uvSampling(int NumSample, vector<Triangle> &triangles) {
    
    double step = 1.0 / NumSample;
    double u, v;
    
    point4 v_a, v_b, v_c, v_d;
    vec4 n_a, n_b, n_c, n_d;
    
    // Total sample number: NumSample x NumSample
    for (int i = 0; i < NumSample; ++i) {
        for (int j = 0; j < NumSample; ++j) {
            u = i * step;
            v = j * step;
            
            interpolate(u, v, v_a, n_a);
            interpolate(u, v + step, v_b, n_b);
            interpolate(u + step, v, v_c, n_c);
            interpolate(u + step, v + step, v_d, n_d);
            
            triangles.push_back(Triangle(v_a, v_b, v_c, n_a, n_b, n_c));
            triangles.push_back(Triangle(v_c, v_b, v_d, n_c, n_b, n_d));
        }
    }
}

void Patch::interpolate(double u, double v, point4 &point, vec4 &normal) {
    vector<point4> u_curve;
    u_curve.resize(u_degree + 1);
    vector<point4> v_curve;
    v_curve.resize(v_degree + 1);
    vector<point4> tmp_curve;
    tmp_curve.resize(v_degree + 1);
    
    vec4 tmp_tangent;
    point4 u_point;
    point4 v_point;
    vec4 u_tangent;
    vec4 v_tangent;
    
    for (int i = 0; i < v_degree + 1; ++i) {
        eval_bez(this->points[i], u_degree, u, v_curve[i], tmp_tangent);
    }
    
    for (int i = 0; i < u_degree + 1; ++i) {
        for (int j = 0; j < v_degree + 1; ++j) {
            tmp_curve[j] = points[j][i];
        }
        eval_bez(tmp_curve, v_degree, v, u_curve[i], tmp_tangent);
     }
    
    eval_bez(u_curve, u_degree, u, u_point, u_tangent);
    eval_bez(v_curve, v_degree, v, v_point, v_tangent);
    
//    std::cout<<"u_point: "<<u_tangent.x<<" "<<u_tangent.y<<" "<<u_tangent.z<<std::endl;
//    std::cout<<"v_point: "<<v_tangent.x<<" "<<v_tangent.y<<" "<<v_tangent.z<<std::endl;
    
    normal = normalize(cross(u_tangent, v_tangent));
//    if (normal.x != 0.0 && normal.y != 0.0 && normal.z != 0.0) {
//        normal = normalize(normal);
//    }
    point = u_point;
    
}

/*
 *  Model
 */
Model::Model() {}

void Model::setPatches(vector<Patch> patches) {
    this->patches = patches;
}

void Model::subDivide(int NumSample) {
    for (int i = 0; i < patches.size(); ++i) {
        Patch patch = patches[i];
        patch.uvSampling(NumSample, triangles);
    }
}

void Model::getDataFromTriangles(vector<point4> &vertices, vector<vec4> &normals) {
    
    for (int i = 0; i < triangles.size(); ++i) {
        Triangle t = triangles[i];
        vertices.push_back(t.v_1);
        normals.push_back(t.n_1);
        vertices.push_back(t.v_2);
        normals.push_back(t.n_2);
        vertices.push_back(t.v_3);
        normals.push_back(t.n_3);
    }
}

void Model::clearData() {
    triangles.clear();
    patches.clear();
}