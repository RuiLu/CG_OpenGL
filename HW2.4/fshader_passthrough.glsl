// the simplest fragment shader: get the color (from the vertex shader - 
// actually interpolated from values specified in the vertex shader - and
// just pass them through to the render:
// 
// on the mac, you may need to say "varying vec4 color;" instead of this:
//varying vec4 color;

// vector parameters that are passed from vshader
varying vec3 light_vertex;
varying vec3 viewer_vertex;
varying vec3 H;
varying vec3 N;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

// product of components, which we will use for shading calculations:
vec3 product(vec4 a, vec4 b)
{
    return vec3(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
}

void main() 
{
    
  // implement blinn/phong shading in a fragement shader
  vec3 ambient_color = product(material_ambient, light_ambient);
    
  float kd = max(dot(light_vertex, N), 0.0);
  vec3 diffuse_color = kd * product(material_diffuse, light_diffuse);
    
  float ks = max(dot(N, H), 0.0);
  vec3 specular_color = pow(ks, material_shininess) * product(material_specular, light_specular);
  
  // "gl_FragColor" is already defined for us - it's the one thing you have
  // to set in the fragment shader:
  gl_FragColor = vec4(diffuse_color + ambient_color + specular_color, 1.0);;
}

