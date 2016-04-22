// on mac you may have to say: "attribute vec4 vPosition;" instead of this:
attribute vec4 vPosition;
// on mac you may have to say: "attribute vec4 vColor;" instead of this:
attribute vec4 vNormal;

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

uniform vec4 viewer_pos;
uniform mat4 LookAt;
uniform mat4 Perspective;

// product of components, which we will use for shading calculations:
vec3 product(vec4 a, vec4 b)
{
    return vec3(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
}

void main() 
{

    vec3 light_vertex = normalize(vec3(light_position - vPosition));
    vec3 viewer_vertex = normalize(vec3(viewer_pos - vPosition));
    vec3 H = normalize(vec3(light_vertex + viewer_vertex));
    vec3 N = vec3(vNormal);
    
    vec3 ambient_color = product(material_ambient, light_ambient);
    
    float kd = max(dot(light_vertex, N), 0.0);
    vec3 diffuse_color = kd * product(material_diffuse, light_diffuse);

    float ks = max(dot(N, H), 0.0);
    vec3 specular_color = pow(ks, material_shininess) * product(material_specular, light_specular);
    
    color = vec4(diffuse_color + ambient_color + specular_color, 1.0);
    
    gl_Position = Perspective * LookAt * vPosition;
}
