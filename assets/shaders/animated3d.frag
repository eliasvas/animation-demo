#version 330 core

vec2 light_bias = vec2(0.7,0.6); //balance between ambient and diffuse..

in vec2 f_tex_coords;
in vec3 f_normal;

out vec4 color;

uniform sampler2D diffuse_map;
uniform vec3 light_direction;

void main(){
	vec4 diffuse_color = texture(diffuse_map, f_tex_coords);
	vec3 unit_normal = normalize(f_normal);
	float diff_light = max(dot(-light_direction, unit_normal), 0.0) * light_bias.x + light_bias.y;
	color = diffuse_color * diff_light;
	color.a = 1.0;
}