//
//  readscene.cpp
//  CG_HW_2.1
//
//  Created by Rui Lu on 4/13/16.
//  Copyright © 2016 Rui Lu. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "amath.h"
//#include "hello_triangle.h"

#define IM_DEBUGGING

using namespace std;

typedef amath::vec4  point4;

void read_wavefront_file (const char *file, vector<point4> &vertices, vector<vec4> &normals) {
    
    vector<int> tris;
    vector<float> verts;
    
    vector<vector<vec3>> all_vert_norms;
    vector<vector<vec3>> vert_norms;
    vector<vector<vec3>> triangles;
    vector<vec3> tri_norms;
    vector<vec3> vert_temp;
    
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
    
    double v0_x, v0_y, v0_z;
    double v1_x, v1_y, v1_z;
    double v2_x, v2_y, v2_z;
    
    vert_norms.resize(verts.size() / 3);
    vert_temp.resize(verts.size() / 3);
    triangles.resize(tris.size() / 3);
    
    cout<<verts.size()<<" "<<vert_norms.size()<<endl;
    
    vec3 n;
    
    for (int i = 0; i < (int)(tris.size() / 3); i++) {
        
        v0_x = verts[3 * tris[3 * i]];
        v0_y = verts[3 * tris[3 * i] + 1];
        v0_z = verts[3 * tris[3 * i] + 2];
        v1_x = verts[3 * tris[3 * i + 1]];
        v1_y = verts[3 * tris[3 * i + 1] + 1];
        v1_z = verts[3 * tris[3 * i + 1] + 2];
        v2_x = verts[3 * tris[3 * i + 2]];
        v2_y = verts[3 * tris[3 * i + 2] + 1];
        v2_z = verts[3 * tris[3 * i + 2] + 2];
        
        vertices.push_back(point4(v0_x, v0_y, v0_z, 1.0));
        vertices.push_back(point4(v1_x, v1_y, v1_z, 1.0));
        vertices.push_back(point4(v2_x, v2_y, v2_z, 1.0));
        
        vert_temp[tris[3 * i]] = vec3(v0_x, v0_y, v0_z);
        vert_temp[tris[3 * i + 1]] = vec3(v1_x, v1_y, v1_z);
        vert_temp[tris[3 * i + 2]] = vec3(v2_x, v2_y, v2_z);
        
        n = normalize(cross(vert_temp[tris[3 * i + 1]] - vert_temp[tris[3 * i]],
                                  vert_temp[tris[3 * i + 2]] - vert_temp[tris[3 * i]]));
        
        
        triangles[i].push_back(vec3(v0_x, v0_y, v0_z));
        triangles[i].push_back(vec3(v1_x, v1_y, v1_z));
        triangles[i].push_back(vec3(v2_x, v2_y, v2_z));
        
        tri_norms.push_back(n);
        vert_norms[tris[3 * i]].push_back(n);
        vert_norms[tris[3 * i + 1]].push_back(n);
        vert_norms[tris[3 * i + 2]].push_back(n);
        
    }
    
    all_vert_norms.resize(vertices.size());
//    std::cout<<"triangle normal size: "<<tri_norms.size()<<std::endl;
//    std::cout<<"triangle size: "<<triangles.size()<<std::endl;
    
    std::cout<<"From now on, you will wait for a relative long time, because there is 3-for-loop, \n"
             <<"I will optimize it.\n"<<std::endl;
    
    for (int i = 0; i < triangles.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            vec3 v_out = triangles[i][j];
            for (int k = 0; k < vertices.size(); ++k) {
                vec4 v_in = vertices[k];
                if (v_out[0] == v_in[0] && v_out[1] == v_in[1] && v_out[1] == v_in[1]) {
                    all_vert_norms[k].push_back(tri_norms[i]);
                }
            }
        }
    }
    
    std::cout<<"a_v_n size: "<<all_vert_norms.size()<<std::endl;
    
    float count = 0.0;
    vec3 temp;
    for (int i = 0; i < all_vert_norms.size(); ++i) {
        count = (int)all_vert_norms[i].size();
        temp = vec3(0.0, 0.0, 0.0);
        
        for (int j = 0; j < count; ++j) {
            temp += all_vert_norms[i][j];
        }
        
        temp = normalize(temp / count);
        normals.push_back(vec4(temp[0], temp[1], temp[2], 0.0));
    }
    
}