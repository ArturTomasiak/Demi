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
uniform float gamma;
uniform float font_size;

void main() {
    vec2 tex_coord = fs_in.coordinates * font_size;
    ivec2 pixel = ivec2(clamp(tex_coord, vec2(0.0), font_size - vec2(1.0)));
    
    int tex_idx = texture_map[fs_in.index];
    int layer = letter_map[fs_in.index];
    vec3 lcd = texelFetch(textures[tex_idx], ivec3(pixel, layer), 0).rgb;
    
    float alpha = dot(lcd, vec3(0.299, 0.587, 0.114));
    
    vec3 base_color = pow(color[color_map[fs_in.index]], vec3(gamma));
    vec3 col = base_color * alpha;
    
    out_color = vec4(pow(col, vec3(1.0 / gamma)), alpha);
}