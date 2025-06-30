out vec4 color;

in VS_OUT{
    vec2 coordinates;
    flat int index;
}fs_in;

uniform sampler2DArray textures[9];
uniform int letter_map[arr_limit];
uniform int texture_map[arr_limit];
uniform vec3 text_color;

void main() {
    int tex_index = texture_map[fs_in.index];
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textures[tex_index], vec3(fs_in.coordinates.xy, letter_map[fs_in.index])).r);
    color = vec4(text_color, 1.0) * sampled;
}