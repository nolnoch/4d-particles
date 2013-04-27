#version 420

layout (std140) uniform Light {
    vec3 lightPos;
    vec3 lightAmb;
    vec3 lightDiff;
    vec3 lightSpec;
};

uniform sampler2D tex;

in vec3 v;
in vec3 N;
in vec2 texCoord;
in vec3 matDiff;
in vec3 matSpec;
in float shiny;

out vec4 phongColor;

void main() {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 texture;
    
    vec3 L = normalize(lightPos - v);
    vec3 R = normalize(reflect(-L, N));
    vec3 V = normalize(-v);
    
    ambient = lightAmb * vec3(0.15, 0.15, 0.15);
    
    diffuse = clamp(lightDiff * matDiff * max(dot(N, L), 0.0), 0.0, 1.0);
    if (texCoord.s + texCoord.t > 0) {
      texture = texture2D(tex, vec2(texCoord.s, texCoord.t));
    } else {
      texture = vec4(1.0);
    }
    
    specular = clamp(lightSpec * matSpec * pow(max(dot(R, V), 0.0), shiny), 0.0, 1.0);
    
    phongColor = vec4(clamp(ambient + (diffuse * texture.rgb) + specular, 0.0, 1.0), 1.0);   
}
