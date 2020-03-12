#version 410

in vec2 uv;

uniform vec2 cursor;
uniform sampler2D tex_ux;
uniform sampler2D tex_uy;
uniform sampler2D tex_p;
uniform sampler2D tex_dv;
uniform vec3 colormap_a, colormap_b, colormap_c, colormap_d;

out vec4 frag_color;

vec4 colormap(float v) {

 vec3 c = colormap_a + colormap_b * cos(6.28318 * (colormap_c * vec3(v, v, v) + colormap_d));
 return vec4(c.xyz, 1.0f);

}

vec4 val(float v) {

 return vec4(v, v, v, 1.0f);

}

float dist(vec2 a, vec2 b) {
  vec2 r = b-a;
  return sqrt(r.x*r.x + r.y*r.y);
}

void main () {

  frag_color = vec4(0.5, texture(tex_ux, uv).x+0.5, texture(tex_uy, uv).x+0.5, 1.0);
  //frag_color = colormap(texture(tx, uv).x);
  //frag_color += dist(uv, cursor) < 0.01f ? vec4(0, 0.5, 0, 0) : vec4(0,0,0,0);

}
