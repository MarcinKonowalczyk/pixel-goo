#include "shader.h"

#include <iostream>

#include <array>
#include <vector>
#include <utility>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
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
// int whichMonitor = 0;
int whichMonitor = 1;

// Textures and framebuffers
GLuint textures[7];
GLuint framebuffers[7];
const int densityMapIndex = 0;
const int positionBufferIndex1 = 1;
const int positionBufferIndex2 = 2;
const int velocityBufferIndex1 = 3;
const int velocityBufferIndex2 = 4;
const int trailMapIndex1 = 5;
const int trailMapIndex2 = 6;

Shader screenRenderingShader("screenRenderingShader");
Shader densityMapShader("densityMapShader");
Shader positionShader("positionShader");
Shader velocityShader("velocityShader");
Shader trailFirstShader("trailFirstShader");
Shader trailSecondShader("trailSecondShader");

// Include shader source files
#include "trail_first.h"
#include "trail_second.h"
#include "screen.h"
#include "density.h"
#include "position.h"
#include "velocity.h"

// Alpha blending of each of the fragments
const float densityAlpha = 0.005f;
// const float densityAlpha = 0.9f;
const float kernelRadius = 30.0f;

// This can be quite a lot because the density map is lerped and particles dither
const int densityMapDownsampling = 10;
// const int densityMapDownsampling = 20;
int density_width = width/densityMapDownsampling + 1;
int density_height = height/densityMapDownsampling + 1;

const float dragCoefficient = 0.13;
const float ditherCoefficient = 0.08;

// Alpha blending of each of the fragments
const float trailIntensity = 0.06f;
const float trailAlpha = 0.85f;
// const float trailAlpha = 0.90f;
const float trailRadius = 15.0f;
const int trailMapDownsampling = 10;
// const int trailMapDownsampling = 20;
const float trailVelocityFloor = 0.7;
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
// const int P = 200000;
const int P = 300000;
// const int P = 1000000; // emmmmm...

int physicsBufferWidth; // = P
int physicsBufferHeight; // P/max_line_width

void window_setup();
void shader_setup();
void updateShaderWindowShape(int width, int height);
void updateShaderPhysicsBufferShape(int width, int height);
void allocateMapBuffer(const int mapIndex, const int width, const int height);
void allocatePhysicsBuffer(const int index, const char* data);
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
    allocateMapBuffer(densityMapIndex, density_width, density_height);

    // Texture 1 - Position buffer 1    
    float margin = glm::min(width,height)*0.08;
    // float box_edge = glm::min(width,height)*0.2;
    float noise_seed = 10*glfwGetTime() + glm::linearRand<float>(0, 1);
    // char* positions = (char*) malloc(2*P*sizeof(float));
    int N = physicsBufferWidth*physicsBufferHeight*2;
    float* positions = new float[N];
    for (int i = 0; i < N; i += 2) {
        glm::vec2 position;
        // glm::vec2 position = glm::clamp( glm::vec2( glm::gaussRand<float>(0.5, 0.5), glm::gaussRand<float>(0.5, 0.5)), 0.0f, 1.0f );
        position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        // float noise_value;
        // do {
        //     position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        //     // noise_value = glm::perlin( glm::vec3( position.x*4, position.y*4, noise_seed)) + 0.5;
        //     noise_value = 2*glm::perlin( glm::vec3( position.x*50, position.y*50, noise_seed));
        //     noise_value *= 10*glm::perlin( glm::vec3( position.x*4, position.y*4, noise_seed));
        //     noise_value = glm::clamp( noise_value, 0.1f, 1.0f);
        // } while (glm::linearRand<float>(0, 1) < noise_value);
        position *= ( glm::vec2( width, height ) - 2*margin );
        position += margin;

        // position = glm::diskRand(1.0) * (double)box_edge;
        // position = glm::vec2( glm::linearRand<float>(-1, 1), glm::linearRand<float>(-1, 1) )*box_edge;
        // position += glm::vec2(width/2,height/2);
        positions[i] = position.x;
        positions[i+1] = position.y;
    }

    allocatePhysicsBuffer(positionBufferIndex1, (const char*) positions);
    delete[] positions;

    // Texture 2,3,4 - position buffer 2, velocity buffer 1 and 2
    float* emptyPhysicsBuffer = new float[N];
    for (int i = 0; i < N; i++ ) { emptyPhysicsBuffer[i] = 0; }
    allocatePhysicsBuffer(positionBufferIndex2, (const char*) emptyPhysicsBuffer);
    allocatePhysicsBuffer(velocityBufferIndex1, (const char*) emptyPhysicsBuffer);
    allocatePhysicsBuffer(velocityBufferIndex2, (const char*) emptyPhysicsBuffer);
    delete[] emptyPhysicsBuffer;

    // Texture 5,6 - Trail double map
    allocateMapBuffer(trailMapIndex1, trail_width, trail_height);
    allocateMapBuffer(trailMapIndex2, trail_width, trail_height);

    // Write uniforms to shaders
    screenRenderingShader.setUniform("density_map", densityMapIndex);
    densityMapShader.setUniform("density_map_downsampling", densityMapDownsampling);
    densityMapShader.setUniform("density_alpha", densityAlpha);
    densityMapShader.setUniform("kernel_radius", kernelRadius);
    velocityShader.setUniform("drag_coefficient", dragCoefficient);
    velocityShader.setUniform("dither_coefficient", ditherCoefficient);
    velocityShader.setUniform("density_map", densityMapIndex);
    trailFirstShader.setUniform("alpha", trailAlpha);
    trailSecondShader.setUniform("trail_map_downsampling", trailMapDownsampling);
    trailSecondShader.setUniform("trail_intensity", trailIntensity);
    trailSecondShader.setUniform("velocity_floor", trailVelocityFloor);
    trailSecondShader.setUniform("kernel_radius", trailRadius);

    double xpos; double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float mouse_position[] = {(float)xpos, (float)ypos};
    velocityShader.setUniform("mouse_position", 2, mouse_position);

    // Position and velocity double buffer pointers
    int currentPositionBuffer = positionBufferIndex1; // Start by using buffer 1
    int otherPositionBuffer = positionBufferIndex2;
    int currentVelocityBuffer = velocityBufferIndex1;
    int otherVelocityBuffer = velocityBufferIndex2;
    int currentTrailMap = trailMapIndex1;
    int otherTrailMap = trailMapIndex2;

    // Loop counter passed to the shaders for use in random()
    int epoch_counter = 0;

    // Physics timing preamble
    float exp_average_flip_time = 0.0f;
    float alpha_flip_time = 0.1;

    GLubyte* pixels = nullptr;

    while (!glfwWindowShouldClose(window)) {

        otherPositionBuffer = currentPositionBuffer == positionBufferIndex1 ? positionBufferIndex2 : positionBufferIndex1;
        otherVelocityBuffer = currentVelocityBuffer == velocityBufferIndex1 ? velocityBufferIndex2 : velocityBufferIndex1;
        otherTrailMap = currentTrailMap == trailMapIndex1 ? trailMapIndex2 : trailMapIndex1;

        // Poll mouse position
        glfwGetCursorPos(window, &xpos, &ypos);
        float mouse_position[] = {(float)xpos, (float)ypos};

        // Velocity pass
        // velocityShader.use()
        velocityShader.setUniform("mouse_position", 2, mouse_position);
        velocityShader.setUniform("position_buffer", currentPositionBuffer);
        velocityShader.setUniform("velocity_buffer", currentVelocityBuffer);
        velocityShader.setUniform("trail_map", currentTrailMap);
        velocityShader.setUniform("epoch_counter", epoch_counter);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherVelocityBuffer]);
        glViewport(0, 0, physicsBufferWidth, physicsBufferHeight);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Position pass
        // positionShader.use()
        positionShader.setUniform("position_buffer", currentPositionBuffer);
        positionShader.setUniform("velocity_buffer", otherVelocityBuffer); // read from updated velocity buffer
        
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherPositionBuffer]);
        glViewport(0, 0, physicsBufferWidth, physicsBufferHeight); // Change the viewport to the size of the 1D texture vector
        glClear(GL_COLOR_BUFFER_BIT); // Dont need to clear it as its writing to each pixel anyway
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Density map pass
        // densityMapShader.use();
        densityMapShader.setUniform("position_buffer", otherPositionBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[densityMapIndex]);
        glViewport(0, 0, density_width, density_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);
        
        // Trail map
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[currentTrailMap]);
        glViewport(0, 0, trail_width, trail_height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // trailFirstShader.use() // First pass (alpha blend of the double buffer)
        trailFirstShader.setUniform("alpha", trailAlpha);
        trailFirstShader.setUniform("previous_trail_map", otherTrailMap);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Need to only write 4 points since vertex shader makes them into a quad over the entire screen anyway
        
        // trailSecondShader.use(); // Second pass
        trailSecondShader.setUniform("position_buffer", otherPositionBuffer);
        trailSecondShader.setUniform("velocity_buffer", otherVelocityBuffer);
        glDrawArrays(GL_POINTS, 0, P);

        // Screen rendering pass
        // screenRenderingShader.use()
        screenRenderingShader.setUniform("position_buffer", otherPositionBuffer);
        screenRenderingShader.setUniform("velocity_buffer", otherVelocityBuffer);

        // glUseProgram(densityMapShader);
        // glUniform1i(glGetUniformLocation(densityMapShader, "density_map_downsampling"), 1); // Turn off downsampling to render points on screen
        // glUniform1i(glGetUniformLocation(densityMapShader, "position_buffer"), positionBufferIndex1);

        // glUseProgram(trailFirstShader);
        // glUniform1i(glGetUniformLocation(trailFirstShader, "previous_trail_map"), currentTrailMap);
        // glUniform1f(glGetUniformLocation(trailFirstShader, "alpha"), 1.0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Flip timing start
        float flip_buffer_start = glfwGetTime();

        // Swap draw and screen buffer
        glfwSwapBuffers(window);

        // Save frame
        // saveFrame(epoch_counter, width, height, &pixels);

        // Flip timing end
        float delta_flip_time = (glfwGetTime()-flip_buffer_start)*1000;
        exp_average_flip_time == 0.0f
            ? exp_average_flip_time = delta_flip_time
            : exp_average_flip_time = alpha_flip_time*delta_flip_time + (1-alpha_flip_time)*exp_average_flip_time;
        std::cout << "epoch: " << epoch_counter << " buffer flip time: " << exp_average_flip_time << "ms" << std::endl;

        glfwPollEvents();

        // Swap double buffers
        currentPositionBuffer = otherPositionBuffer;
        currentVelocityBuffer = otherVelocityBuffer;
        currentTrailMap = otherTrailMap;
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
        density_width = width/densityMapDownsampling + 1;
        density_height = height/densityMapDownsampling + 1;
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
        density_width = width/densityMapDownsampling + 1;
        density_height = height/densityMapDownsampling + 1;
        allocateMapBuffer(densityMapIndex, density_width, density_height);
        trail_width = width/trailMapDownsampling + 1;
        trail_height = height/trailMapDownsampling + 1;
        allocateMapBuffer(trailMapIndex1, trail_width, trail_height);
        allocateMapBuffer(trailMapIndex2, trail_width, trail_height);
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

    densityMapShader.create();
    densityMapShader.compile(GL_VERTEX_SHADER, density_VertexShaderSource);
    densityMapShader.compile(GL_FRAGMENT_SHADER, density_FragmentShaderSource);
    densityMapShader.link();

    positionShader.create();
    positionShader.compile(GL_VERTEX_SHADER, position_VertexShaderSource);
    positionShader.compile(GL_FRAGMENT_SHADER, position_FragmentShaderSource);
    positionShader.link();

    velocityShader.create();
    velocityShader.compile(GL_VERTEX_SHADER, velocity_VertexShaderSource);
    velocityShader.compile(GL_FRAGMENT_SHADER, velocity_FragmentShaderSource);
    velocityShader.link();

    trailFirstShader.create();
    trailFirstShader.compile(GL_VERTEX_SHADER, trail_first_VertexShaderSource);
    trailFirstShader.compile(GL_FRAGMENT_SHADER, trail_first_FragmentShaderSource);
    trailFirstShader.link();

    trailSecondShader.create();
    trailSecondShader.compile(GL_VERTEX_SHADER, trail_second_VertexShaderSource);
    trailSecondShader.compile(GL_FRAGMENT_SHADER, trail_second_FragmentShaderSource);
    trailSecondShader.link();
}

void updateShaderWindowShape(int new_width, int new_height) {
    float window_shape[2] = {(float)new_width, (float)new_height};
    screenRenderingShader.setUniform("window_size", 2, window_shape);
    densityMapShader.setUniform("window_size", 2, window_shape);
    positionShader.setUniform("window_size", 2, window_shape);
    velocityShader.setUniform("window_size", 2, window_shape);
    // trailFirstShader.setUniform("window_size", 2, window_shape);
    trailSecondShader.setUniform("window_size", 2, window_shape);
}

void updateShaderPhysicsBufferShape(int new_width, int new_height) {
    float buffer_shape[2] = {(float)new_width, (float)new_height};
    screenRenderingShader.setUniform("physics_buffer_size", 2, buffer_shape);
    densityMapShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // positionShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // velocityShader.setUniform("physics_buffer_size", 2, buffer_shape);
    // trailFirstShader.setUniform("physics_buffer_size", 2, buffer_shape);
    trailSecondShader.setUniform("physics_buffer_size", 2, buffer_shape);
}

void allocateMapBuffer(const int mapIndex, const int width, const int height) {
    glActiveTexture(GL_TEXTURE0 + mapIndex);
    glBindTexture(GL_TEXTURE_2D, textures[mapIndex]);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);

    // Bind the density map to a frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[mapIndex]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[mapIndex], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }
}

void allocatePhysicsBuffer(const int index, const char* data) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, physicsBufferWidth, physicsBufferHeight, 0, GL_RG, GL_FLOAT, data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[index], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }
}

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