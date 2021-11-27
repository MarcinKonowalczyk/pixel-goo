#include "shader.h"
#include "buffer.h"

#include <iostream>

#include <array>
#include <vector>
#include <utility>
#include <fstream>

#include <glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>

// #include <stb_truetype.h>

// Window
GLFWwindow* window;
int width = 800; int height = 600;
// int width = 400; int height = 400;
const char* title = "Pixel Goo";
// const bool fullscreen = false;
const bool fullscreen = true;
int whichMonitor = 0;
// int whichMonitor = 1;

// Textures and framebuffers
GLuint textures[7];
GLuint framebuffers[7];
const PBindex densityBufferIndex = 0;
const PBindex positionBufferIndex1 = 1;
const PBindex positionBufferIndex2 = 2;
const PBindex velocityBufferIndex1 = 3;
const PBindex velocityBufferIndex2 = 4;
const PBindex trailBufferIndex1 = 5;
const PBindex trailBufferIndex2 = 6;

GLuint vertex_buffer;

Shader screenShader("screenShader");
Shader densityShader("densityShader");
Shader positionShader("positionShader");
Shader velocityShader("velocityShader");
Shader copyShader("copyShader");
Shader trailShader("trailShader");

// Include shader source files
#include "copy.h"
#include "trail.h"
#include "screen.h"
#include "density.h"
#include "position.h"
#include "velocity.h"

Buffer trailBuffer("Trail Buffer", textures, framebuffers);
Buffer positionBuffer("Position buffer", textures, framebuffers);
Buffer velocityBuffer("Velocity buffer", textures, framebuffers);
Buffer densityBuffer("Density buffer", textures, framebuffers);
Buffer screenBuffer("Screen buffer", NULL, NULL);

// Alpha blending of each of the fragments
const float densityAlpha = 0.005f;
// const float densityAlpha = 0.9f;
const float kernelRadius = 30.0f;

// This can be quite a lot because the density buffer is lerped and particles dither
const int densityBufferDownsampling = 10;
// const int densityBufferDownsampling = 20;
int density_width = width/densityBufferDownsampling + 1;
int density_height = height/densityBufferDownsampling + 1;

const float dragCoefficient = 0.13;
const float ditherCoefficient = 0.08;

// Alpha blending of each of the fragments
const float trailIntensity = 0.06f;
const float trailAlpha = 0.85f;
// const float trailAlpha = 0.90f;
const float trailRadius = 15.0f;
const int trailBufferDownsampling = 5;
// const int trailBufferDownsampling = 20;
const float trailVelocityFloor = 0.6;
int trail_width = width/trailBufferDownsampling + 1;
int trail_height = height/trailBufferDownsampling + 1;

// Particles
// const int P = 16384; // <- render buffer max
// const int P = 30000;
// const int P = 160000;
const int P = 200000;
// const int P = 300000;
// const int P = 500000;
// const int P = 1000000; // emmmmm...

void window_setup();
void shader_setup();
void buffer_setup();
void updateShaderWindowShape(int width, int height);
void updateShaderBufferShape(int width, int height);
void saveFrame(const int epoch_counter, unsigned int width, unsigned int height, GLubyte **pixels);

//========================================
//                                        
//  ###    ###    ###    ##  ##     ##  
//  ## #  # ##   ## ##   ##  ####   ##  
//  ##  ##  ##  ##   ##  ##  ##  ## ##  
//  ##      ##  #######  ##  ##    ###  
//  ##      ##  ##   ##  ##  ##     ##  
//                                        
//========================================


int main() {
    window_setup();
    shader_setup();
    buffer_setup();

    // Write uniforms to shaders
    screenShader.setUniform("density_buffer", densityBuffer.current);
    densityShader.setUniform("density_buffer_downsampling", densityBufferDownsampling);
    densityShader.setUniform("density_alpha", densityAlpha);
    densityShader.setUniform("kernel_radius", kernelRadius);
    velocityShader.setUniform("drag_coefficient", dragCoefficient);
    velocityShader.setUniform("dither_coefficient", ditherCoefficient);
    velocityShader.setUniform("density_buffer", densityBuffer.current);
    copyShader.setUniform("alpha", trailAlpha);
    trailShader.setUniform("trail_buffer_downsampling", trailBufferDownsampling);
    trailShader.setUniform("trail_intensity", trailIntensity);
    trailShader.setUniform("velocity_floor", trailVelocityFloor);
    trailShader.setUniform("kernel_radius", trailRadius);

    double xpos; double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float mouse_position[] = {(float)xpos, (float)ypos};
    velocityShader.setUniform("mouse_position", 2, mouse_position);

    // Position and velocity double buffer pointers

    // Loop counter passed to the shaders for use in random()
    int epoch_counter = 0;

    // Physics timing preamble
    float exp_average_flip_time = 0.0f;
    float alpha_flip_time = 0.1;

    GLubyte* pixels = nullptr;


    //======================================
    //                                      
    //  ##       #####    #####   #####   
    //  ##      ##   ##  ##   ##  ##  ##  
    //  ##      ##   ##  ##   ##  #####   
    //  ##      ##   ##  ##   ##  ##      
    //  ######   #####    #####   ##      
    //                                      
    //======================================

    while (!glfwWindowShouldClose(window)) {

        // Poll mouse position
        glfwGetCursorPos(window, &xpos, &ypos);
        float mouse_position[] = {(float)xpos, (float)ypos};

        // Velocity pass
        velocityShader.use();
        velocityShader.setUniform("mouse_position", 2, mouse_position);
        velocityShader.setUniform("position_buffer", positionBuffer.current);
        velocityShader.setUniform("velocity_buffer", velocityBuffer.current);
        velocityShader.setUniform("trail_buffer", trailBuffer.current);
        velocityShader.setUniform("epoch_counter", epoch_counter);
        velocityBuffer.bind(other);
        velocityBuffer.update();
        velocityBuffer.flip_buffers();

        // Position pass
        positionShader.use();
        positionShader.setUniform("position_buffer", positionBuffer.current);
        positionShader.setUniform("velocity_buffer", velocityBuffer.current); // read from updated velocity buffer
        // positionShader.setUniform("epoch_counter", epoch_counter);
        positionBuffer.bind(other);
        // positionBuffer.update();
        positionBuffer.update();
        positionBuffer.flip_buffers();

        // Density buffer pass
        densityShader.use();
        densityShader.setUniform("position_buffer", positionBuffer.current);
        densityShader.setUniform("density_buffer_downsampling", densityBufferDownsampling);
        densityBuffer.bind(current);
        densityBuffer.update(P);
        
        // Trail buffer
        trailBuffer.bind(other);

        copyShader.use(); // First pass (alpha blend of the double buffer)
        copyShader.setUniform("alpha", trailAlpha);
        copyShader.setUniform("source_buffer", trailBuffer.current);
        trailBuffer.update();

        trailShader.use(); // Second pass
        trailShader.setUniform("position_buffer", positionBuffer.current);
        trailShader.setUniform("velocity_buffer", velocityBuffer.current);
        trailBuffer.update(P);
        trailBuffer.flip_buffers();

        // Screen rendering pass
        screenBuffer.bind(screen);
        
        screenShader.use();
        screenShader.setUniform("position_buffer", positionBuffer.current);
        screenShader.setUniform("velocity_buffer", velocityBuffer.current);
        screenShader.setUniform("epoch_counter", epoch_counter);
        screenBuffer.update(P);

        // View density buffer
        // copyShader.use();
        // copyShader.setUniform("source_buffer", densityBuffer.current);
        // copyShader.setUniform("alpha", 1.0f);
        // screenBuffer.update();

        // View trail buffer
        // copyShader.use();
        // copyShader.setUniform("source_buffer", trailBuffer.current);
        // copyShader.setUniform("alpha", 1.0f);
        // screenBuffer.update();

        float flip_buffer_start = glfwGetTime();
        glfwSwapBuffers(window); // Swap draw and screen buffer
        // saveFrame(epoch_counter, width, height, &pixels); // Save frame
        float delta_flip_time = (glfwGetTime()-flip_buffer_start)*1000;
        exp_average_flip_time == 0.0f
            ? exp_average_flip_time = delta_flip_time
            : exp_average_flip_time = alpha_flip_time*delta_flip_time + (1-alpha_flip_time)*exp_average_flip_time;
        fprintf(stdout, "epoch: %03d buffer flip time: %.2f ms\n", epoch_counter, exp_average_flip_time);

        glfwPollEvents();
        epoch_counter++;
    }

    // Clean up
    // TODO: clean up after oneself better
    free(pixels);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteTextures(7, textures);
    glDeleteFramebuffers(7, framebuffers);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

//============================================================
//                                                            
//  ##      ##  ##  ##     ##  ####     #####   ##      ##  
//  ##      ##  ##  ####   ##  ##  ##  ##   ##  ##      ##  
//  ##  ##  ##  ##  ##  ## ##  ##  ##  ##   ##  ##  ##  ##  
//  ##  ##  ##  ##  ##    ###  ##  ##  ##   ##  ##  ##  ##  
//   ###  ###   ##  ##     ##  ####     #####    ###  ###   
//                                                            
//============================================================

void window_setup() {
    fprintf(stdout, "Setting up glfw window...\n");

    // Setup window
    glfwSetErrorCallback([](int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    });

    if (!glfwInit()) { exit(EXIT_FAILURE); }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER,GL_FALSE);
#endif
    // glfwWindowHint(GLFW_DECORATED,GLFW_FALSE);
    // glfwWindowHint(GLFW_MAXIMIZED,GLFW_TRUE);

    // glfwWindowHint(GLFW_SAMPLES,GLFW_FALSE); // Disable multisampling


    GLFWmonitor* monitor;
    if (fullscreen) {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        std::cout << monitorCount << " monitors found" << std::endl;
        if (whichMonitor >= monitorCount) { whichMonitor = 0; };
        std::cout << "using monitor " << whichMonitor << std::endl;
        monitor = monitors[whichMonitor];
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;
        std::cout << width << " " << height << std::endl;
        density_width = width/densityBufferDownsampling + 1;
        density_height = height/densityBufferDownsampling + 1;
        trail_width = width/trailBufferDownsampling + 1;
        trail_height = height/trailBufferDownsampling + 1;
    } else {
        monitor = nullptr;
    }

    window = glfwCreateWindow(width, height, title, monitor, nullptr);

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        exit(EXIT_FAILURE);
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(0); // Disable vsync
    glfwSwapInterval(1); // Enable vsync
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int new_width, int new_height) {
        std::cout << "glfwSetFramebufferSizeCallback()" << std::endl;
        glViewport(0, 0, new_width, new_height);
        width = new_width;
        height = new_height;
        updateShaderWindowShape(width, height);
        density_width = width/densityBufferDownsampling + 1;
        density_height = height/densityBufferDownsampling + 1;
        densityBuffer.reallocate(current, density_width, density_height);
        trail_width = width/trailBufferDownsampling + 1;
        trail_height = height/trailBufferDownsampling + 1;
        trailBuffer.reallocate(current, trail_width, trail_height);
        trailBuffer.reallocate(other, trail_width, trail_height);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            std::cout << key << std::endl;
            if (key == GLFW_KEY_ESCAPE) {glfwSetWindowShouldClose(window, true);}
        }
    });

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(EXIT_FAILURE);
    }

    // Enable point size rendering and alpha blending
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//=====================================================
//                                                     
//   ####  ##   ##    ###    ####    #####  #####    
//  ##     ##   ##   ## ##   ##  ##  ##     ##  ##   
//   ###   #######  ##   ##  ##  ##  #####  #####    
//     ##  ##   ##  #######  ##  ##  ##     ##  ##   
//  ####   ##   ##  ##   ##  ####    #####  ##   ##  
//                                                     
//=====================================================

void shader_setup() {
    fprintf(stdout, "Compiling shaders...\n");
    screenShader.create();
    screenShader.compile(GL_VERTEX_SHADER, screen_VertexShaderSource);
    screenShader.compile(GL_FRAGMENT_SHADER, screen_FragmentShaderSource);
    screenShader.link();

    densityShader.create();
    densityShader.compile(GL_VERTEX_SHADER, density_VertexShaderSource);
    densityShader.compile(GL_FRAGMENT_SHADER, density_FragmentShaderSource);
    densityShader.link();

    positionShader.create();
    positionShader.compile(GL_VERTEX_SHADER, position_VertexShaderSource);
    positionShader.compile(GL_FRAGMENT_SHADER, position_FragmentShaderSource);
    positionShader.link();

    velocityShader.create();
    velocityShader.compile(GL_VERTEX_SHADER, velocity_VertexShaderSource);
    velocityShader.compile(GL_FRAGMENT_SHADER, velocity_FragmentShaderSource);
    velocityShader.link();

    copyShader.create();
    copyShader.compile(GL_VERTEX_SHADER, copy_VertexShaderSource);
    copyShader.compile(GL_FRAGMENT_SHADER, copy_FragmentShaderSource);
    copyShader.link();

    trailShader.create();
    trailShader.compile(GL_VERTEX_SHADER, trail_VertexShaderSource);
    trailShader.compile(GL_FRAGMENT_SHADER, trail_FragmentShaderSource);
    trailShader.link();

    updateShaderWindowShape(width, height);
}

void updateShaderWindowShape(int new_width, int new_height) {
    float window_shape[2] = {(float)new_width, (float)new_height};
    screenShader.setUniform("window_shape", 2, window_shape);
    densityShader.setUniform("window_shape", 2, window_shape);
    positionShader.setUniform("window_shape", 2, window_shape);
    velocityShader.setUniform("window_shape", 2, window_shape);
    // copyShader.setUniform("window_shape", 2, window_shape);
    trailShader.setUniform("window_shape", 2, window_shape);
    screenBuffer.width = new_width;
    screenBuffer.height = new_height;
}

void updateShaderBufferShape(int new_width, int new_height) {
    float buffer_shape[2] = {(float)new_width, (float)new_height};
    screenShader.setUniform("buffer_size", 2, buffer_shape);
    densityShader.setUniform("buffer_size", 2, buffer_shape);
    // positionShader.setUniform("buffer_size", 2, buffer_shape);
    // velocityShader.setUniform("buffer_size", 2, buffer_shape);
    // copyShader.setUniform("buffer_size", 2, buffer_shape);
    trailShader.setUniform("buffer_size", 2, buffer_shape);
}

//===================================================
//                                                   
//  #####   ##   ##  #####  #####  #####  #####    
//  ##  ##  ##   ##  ##     ##     ##     ##  ##   
//  #####   ##   ##  #####  #####  #####  #####    
//  ##  ##  ##   ##  ##     ##     ##     ##  ##   
//  #####    #####   ##     ##     #####  ##   ##  
//                                                   
//===================================================


void buffer_setup() {
    fprintf(stdout, "Settgin up buffers...\n");
    // Setup vertex array
    int* vertices = (int*) malloc(2*P*sizeof(int));
    for (int i = 0; i < 2*P; i++) { vertices[i] = 0; }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 2*P*sizeof(int), vertices, GL_STATIC_DRAW);
    free(vertices);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Calculate size of the physics buffer
    int PBwidth = ceil(sqrt(P));
    int PBheight = ceil(P/sqrt(P));
    fprintf(stdout, "%d points\n", P);
    fprintf(stdout, "Physics framebuffer shape: %d x %d\n", PBwidth, PBheight);
    updateShaderBufferShape(PBwidth, PBheight);

    // Initalise textures and the associated framebuffers
    glGenTextures(7, textures);
    glGenFramebuffers(7, framebuffers);

    // Texture 0 - Density buffer
    densityBuffer.minmag_filter = GL_NEAREST; // GL_LINEAR
    densityBuffer.wrap_st = GL_REPEAT;
    densityBuffer.dim = BE_1D;
    densityBuffer.allocate(current, densityBufferIndex, density_width, density_height, NULL);

    positionBuffer.minmag_filter = GL_NEAREST;
    positionBuffer.dim = BE_2D;
    velocityBuffer.minmag_filter = GL_NEAREST;
    velocityBuffer.dim = BE_2D;

    // Texture 1 - Position buffer 1
    float margin = glm::min(width, height)*0.1;
    float noise_seed = 10*glfwGetTime() + glm::linearRand<float>(0, 1);
    fprintf(stdout, "Generating starting positions...\n");
    int N = PBwidth*PBheight*2;
    float* positions = new float[N];
    for (int i = 0; i < N; i += 2) {
        // Generate random position in unit range
        glm::vec2 position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        glm::vec2 noise = glm::vec2(
                glm::perlin( glm::vec3( 10*position.x, 10*position.y, noise_seed)),
                glm::perlin( glm::vec3( 10*position.x, 10*position.y, noise_seed+1))
            );
        position += 0.1f * glm::mod(noise , 1.0f);

        // Cull position to the center of the screen
        position *= ( glm::vec2( width, height ) - 2*margin );
        position += margin;

        positions[i] = position.x;
        positions[i+1] = position.y;
    }

    fprintf(stdout, "Allocating buffers...\n");
    positionBuffer.allocate(current, positionBufferIndex1, PBwidth, PBheight, (const char*) positions);
    delete[] positions;

    // Texture 2,3,4 - position buffer 2, velocity buffer 1 and 2
    positionBuffer.allocate(other, positionBufferIndex2, PBwidth, PBheight, NULL);
    velocityBuffer.allocate(current, velocityBufferIndex1, PBwidth, PBheight, NULL);
    velocityBuffer.allocate(other, velocityBufferIndex2, PBwidth, PBheight, NULL);

    // Texture 5,6 - Trail double buffer
    trailBuffer.minmag_filter = GL_NEAREST;
    trailBuffer.wrap_st = GL_REPEAT;
    trailBuffer.dim = BE_1D;
    trailBuffer.allocate(current, trailBufferIndex1, trail_width, trail_height, NULL);
    trailBuffer.allocate(other, trailBufferIndex2,  trail_width, trail_height, NULL);
}


//====================================
//                                    
//   ####    ###    ##   ##  #####  
//  ##      ## ##   ##   ##  ##     
//   ###   ##   ##  ##   ##  #####  
//     ##  #######   ## ##   ##     
//  ####   ##   ##    ###    #####  
//                                    
//====================================

void saveFrame(const int epoch_counter, unsigned int width, unsigned int height, GLubyte** pixels) {

    // Construct filename
    std::string epoch_string = "";
    if (epoch_counter <= 9999) {epoch_string += "0";};
    if (epoch_counter <= 999) {epoch_string += "0";};
    if (epoch_counter <= 99) {epoch_string += "0";};
    if (epoch_counter <= 9) {epoch_string += "0";};
    epoch_string += std::to_string(epoch_counter);
    std::string filename = "./frames/" + epoch_string + ".png";

    size_t i, j, cur;
    const size_t format_nchannels = 3;

    std::ofstream fout;
    fout.open(filename.c_str(), std::ios::binary | std::ios::out);
    char header[1024];
    size_t header_size = snprintf(header, sizeof(header), "P6\n%d %d\n%d\n", width, height, 255);
    fout.write(header, header_size);
    
    size_t pixels_s = format_nchannels * sizeof(GLubyte) * width * height;
    *pixels = (GLubyte*) realloc(*pixels, pixels_s);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
    fout.write((char*) *pixels, pixels_s);

    fout.close();
}
