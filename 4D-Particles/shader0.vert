#version 420

in vec3 vertexLocation;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec3 vertexMatDiffuse;
in vec3 vertexMatSpecular;
in float vertexShininess;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

out vec3 v;
out vec3 N;
out vec2 texCoord;
out vec3 matDiff;
out vec3 matSpec;
out float shiny;

void main() {
    vec4 vLoc = vec4(vertexLocation, 1.0);
    vec3 newNormal = vec3(-vertexNormal.x, -vertexNormal.y, -vertexNormal.z);
    mat3 normalMatrix = mat3(transpose(inverse(modelviewMatrix)));

    v = (modelviewMatrix * vLoc).xyz;
    N = normalize(normalMatrix * newNormal);
    
    texCoord = vertexTexCoord;
    matDiff = vertexMatDiffuse;
    matSpec = vertexMatSpecular;
    shiny = vertexShininess;
        
    gl_Position = projectionMatrix * modelviewMatrix * vLoc;
}
