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
#include "beziertool.h"
#include "amath.h"

#define PI 3.14159265

using namespace std;

int NumVertices = 0;
int NumNormals = 0;
int NumPatches = 0;

typedef amath::vec4  point4;
typedef amath::vec4  color4;

GLfloat theta = 357.0;  // polar angle for spherical polar coordinate
GLfloat phi = 0.8; // azimuthal angle for spherical polar coordinate
GLfloat radius = 5.0;
GLfloat near_plane = 1.0;
GLfloat far_plane = 50.0;
GLfloat field_of_view = 40.0;
GLfloat aspect_ration = 1.0;

// viewer's position, for lighting calculations
vec4 viewer;
vec4 at;

GLuint buffers[2];

point4 *points;
vec4 *normals;
vector<point4> v;
vector<vec4> n;

vector<Patch> models;
Model model;
int NumSample = 10;

// light & material definitions, again for lighting calculations:
point4 light_position = point4(100.0, 100.0, 100.0, 1.0);
color4 light_ambient = color4(0.2, 0.2, 0.2, 1.0);
color4 light_diffuse = color4(1.0, 1.0, 1.0, 1.0);
color4 light_specular = color4(1.0, 1.0, 1.0, 1.0);

color4 material_ambient = color4(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse = color4(1.0, 0.8, 0.0, 1.0);
color4 material_specular = color4(1.0, 0.8, 0.0, 1.0);
GLfloat material_shininess = 100.0;

// a transformation matrix, for the rotation, which we will apply to every
// vertex:
mat4 ctm;
mat4 lookAt;
mat4 perspective;

std::string filename;
std::string extension;

GLuint program; //shaders

void read_wavefront_file(const char *file, std::vector<point4> &v, std::vector<vec4> &n);
void read_custom_file(const char* file, vector<Patch> &model);

inline vec4 sphere_To_cartesian(float r, float theta, float phi) {
    return vec4(r*sin(theta)*sin(phi), r*cos(phi), r*cos(theta)*sin(phi), 1.0);
}

// initialization: set up a Vertex Array Object (VAO) and then
void init()
{
    
    // create a vertex array object - this defines mameory that is stored
    // directly on the GPU
    GLuint vao;
    
    // deending on which version of the mac OS you have, you may have to do this:
#ifdef __APPLE__
    glGenVertexArraysAPPLE( 1, &vao );  // give us 1 VAO:
    glBindVertexArrayAPPLE( vao );      // make it active
#else
    glGenVertexArrays( 1, &vao );   // give us 1 VAO:
    glBindVertexArray( vao );       // make it active
#endif

    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);  // make it active
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(*points) * NumVertices + sizeof(*normals) * NumNormals, NULL, GL_STATIC_DRAW);
    
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(*points) * NumVertices, points);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(*points) * NumVertices, sizeof(*normals) * NumNormals, \
                    normals);
    
    program = InitShader("vshader_passthrough.glsl", "fshader_passthrough.glsl");
    glUseProgram(program);
    
    // this time, we are sending TWO attributes through: the position of each
    // transformed vertex, and the color we have calculated in tri().
    GLuint loc, loc2;
    
    loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    loc2 = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(loc2);
    glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(*points) * NumVertices));
    
    // set value to vshader according to location
    GLuint lp = glGetUniformLocation(program, "light_position");
    glUniform4fv(lp, 1, (GLfloat *)light_position);
    
    GLuint la = glGetUniformLocation(program, "light_ambient");
    glUniform4fv(la, 1, light_ambient);
    
    GLuint ld = glGetUniformLocation(program, "light_diffuse");
    glUniform4fv(ld, 1, (GLfloat *)light_diffuse);
    
    GLuint ls = glGetUniformLocation(program, "light_specular");
    glUniform4fv(ls, 1, (GLfloat *)light_specular);
    
    GLuint ma = glGetUniformLocation(program, "material_ambient");
    glUniform4fv(ma, 1, (GLfloat *)material_ambient);
    
    GLuint md = glGetUniformLocation(program, "material_diffuse");
    glUniform4fv(md, 1, (GLfloat *)material_diffuse);
    
    GLuint ms = glGetUniformLocation(program, "material_specular");
    glUniform4fv(ms, 1, (GLfloat *)material_specular);
    
    GLuint msh = glGetUniformLocation(program, "material_shininess");
    glUniform1f(msh, material_shininess);
    
    // set the background color (white)
    glClearColor(1.0, 1.0, 1.0, 1.0); 
}



void display( void )
{
 
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(*points) * NumVertices + sizeof(*normals) * NumNormals, NULL, GL_STATIC_DRAW);
    
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(*points) * NumVertices, points);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(*points) * NumVertices, sizeof(*normals) * NumNormals, \
                    normals);
    
    GLuint loc, loc2;
    
    loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    loc2 = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(loc2);
    glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(*points) * NumVertices));
    
    
    // clear the window (with white) and clear the z-buffer (which isn't used
    // for this example).
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // based on where the mouse has moved to, construct a transformation matrix:
    // ctm = Translate(0.0, 0.0, 0.0);
    
    viewer = sphere_To_cartesian(radius, theta, phi);
    
    vec4 viewer_dir = vec4(0.0, 0.0, 0.0, 1.0) - viewer;
    vec3 right_dir = cross(viewer_dir, vec4(0.0, 1.0, 0.0, 0.0));
    vec4 up = cross(vec4(right_dir, 0.0), viewer_dir);
    
    lookAt = LookAt(viewer, at, up);
    perspective = Perspective(field_of_view, aspect_ration, near_plane, far_plane);
    
    glUniform4fv(glGetUniformLocation(program, "viewer_pos"), 1, viewer);
    glUniformMatrix4fv(glGetUniformLocation(program, "LookAt"), 1, GL_TRUE, lookAt);
    glUniformMatrix4fv(glGetUniformLocation(program, "Perspective"), 1, GL_TRUE, perspective);
    
    // draw the VAO:
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    
    // move the buffer we drew into to the screen, and give us access to the one
    // that was there before:
    glutSwapBuffers();
}


// use this motion func to demonstrate translation - it adjusts posx and
// posy based on the mouse movement. posx and posy are then used in the
// display callback to generate the transformation, ctm, that is applied
// to all the vertices before they are displayed:
void mouse_move_translate (int x, int y)
{
    
    static int lastx = 0;
    static int lasty = 0;  // keep track of where the mouse was last:

    float amntX = x - lastx;
    if (amntX != 0) {
        theta +=  amntX / 10;
        if (theta > 360.0 ) theta -= 360.0;
        if (theta < 0.0 ) theta += 360.0;
        
        lastx = x;
    }

    float amntY = y - lasty;
    if (amntY != 0) {
        phi +=  amntY / 50;
        if (phi < 5 * PI / 180) phi = 5 * PI / 180;
        if (phi > PI - 5 * PI / 180) phi = PI - 5 * PI / 180;
        lasty = y;
    }
    
    // force the display routine to be called as soon as possible:
    glutPostRedisplay();
    
}


// the keyboard callback, called whenever the user types something with the
// regular keys.
void mykey(unsigned char key, int mousex, int mousey)
{
    if(key=='q'||key=='Q') {
        delete[] points;
        delete[] normals;
        exit(0);
    }
    
    int reset_old_samples = NumSample;
    
    // and r resets the view:
    if (key =='r') {
        radius = 5.0;
        theta = 357.0;
        phi = 0.8;
        NumSample = 10;
    }
    
    // z moves the camera closer to the origin
    if (key == 'z') {
        if (radius > 2) {
            std::cout<<"move closer."<<std::endl;
            radius -= 0.2;
        } else {
            std::cout<<"reach near-plane limit."<<std::endl;
        }
    }
    
    // x pulls the camera away from the origin
    if (key == 'x') {
        if (radius <= 50) {
            std::cout<<"pull away."<<std::endl;
            radius += 0.2;
        } else {
            std::cout<<"reach far-plane limit."<<std::endl;

        }
    }
    
    if (extension.compare("txt") == 0) {
        
        int oldNumSample = NumSample;
        if (key == '<') {
            if (NumSample < 20) {
                NumSample++;
                std::cout<<'<'<<std::endl;
            }
        }
        if (key == '>') {
            if (NumSample > 2) {
                NumSample--;
                std::cout<<'>'<<std::endl;
            }
        }
        
        if (oldNumSample != NumSample || reset_old_samples != NumSample) {
            std::cout<<"change"<<std::endl;
            
            v.clear();
            n.clear();
            
            model.clearData();
            model.setPatches(models);
            model.subDivide(NumSample);
            model.getDataFromTriangles(v, n);
            
            std::cout<<v.size()<<std::endl;
            std::cout<<n.size()<<std::endl;
            NumVertices = (int)v.size();
            NumNormals = (int)n.size();
            
            delete[] points;
            delete[] normals;
            
            points = new point4[NumVertices];
            normals = new vec4[NumNormals];
            
            for (int i = 0; i < NumVertices; ++i) {
                points[i] = v[i];
            }
            for (int i = 0; i < NumNormals; ++i) {
                normals[i] = n[i];
            }
        }
    }
    
    glutPostRedisplay();
    
}



int main(int argc, char** argv)
{
    filename = (std::string)argv[1];
    extension = filename.substr(filename.size() - 3);
    
    if (extension.compare("obj") == 0) {
        read_wavefront_file(argv[1], v, n);
        NumVertices = (int)v.size();
        NumNormals = (int)n.size();
        
        points = new point4[NumVertices];
        normals = new vec4[NumNormals];
        
//        points.resize(NumVertices);
//        normals.reserve(NumNormals);
        
        for (int i = 0; i < NumVertices; ++i) {
            points[i] = v[i];
        }
        for (int i = 0; i < NumNormals; ++i) {
            normals[i] = n[i];
        }
    } else if (extension.compare("txt") == 0) {
        read_custom_file(argv[1], models);
        NumPatches = (int)models.size();
        model.setPatches(models);
        model.subDivide(NumSample);
        model.getDataFromTriangles(v, n);
        
        std::cout<<"length of v: "<<v.size()<<std::endl;
        std::cout<<"length of n: "<<n.size()<<std::endl;
        
        NumVertices = (int)v.size();
        NumNormals = (int)n.size();
        
        points = new point4[NumVertices];
        normals = new vec4[NumNormals];

        for (int i = 0; i < NumVertices; ++i) {
            points[i] = v[i];
        }
        for (int i = 0; i < NumNormals; ++i) {
            normals[i] = n[i];
        }
    }
    
    
    viewer = sphere_To_cartesian(radius, theta, phi);
    at = vec4(0.0, 0.0, 0.0, 1.0);
    
    // initialize glut, and set the display modes
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    
    // give us a window in which to display, and set its title:
    glutInitWindowSize(512, 512);
    glutCreateWindow("HW2.4 - rl2784");
    
    // for displaying things, here is the callback specification:
    glutDisplayFunc(display);
    
    // when the mouse is moved, call this function!
    // you can change this to mouse_move_translate to see how it works
    glutMotionFunc(mouse_move_translate);
    
    // for any keyboard activity, here is the callback:
    glutKeyboardFunc(mykey);
    
#ifndef __APPLE__
    // initialize the extension manager: sometimes needed, sometimes not!
    glewInit();
#endif
    
    // call the init() function, defined above:
    init();
    
    // enable the z-buffer for hidden surface removel:
    glEnable(GL_DEPTH_TEST);

    // once we call this, we no longer have control except through the callbacks:
    glutMainLoop();
    return 0;
}
