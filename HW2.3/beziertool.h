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


#endif /* beziertool_h */
