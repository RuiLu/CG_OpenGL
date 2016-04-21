// Very simple display triangle program, that allows you to rotate the
// triangle around the Y axis.
//
// This program does NOT use a vertex shader to define the vertex colors.
// Instead, it computes the colors in the display callback (using Blinn/Phong)
// and passes those color values, one per vertex, to the vertex shader, which
// passes them directly to the fragment shader. This achieves what is called
// "gouraud shading".

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#endif

#include <vector>
#include "amath.h"

int NumVertices = 0;

typedef amath::vec4  point4;
typedef amath::vec4  color4;

float theta = 0.0;  // polar angle for spherical polar coordinate
float phi = 0.0; // azimuthal angle for spherical polar coordinate

float posx = 0.0;   // translation along X
float posy = 0.0;   // translation along Y
float posz = 5.0;   // translation along Z

float radius = 5.0;
float beta = 0.0;  // rotation around the X axis

GLuint buffers[2];
GLint matrix_loc;

point4 *vertices;

std::vector<point4> v;

// viewer's position, for lighting calculations
vec4 viewer = vec4(posx, posy, radius, 1.0);
vec4 up = vec4(0.0, 1.0, 0.0, 1.0);
vec4 at = vec4(0.0, 0.0, 0.0, 1.0);

// light & material definitions, again for lighting calculations:
//point4 light_position = point4(0.0, 0.0, -1.0, 0.0);
point4 light_position = point4(100.0, 100.0, 100.0, 1.0);
color4 light_ambient = color4(0.2, 0.2, 0.2, 1.0);
color4 light_diffuse = color4(1.0, 1.0, 1.0, 1.0);
color4 light_specular = color4(1.0, 1.0, 1.0, 1.0);

color4 material_ambient = color4(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse = color4(1.0, 0.8, 0.0, 1.0);
color4 material_specular = color4(1.0, 0.8, 0.0, 1.0);
float material_shininess = 100.0;

// we will copy our transformed points to here:
point4 *points;

// and we will store the colors, per face per vertex, here. since there is
// only 1 triangle, with 3 vertices, there will just be 3 here:
color4 *colors;

// a transformation matrix, for the rotation, which we will apply to every
// vertex:
mat4 ctm;

// a transformation matrix, for LookAt(), which we will apply to every vertex
mat4 lookAt;

// a transformation matrix, for Perspective(), which we will apply to every vertex
mat4 perspective;

mat4 modelView;

GLuint program; //shaders
GLuint modelView_loc;

void read_wavefront_file (const char *file, std::vector<point4> &v);

// product of components, which we will use for shading calculations:
vec4 product(vec4 a, vec4 b)
{
    return vec4(a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]);
}


// transform the triangle's vertex data and put it into the points array.
// also, compute the lighting at each vertex, and put that into the colors
// array.
void tri()
{
    for (int i = 0; i < NumVertices / 3; ++i) {
        // compute the lighting at each vertex, then set it as the color there:
        vec3 n1 = normalize(cross(ctm*vertices[i*3+1] - ctm*vertices[i*3],
                                  ctm*vertices[i*3+2] - ctm*vertices[i*3+1]));
        vec4 n = vec4(n1[0], n1[1], n1[2], 0.0);
        vec4 half = normalize(light_position+viewer);
        color4 ambient_color, diffuse_color, specular_color;
        
//        ambient_color = product(material_ambient, light_ambient);
        
        
        vec4 l_v = normalize(light_position - vertices[i*3]);
        float dd = dot(l_v, n);
        
        //float dd = dot(light_position, n);
        
        if(dd>0.0) diffuse_color = dd*product(light_diffuse, material_diffuse);
        else diffuse_color =  color4(0.0, 0.0, 0.0, 1.0);
        
        dd = dot(half, n);
        if(dd > 0.0) specular_color = exp(material_shininess*log(dd))*product(light_specular, material_specular);
        else specular_color = vec4(0.0, 0.0, 0.0, 1.0);
        
        
        // now transform the vertices according to the ctm transformation matrix,
        // and set the colors for each of them as well. as we are going to give
        // flat shading, we will ingore the specular component for now.
        points[i*3] = vertices[i*3];
        colors[i*3] = diffuse_color;
        
        points[i*3+1] = vertices[i*3+1];
        colors[i*3+1] = diffuse_color;
        
        points[i*3+2] = vertices[i*3+2];
        colors[i*3+2] = diffuse_color;
    }

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
    
    // set up vertex buffer object - this will be memory on the GPU where
    // we are going to store our vertex data (that is currently in the "points"
    // array)
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);  // make it active
    
    // specify that its part of a VAO, what its size is, and where the
    // data is located, and finally a "hint" about how we are going to use
    // the data (the driver will put it in a good memory location, hopefully)
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(*points) * NumVertices + sizeof(*colors) * NumVertices, NULL, GL_STATIC_DRAW);
    
    // load in these two shaders...  (note: InitShader is defined in the
    // accompanying initshader.c code).
    // the shaders themselves must be text glsl files in the same directory
    // as we are running this program:
    program = InitShader("vshader_passthrough.glsl", "fshader_passthrough.glsl");
 
    // ...and set them to be active
    glUseProgram(program);
    
    
    // this time, we are sending TWO attributes through: the position of each
    // transformed vertex, and the color we have calculated in tri().
    GLuint loc, loc2;
    
    loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    
    // the vPosition attribute is a series of 4-vecs of floats, starting at the
    // beginning of the buffer
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    loc2 = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(loc2);

    glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(*points) * NumVertices));
    
    /*
      Access to uniform variables is available after linking the program. With glGetUniformLocation you can
      retrieve the location of the uniform variable within the specified program object. Once you have that
      location you can set the value. If the variable is not found, -1 is returned. With glUniform you can
      set the value of the uniform variable.
    */
    
    modelView_loc = glGetUniformLocation(program, "modelView");
    
    // set value to vshader according to location
    GLuint attr_loc = glGetUniformLocation(program, "test");
    glUniform4fv(attr_loc, 1, vec4(0.7,0.7,0.7,1.0));
    
    GLuint lp_loc = glGetUniformLocation(program, "light_position");
    glUniform4fv(lp_loc, 1, light_position);
    
    GLuint la_loc = glGetUniformLocation(program, "light_ambient");
    glUniform4fv(la_loc, 1, light_ambient);
    
    GLuint ld_loc = glGetUniformLocation(program, "light_diffuse");
    glUniform4fv(ld_loc, 1, light_diffuse);
    
    GLuint ls_loc = glGetUniformLocation(program, "light_specular");
    glUniform4fv(ls_loc, 1, light_specular);
    
    GLuint ma_loc = glGetUniformLocation(program, "material_ambient");
    glUniform4fv(ma_loc, 1, material_ambient);

    GLuint md_loc = glGetUniformLocation(program, "material_diffuse");
    glUniform4fv(md_loc, 1, material_diffuse);

    GLuint ms_loc = glGetUniformLocation(program, "material_specular");
    glUniform4fv(ms_loc, 1, material_specular);
    
    GLuint msh_loc = glGetUniformLocation(program, "material_shininess");
    glUniform1f(msh_loc, material_shininess);
    
    // set the background color (white)
    glClearColor(1.0, 1.0, 1.0, 1.0); 
}



void display( void )
{
 
    // clear the window (with white) and clear the z-buffer (which isn't used
    // for this example).
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // based on where the mouse has moved to, construct a transformation matrix:
//    ctm = Translate(posx*.01,posy*.01,0.) * RotateY(theta) * RotateX(beta);
    ctm = Translate(0.,0.,0.);
    std::cout<<viewer<<std::endl;
    
    lookAt = LookAt(viewer, at, up);
    perspective = Perspective(40.0, 1.0, 1.0, 50.0);
    modelView = perspective * lookAt;
    glUniformMatrix4fv(modelView_loc, 1, GL_TRUE, modelView);
    
    // now build transform all the vertices and put them in the points array,
    // and their colors in the colors array:
    tri();
    
    // tell the VBO to get the data from the points array and the colors array:
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(*points) * NumVertices, *points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(*points) * NumVertices, sizeof(*colors) * NumVertices, *colors );
    
    // draw the VAO:
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    
    
    // move the buffer we drew into to the screen, and give us access to the one
    // that was there before:
    glutSwapBuffers();
}


// use this motion function to demonstrate rotation - it adjusts "theta" based
// on how the mouse has moved. Theta is then used the the display callback
// to generate the transformation, ctm, that is applied
// to all the vertices before they are displayed:
void mouse_move_rotate (int x, int y)
{
    
    static int lastx = 0;// keep track of where the mouse was last for x
    static int lasty = 0;// keep track of where the mouse was last for y
    
    int amntX = x - lastx;
    if (amntX != 0) {
        theta +=  amntX;
        if (theta > 360.0 ) theta -= 360.0;
        if (theta < 0.0 ) theta += 360.0;
        
        lastx = x;
    }
    
    int amntY = y - lasty;
    if (amntY != 0) {
        beta +=  amntY;
        if (beta > 360.0 ) beta -= 360.0;
        if (beta < 0.0 ) beta += 360.0;
        
        lasty = y;
    }

    // force the display routine to be called as soon as possible:
    glutPostRedisplay();
    
}


// use this motion func to demonstrate translation - it adjusts posx and
// posy based on the mouse movement. posx and posy are then used in the
// display callback to generate the transformation, ctm, that is applied
// to all the vertices before they are displayed:
void mouse_move_translate (int x, int y)
{
    
    static int lastx = 0;
    static int lasty = 0;  // keep track of where the mouse was last:

//    if (x - lastx < 0) ++posx;
//    else if (x - lastx > 0) --posx;
//    lastx = x;
//
//    if (y - lasty > 0) --posy;
//    else if (y - lasty < 0) ++posy;
//    lasty = y;
    
    
    
    if (x != lastx) {
        int amntX = x - lastx;
        theta +=  amntX / 1.8;
        if (theta > 360.0 ) theta -= 360.0;
        if (theta < -360.0 ) theta += 360.0;
        
        lastx = x;
    }
    if (y != lasty) {
        int amntY = y - lasty;
        phi +=  amntY / 1.8;
        if (phi > 360.0 ) phi -= 360.0;
        if (phi < -360.0 ) phi += 360.0;
        
        lasty = y;
    }
    
    viewer.x = radius * sin(theta) * cos(phi);
    viewer.y = radius * sin(theta) * sin(phi);
    viewer.z = radius * cos(theta);
    
    // force the display routine to be called as soon as possible:
    glutPostRedisplay();
    
}


// the keyboard callback, called whenever the user types something with the
// regular keys.
void mykey(unsigned char key, int mousex, int mousey)
{
    if(key=='q'||key=='Q') {
        delete vertices;
        delete points;
        delete colors;
        exit(0);
    }
    
    // and r resets the view:
    if (key =='r') {
        posx = 0;
        posy = 0;
        theta = 180.0;
        beta = 0.0;
        viewer.x = 0.0;
        viewer.y = 0.0;
        viewer.z = 5.0;
        radius = 5.0;
        glutPostRedisplay();
    }
    
    // z moves the camera closer to the origin
    if (key == 'z') {
        std::cout<<"z"<<std::endl;
        if (radius >=2) {
            float old_radius = radius;
            radius--;
            viewer.x = viewer.x * radius / old_radius;
            viewer.y = viewer.y * radius / old_radius;
            viewer.z = viewer.z * radius / old_radius;
        }
        
        glutPostRedisplay();
    }
    
    // x pulls the camera away from the origin
    if (key == 'x') {
        std::cout<<"x"<<std::endl;
        if (radius <=50) {
            float old_radius = radius;
            radius++;
            viewer.x = viewer.x * radius / old_radius;
            viewer.y = viewer.y * radius / old_radius;
            viewer.z = viewer.z * radius / old_radius;
        }
        glutPostRedisplay();
    }
}



int main(int argc, char** argv)
{
    
    read_wavefront_file(argv[1], v);
    NumVertices = (int)v.size();
    
    // initialize glut, and set the display modes
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    
    // give us a window in which to display, and set its title:
    glutInitWindowSize(512, 512);
    glutCreateWindow("HW2.2 - rl2784");
    
    // for displaying things, here is the callback specification:
    glutDisplayFunc(display);
    
    // when the mouse is moved, call this function!
    // you can change this to mouse_move_translate to see how it works
//    glutMotionFunc(mouse_move_rotate);
    glutMotionFunc(mouse_move_translate);
    // for any keyboard activity, here is the callback:
    glutKeyboardFunc(mykey);
    
#ifndef __APPLE__
    // initialize the extension manager: sometimes needed, sometimes not!
    glewInit();
#endif
    
    points = new point4[NumVertices];
    colors = new color4[NumVertices];
    
    vertices = new point4[NumVertices]();

    for (int i = 0; i < v.size(); ++i) {
        vertices[i] = v[i];
    }
    
    // call the init() function, defined above:
    init();
    
    // enable the z-buffer for hidden surface removel:
    glEnable(GL_DEPTH_TEST);

    // once we call this, we no longer have control except through the callbacks:
    glutMainLoop();
    return 0;
}
