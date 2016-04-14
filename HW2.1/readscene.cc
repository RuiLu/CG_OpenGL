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
#include "amath.h"
//#include "hello_triangle.h"

#define IM_DEBUGGING

using namespace std;

typedef amath::vec4  point4;

void read_wavefront_file (const char *file, vector<point4> &vertices) {
    
    vector<int> tris;
    vector<float> verts;
    
    std::cout<<file<<std::endl;
    
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
        
    
    }
}