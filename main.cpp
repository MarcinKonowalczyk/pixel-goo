#include "shader.h"
#include "physics_buffer.h"

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
const bool fullscreen = false;
// const bool fullscreen = true;
// int whichMonitor = 0;
int whichMonitor = 1;

// Textures and framebuffers
GLuint textures[7];
GLuint framebuffers[7];
const PBindex densityBufferIndex = 0;
const PBindex positionBufferIndex1 = 1;
const PBindex positionBufferIndex2 = 2;
const PBindex velocityBufferIndex1 = 3;
const PBindex velocityBufferIndex2 = 4;
const PBindex trailMapIndex1 = 5;
const PBindex trailMapIndex2 = 6;

Shader screenRenderingShader("screenRenderingShader");
Shader densityShader("densityShader");
Shader positionShader("positionShader");
Shader velocityShader("velocityShader");
Shader trailShaderFirst("trailShaderFirst");
Shader trailShaderSecond("trailShaderSecond");

// Include shader source files
#include "trail_first.h"
#include "trail_second.h"
#include "screen.h"
#include "density.h"
#include "position.h"
#include "velocity.h"

PhysicsBuffer trailMap("Trail Map", textures, framebuffers);
PhysicsBuffer positionBuffer("Position buffer", textures, framebuffers);
PhysicsBuffer velocityBuffer("Velocity buffer", textures, framebuffers);
PhysicsBuffer densityBuffer("Density buffer", textures, framebuffers);

// Alpha blending of each of the fragments
const float densityAlpha = 0.005f;
// const float densityAlpha = 0.9f;
const float kernelRadius = 30.0f;

// This can be quite a lot because the density map is lerped and particles dither
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
const int trailMapDownsampling = 5;
// const int trailMapDownsampling = 20;
const float trailVelocityFloor = 0.6;
int trail_width = width/trailMapDownsampling + 1;
int trail_height = height/trailMapDownsampling + 1;

// Particles
// const float density = 200000/(1.0f*1920*1080);
// const int P = width*height*density;./b
// const int P = 11;
// const int P = 5000;
// const int P = 16384; // <- render buffer max
// const int P = 30000;
// const int P = 160000;
const int P = 200000;
// const int P = 300000;
// const int P = 500000;
// const int P = 1000000; // emmmmm...

int physicsBufferWidth; // = P
int physicsBufferHeight; // P/max_line_width

void window_setup();
void shader_setup();
void updateShaderWindowShape(int width, int height);
void updateShaderPhysicsBufferShape(int width, int height);
// void allocateMapBuffer(const int mapIndex, const int width, const int height);
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
    // buffer_setup();

    // Setup vertex array
    int* vertices = (int*) malloc(2*P*sizeof(int));
    for (int i = 0; i < 2*P; i++) {
        vertices[i] = 0;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 2*P*sizeof(int), vertices, GL_STATIC_DRAW);
    free(vertices);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    updateShaderWindowShape(width, height);

    // Calculate size of the physics buffer
    GLint max_renderbuffer_size;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size);
    physicsBufferWidth = ceil(sqrt(P));
    physicsBufferHeight = ceil(P/sqrt(P));
    if (physicsBufferWidth > max_renderbuffer_size) {
        std::cerr << "Physics framebuffer width (" << physicsBufferWidth << ") larger than renderbuffer size (" << max_renderbuffer_size << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (physicsBufferHeight > max_renderbuffer_size) {
        std::cerr << "Physics framebuffer height (" << physicsBufferHeight << ") larger than renderbuffer size (" << max_renderbuffer_size << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Physics framebuffer shape: (" << physicsBufferWidth << ", " << physicsBufferHeight << ")" << std::endl;
    updateShaderPhysicsBufferShape(physicsBufferWidth, physicsBufferHeight);

    // Initalise textures and the associated framebuffers
    glGenTextures(7, textures);
    glGenFramebuffers(7, framebuffers);

    // Texture 0 - Density map
    densityBuffer.minmag_filter = GL_LINEAR;
    densityBuffer.wrap_st = GL_REPEAT;
    densityBuffer.dim = PB_1D;
    densityBuffer.allocate(current, densityBufferIndex, density_width, density_height, NULL);

    positionBuffer.minmag_filter = GL_NEAREST;
    positionBuffer.dim = PB_2D;
    velocityBuffer.minmag_filter = GL_NEAREST;
    velocityBuffer.dim = PB_2D;

    // Texture 1 - Position buffer 1
    float margin = glm::min(width, height)*0.1;
    // float box_edge = glm::min(width,height)*0.2;
    float noise_seed = 10*glfwGetTime() + glm::linearRand<float>(0, 1);
    // char* positions = (char*) malloc(2*P*sizeof(float));
    int N = physicsBufferWidth*physicsBufferHeight*2;
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

    positionBuffer.allocate(current, positionBufferIndex1, physicsBufferWidth, physicsBufferHeight, (const char*) positions);
    delete[] positions;

    // Texture 2,3,4 - position buffer 2, velocity buffer 1 and 2
    // float* empty = new float[N];
    // for (int i = 0; i < N; i++ ) { empty[i] = 0; }
    positionBuffer.allocate(other, positionBufferIndex2, physicsBufferWidth, physicsBufferHeight, NULL);
    velocityBuffer.allocate(current, velocityBufferIndex1, physicsBufferWidth, physicsBufferHeight, NULL);
    velocityBuffer.allocate(other, velocityBufferIndex2, physicsBufferWidth, physicsBufferHeight, NULL);

    // Texture 5,6 - Trail double map
    trailMap.minmag_filter = GL_LINEAR;
    trailMap.wrap_st = GL_REPEAT;
    trailMap.dim = PB_1D;
    trailMap.allocate(current, trailMapIndex1, trail_width, trail_height, NULL);
    trailMap.allocate(other, trailMapIndex2,  trail_width, trail_height, NULL);
    // delete[] empty;

    // Write uniforms to shaders
    screenRenderingShader.setUniform("density_map", densityBuffer.current);
    densityShader.setUniform("density_map_downsampling", densityBufferDownsampling);
    densityShader.setUniform("density_alpha", densityAlpha);
    densityShader.setUniform("kernel_radius", kernelRadius);
    velocityShader.setUniform("drag_coefficient", dragCoefficient);
    velocityShader.setUniform("dither_coefficient", ditherCoefficient);
    velocityShader.setUniform("density_map", densityBuffer.current);
    trailShaderFirst.setUniform("alpha", trailAlpha);
    trailShaderSecond.setUniform("trail_map_downsampling", trailMapDownsampling);
    trailShaderSecond.setUniform("trail_intensity", trailIntensity);
    trailShaderSecond.setUniform("velocity_floor", trailVelocityFloor);
    trailShaderSecond.setUniform("kernel_radius", trailRadius);

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

    while (!glfwWindowShouldClose(window)) {

        // Poll mouse position
        glfwGetCursorPos(window, &xpos, &ypos);
        float mouse_position[] = {(float)xpos, (float)ypos};

        // Velocity pass
        // velocityShader.use()
        velocityShader.setUniform("mouse_position", 2, mouse_position);
        velocityShader.setUniform("position_buffer", positionBuffer.current);
        velocityShader.setUniform("velocity_buffer", velocityBuffer.current);
        velocityShader.setUniform("trail_map", trailMap.current);
        velocityShader.setUniform("epoch_counter", epoch_counter);
        velocityBuffer.bind(other);
        velocityBuffer.update();
        velocityBuffer.flip_buffers();

        // Position pass
        // positionShader.use()
        positionShader.setUniform("position_buffer", positionBuffer.current);
        positionShader.setUniform("velocity_buffer", velocityBuffer.current); // read from updated velocity buffer
        positionBuffer.bind(other);
        positionBuffer.update();
        positionBuffer.flip_buffers();

        // Density map pass
        // densityShader.use();
        densityShader.setUniform("position_buffer", positionBuffer.current);
        densityShader.setUniform("density_map_downsampling", densityBufferDownsampling);
        densityBuffer.bind(current);
        densityBuffer.update(P);
        
        // Trail map
        trailMap.bind(other);
        // trailShaderFirst.use() // First pass (alpha blend of the double buffer)
        trailShaderFirst.setUniform("alpha", trailAlpha);
        trailShaderFirst.setUniform("previous_trail_map", trailMap.current);
        trailMap.update();
        // trailShaderSecond.use(); // Second pass
        trailShaderSecond.setUniform("position_buffer", positionBuffer.current);
        trailShaderSecond.setUniform("velocity_buffer", velocityBuffer.current);
        trailMap.update(P);
        trailMap.flip_buffers();

        // Screen rendering pass
        // screenRenderingShader.use();
        screenRenderingShader.setUniform("position_buffer", positionBuffer.current);
        screenRenderingShader.setUniform("velocity_buffer", velocityBuffer.current);

        // Density map shader
        // densityShader.use();
        // densityShader.setUniform("position_buffer", positionBuffer.current);
        // densityShader.setUniform("density_map_downsampling", 10);

        // glUseProgram(trailShaderFirst);
        // glUniform1i(glGetUniformLocation(trailShaderFirst, "previous_trail_map"), currentTrailMap);
        // glUniform1f(glGetUniformLocation(trailShaderFirst, "alpha"), 1.0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        float flip_buffer_start = glfwGetTime();
        glfwSwapBuffers(window); // Swap draw and screen buffer
        // saveFrame(epoch_counter, width, height, &pixels); // Save frame
        float delta_flip_time = (glfwGetTime()-flip_buffer_start)*1000;
        exp_average_flip_time == 0.0f
            ? exp_average_flip_time = delta_flip_time
            : exp_average_flip_time = alpha_flip_time*delta_flip_time + (1-alpha_flip_time)*exp_average_flip_time;
        std::cout << "epoch: " << epoch_counter << " buffer flip time: " << exp_average_flip_time << "ms" << std::endl;

        glfwPollEvents();
        epoch_counter++;
    }

    // Clean up
    free(pixels);
    glBindVertexArray(0);
    glDeleteBuffers(1, &buf);
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
    std::cout << "Setting up glfw window..." << std::endl;

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
        trail_width = width/trailMapDownsampling + 1;
        trail_height = height/trailMapDownsampling + 1;
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
        trail_width = width/trailMapDownsampling + 1;
        trail_height = height/trailMapDownsampling + 1;
        trailMap.reallocate(current, trail_width, trail_height);
        trailMap.reallocate(other, trail_width, trail_height);
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
    screenRenderingShader.create();
    screenRenderingShader.compile(GL_VERTEX_SHADER, screen_VertexShaderSource);
    screenRenderingShader.compile(GL_FRAGMENT_SHADER, screen_FragmentShaderSource);
    screenRenderingShader.link();

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

    trailShaderFirst.create();
    trailShaderFirst.compile(GL_VERTEX_SHADER, trail_first_VertexShaderSource);
    trailShaderFirst.compile(GL_FRAGMENT_SHADER, trail_first_FragmentShaderSource);
    trailShaderFirst.link();

    trailShaderSecond.create();
    trailShaderSecond.compile(GL_VERTEX_SHADER, trail_second_VertexShaderSource);
    trailShaderSecond.compile(GL_FRAGMENT_SHADER, trail_second_FragmentShaderSource);
    trailShaderSecond.link();
}

void updateShaderWindowShape(int new_width, int new_height) {
    float window_shape[2] = {(float)new_width, (float)new_height};
    screenRenderingShader.setUniform("window_size", 2, window_shape);
    densityShader.setUniform("window_size", 2, window_shape);
    positionShader.setUniform("window_size", 2, window_shape);
    velocityShader.setUniform("window_size", 2, window_shape);
    // trailShaderFirst.setUniform("window_size", 2, window_shape);
    trailShaderSecond.setUniform("window_size", 2, window_shape);
}

void updateShaderPhysicsBufferShape(int new_width, int new_height) {
    float buffer_shape[2] = {(float)new_width, (float)new_height};
    screenRenderingShader.setUniform("physics_buffer_size", 2, buffer_shape);
    densityShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // positionShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // velocityShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // trailShaderFirst.setUniform("physics_buffer_size", 2, buffer_shape);
    trailShaderSecond.setUniform("physics_buffer_size", 2, buffer_shape);
}

// void allocateMapBuffer(const int mapIndex, const int width, const int height) {
//     glActiveTexture(GL_TEXTURE0 + mapIndex);
//     glBindTexture(GL_TEXTURE_2D, textures[mapIndex]);
//     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);

//     // Bind the density map to a frame buffer
//     glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[mapIndex]);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[mapIndex], 0);
//     if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//         fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
//         exit(EXIT_FAILURE);
//     }
// }

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
    // fclose(f);
}