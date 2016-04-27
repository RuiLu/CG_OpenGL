//
//  readscene.cpp
//  CG_HW_2.1
//
//  Created by Rui Lu on 4/13/16.
//  Copyright © 2016 Rui Lu. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <map>
#include "amath.h"
#include "beziertool.h"
//#include "hello_triangle.h"

#define IM_DEBUGGING

using namespace std;

typedef amath::vec4 point4;

void read_wavefront_file (const char *file, vector<point4> &vertices, vector<vec4> &normals) {
    
    vector<int> tris;
    vector<float> verts;
    
    cout<<file<<endl;
    
    tris.clear();
    verts.clear();
    
    ifstream in(file);
    char buffer[1025];
    string cmd;
    
    for (int line = 1; in.good(); line++) {
        in.getline(buffer, 1024);
        buffer[in.gcount()] = 0;
        
        cmd = "";
        
        istringstream iss (buffer);
        iss>>cmd;
        
        if (cmd[0] == '#' || cmd[0] == 'g' || cmd.empty()) {
            continue;
        } else if (cmd == "v") {
            double pa, pb, pc;
            iss >> pa >> pb >> pc;
            
            verts.push_back(pa);
            verts.push_back(pb);
            verts.push_back(pc);
        } else if (cmd == "f") {
            int i, j, k;
            iss >> i >> j >> k;
            
            // vertex numbers in OBJ files start with 1, but in C++ array indices start
            // with 0, so we're shifting everything down by 1
            tris.push_back(i - 1);
            tris.push_back(j - 1);
            tris.push_back(k - 1);
        } else {
            cerr << "Parser error: invalid command at line " << line << endl;
        }
    }
    
    in.close();
    
    int NumVertice = (int)tris.size();
    int NumUniqueVertices = (int)(verts.size() / 3);
    
    vertices.resize(NumVertice);
    normals.resize(NumVertice);
    
    point4* v_temp = new point4[NumUniqueVertices];
    vec4* n_temp = new vec4[NumUniqueVertices];
    
    // get the unique vertex array
    for (int i = 0; i < NumUniqueVertices; ++i) {
        v_temp[i] = point4(verts[3*i], verts[3*i+1], verts[3*i+2], 1.0);
    }
    
    // get each vertex's normal, store in a vector
    for (int i = 0; i < (int)(tris.size() / 3); ++i) {
        int v_1 = tris[i*3];
        int v_2 = tris[i*3+1];
        int v_3 = tris[i*3+2];
        
        vec3 normal = normalize(cross(v_temp[v_2] - v_temp[v_1], v_temp[v_3] - v_temp[v_1]));
        vec4 tri_norm = vec4(normal[0], normal[1], normal[2], 0.0);
        
        n_temp[v_1] += tri_norm;
        n_temp[v_2] += tri_norm;
        n_temp[v_3] += tri_norm;
    }
    
    for (int i = 0; i < NumUniqueVertices; ++i) {
        n_temp[i] = normalize(n_temp[i]);
    }
    
    for (int i = 0; i < (int)(tris.size() /3); ++i) {
        int v_1 = tris[i*3];
        int v_2 = tris[i*3+1];
        int v_3 = tris[i*3+2];
        
        vertices[i*3] = v_temp[v_1];
        normals[i*3] = n_temp[v_1];
        vertices[i*3+1] = v_temp[v_2];
        normals[i*3+1] = n_temp[v_2];
        vertices[i*3+2] = v_temp[v_3];
        normals[i*3+2] = n_temp[v_3];
    }
    
    delete[] v_temp;
    delete[] n_temp;
}

void read_custom_file(const char *file, vector<Patch> &model) {
    
    std::cout<<file<<std::endl;
    
    ifstream in(file);
    char buffer[1025];
    string cmd;
    
    if (in.is_open()) {
        int numPatches = 0;
        
        in.getline(buffer, 1024);
        buffer[in.gcount()] = 0;
        
        cmd = "";
        
        istringstream iss(buffer);
        iss >> cmd;
        
        numPatches = atoi(cmd.c_str());
        cout<<"There are "<<numPatches<<" patches in the custom file."<<endl;
        
        vector<vector<point4>> patches;
        
        for (int i = 0; i < numPatches; ++i) {
            int u = 0, v = 0;
            
            in.getline(buffer, 1024);
            buffer[in.gcount()] = 0;
            
            istringstream iss(buffer);
            iss >> u >> v;
            
            Patch patch(u, v);
            vector<vector<point4>> points;
            
            for (int j = 0; j <= v; j++) {
                in.getline(buffer, 1024);
                buffer[in.gcount()] = 0;
                
                istringstream iss(buffer);
                vector<string> coor_list((istream_iterator<string>(iss)), istream_iterator<string>());
                
                vector<point4> row_points;
                
                for (int k = 0; k <= u; ++k) {
                    double x = atof(coor_list[3 * k].c_str());
                    double y = atof(coor_list[3 * k + 1].c_str());
                    double z = atof(coor_list[3 * k + 2].c_str());
                    
                    row_points.push_back(point4(x, y, z, 1.0));
                }
                points.push_back(row_points);
                patch.setPoints(points);
            }
            model.push_back(patch);
        }
        
    } else {
        cout<<"Custom file not found."<<endl;
    }
    
    in.close();
    
    return;
}