// PROCEDURAL checkerboard, no texture involved. floor() quantizes world x/z
// into cells; the parity of (cellX + cellZ) alternates like a chessboard.
// When the Textures chapter arrives, this collapses into one texture() fetch.

#version 300 es
precision highp float;

// Material property
uniform vec3 MaterialAmbient;
uniform vec3 MaterialSpecular;

// Light property
uniform vec3 LightAmbient;
uniform vec3 LightSpecular;
uniform vec3 LightDiffuse;
uniform float ShininessFactor;

uniform vec3 LightPosition;

in vec3 WorldPos;
in vec3 eyeCoord;
in vec3 normalCoord;

vec3 normalizeNormal, normalizeEyeCoord, normalizeLightVec, V, R, ambient, diffuse, specular;
float sIntensity, cosAngle;

out vec4 outColor;

vec3 PhongShading(vec3 MaterialDiffuse){
    normalizeNormal = normalize(normalCoord);
    normalizeEyeCoord = normalize(eyeCoord);
    normalizeLightVec = normalize(LightPosition - eyeCoord);

    // Diffuse Intensity
    cosAngle = max(0.0, dot(normalizeNormal, normalizeLightVec));

    // Specular Intensity 
    V = -normalizeEyeCoord; // viewer's vector
    R = reflect (-normalizeLightVec, normalizeNormal); // reflectivity
    sIntensity = pow(max(0.0, dot(R, V)), ShininessFactor);

    // ADS color as result of Material & Light interaction
    ambient = MaterialAmbient * LightAmbient;
    diffuse = MaterialDiffuse * LightDiffuse;
    specular = MaterialSpecular * LightSpecular;

    return ambient + (cosAngle * diffuse) + (sIntensity * specular);

}

void main() {
    float cellX = floor(WorldPos.x * 1.5);
    float cellZ = floor(WorldPos.z * 1.5);
    float check = mod(cellX + cellZ, 2.0);

    vec3 dark  = vec3(0.16, 0.19, 0.23);
    vec3 light = vec3(0.75, 0.78, 0.82);
    vec3 col   = mix(dark, light, check);

    // Subtle radial fade so the plane's edge doesn't end in a hard line
    float d = clamp(length(WorldPos.xz) / 8.0, 0.0, 1.0);
    col *= 1.0 - 0.45 * d * d;

    outColor = vec4(PhongShading(col), 1.0);
}