out vec4 out_color;

in VS_OUT{
    vec2 coordinates;
    flat int index;
}fs_in;

uniform sampler2DArray textures[texture_count];
uniform int letter_map[arr_limit];
uniform int texture_map[arr_limit];
uniform int color_map[arr_limit];

uniform vec3 color[4];

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textures[texture_map[fs_in.index]], vec3(fs_in.coordinates.xy, letter_map[fs_in.index])).r);
    out_color = vec4(color[color_map[fs_in.index]], 1.0) * sampled;
}