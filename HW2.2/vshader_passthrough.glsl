// a contrived vertex shader, that colors each vertex in a triangle
// according to its position

// we are going to be getting an attribute from the main program, named
// "vPosition", one for each vertex.
//
// on mac you may have to say: "attribute vec4 vPosition;" instead of this:
attribute vec4 vPosition;

// we are also going to be getting color values, per-vertex, from the main
// program running on the cpu (and that are then stored in the VBO on the
// card. this color, called "vColor", is just going to be passed through 
// to the fragment shader, which will intrpolate its value per-fragment
// amongst the 3 vertex colors on the triangle that fragment is a part of.
//
// on mac you may have to say: "attribute vec4 vColor;" instead of this:
attribute vec4 vColor;
attribute vec4 vNormal;

// we are going to be outputting a single 4-vector, called color, which
// may be different for each vertex.
// the fragment shader will be expecting these values, and interpolate
// them according to the distance of the fragment from the vertices
//
// on mac you may have to say: "varying vec4 color;" instead of this:
varying vec4 color;

uniform vec4 light_position;
uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

uniform mat4 modelView;

// product of components, which we will use for shading calculations:
vec4 product(vec4 a, vec4 b)
{
    return vec4(a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]);
}

void main() 
{
    vec3 pos = vec3(vPosition[0], vPosition[1], vPosition[2]);
    
    vec3 l_p = vec3(light_position[0], light_position[1], light_position[2]);
    vec3 L = normalize(l_p - pos);
    vec3 E = normalize(-pos);
    vec3 H = normalize(L + E);
    
//    vec4 N_temp = normalize(modelView * vNormal);
    vec3 N = vec3(vNormal[0], vNormal[1], vNormal[2]);
    
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
    
    color += product(material_ambient, light_ambient);
    
    float Kd = dot(L, N);
    if (Kd > 0.0) {
        color += Kd * product(light_diffuse, material_diffuse);
    } else {
        color += vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    float Ks = dot(H, N);
    if (Ks > 0.0) {
        color += exp(material_shininess*log(Ks))*product(light_specular, material_specular);
    } else {
        color += vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    gl_Position = modelView * vPosition;
//    color = diffuse_color + ambient_color;
    color[3] = 1.0;
}
