// Fragment shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* densityFragmentShaderSource = R"(
#version 330 core
// in vec3 outColor;
out vec4 color;
in vec2 TexCoord;
uniform sampler2D image;
// layout(size4x32, binding=0) writeonly uniform image2D image;
// uniform layout(rgba32f) image2D image;
// uniform layout(rgba32f) image2D image;
//        layout(location = 0) out vec4 FragColor;
//        void main() {
//           ivec2 coords = ivec2(gl_FragCoord.xy);
//        	vec4 HE = imageLoad(image, coords);
// float Ezdx = HE.z-imageLoad(image, coords-ivec2(1, 0)).z;
// float Ezdy = HE.z-imageLoad(image, coords-ivec2(0, 1)).z;
// 	   HE.xy += dt*vec2(-Ezdy, Ezdx);
//           imageStore(image, coords, HE);
void main() {
    // color = vec4(outColor, 1.0f);
    // color = texture(image, TexCoord);
    // color = texture(image, TexCoord); // * vec4(0.067f, 0.455f, 0.729f, 1.0f);
    color = vec4(0.067f, 0.455f, 0.729f, 1.0f);
    // color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";
