// on mac you may have to say: "attribute vec4 vPosition;" instead of this:
attribute vec4 vPosition;
// on mac you may have to say: "attribute vec4 vColor;" instead of this:
attribute vec4 vNormal;

// on mac you may have to say: "varying vec4 color;" instead of this:
varying vec4 color;

uniform vec4 light_position;
uniform vec4 viewer_pos;
uniform mat4 LookAt;
uniform mat4 Perspective;

varying vec3 light_vertex;
varying vec3 viewer_vertex;
varying vec3 H;
varying vec3 N;

// product of components, which we will use for shading calculations:
vec3 product(vec4 a, vec4 b)
{
    return vec3(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
}

void main() 
{
    light_vertex = normalize(vec3(light_position - vPosition));
    viewer_vertex = normalize(vec3(viewer_pos - vPosition));
    H = normalize(vec3(light_vertex + viewer_vertex));
    N = vec3(vNormal);
    
    gl_Position = Perspective * LookAt * vPosition;
}
