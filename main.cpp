#include <iostream>

#include <array>
#include <vector>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>

// Window
GLFWwindow* window;
int width = 800; int height = 600;
// int width = 600; int height = 800;
const char* title = "Pixel Goo";
const bool fullscreen = false;
// const bool fullscreen = true;
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

// Screen shader
GLuint screenRenderingShader;
extern const GLchar* screenVertexShaderSource;
extern const GLchar* screenFragmentShaderSource;
#include "screen.vert"
#include "screen.frag"

// Density Map
GLuint densityMapShader;
extern const GLchar* densityVertexShaderSource;
extern const GLchar* densityFragmentShaderSource;
#include "density.vert"
#include "density.frag"
// Alpha blending of each of the fragments
const float densityAlpha = 0.005f;
// const float densityAlpha = 0.9f;
const float kernelRadius = 30.0f;

// This can be quite a lot because the density map is lerped and particles dither
const int densityMapDownsampling = 20;
int density_width = width/densityMapDownsampling + 1;
int density_height = height/densityMapDownsampling + 1;

// Position shader
GLuint positionShader;
extern const GLchar* positionVertexShaderSource;
extern const GLchar* positionFragmentShaderSource;
#include "position.vert"
#include "position.frag"

// Velocity shader
GLuint velocityShader;
extern const GLchar* velocityVertexShaderSource;
extern const GLchar* velocityFragmentShaderSource;
#include "velocity.vert"
#include "velocity.frag"

const float dragCoefficient = 0.1;
const float ditherCoefficient = 0.08;

// Trail (double) Map
GLuint trailFirstShader;
extern const GLchar* trailFirstVertexShaderSource;
extern const GLchar* trailFirstFragmentShaderSource;
#include "trail_first.vert"
#include "trail_first.frag"
GLuint trailSecondShader;
extern const GLchar* trailSecondVertexShaderSource;
extern const GLchar* trailSecondFragmentShaderSource;
#include "trail_second.vert"
#include "trail_second.frag"
// Alpha blending of each of the fragments
const float trailIntensity = 0.06f;
const float trailAlpha = 0.90f;
const float trailRadius = 30.0f;
const int trailMapDownsampling = 10;
const float trailVelocityFloor = 0.8;
int trail_width = width/trailMapDownsampling + 1;
int trail_height = height/trailMapDownsampling + 1;

// Particles
// const int P = 11;
// const int P = 5000;
// const int P = 16384; // <- render buffer max
const int P = 100000; // emmmmm...
// const int P = 200000; // emmmmm...

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

void DEBUG_printPositionTexture(const char* message, const int N = 3, const int M = 3) {
    float pixels[2*P];
    std::cout << message << ":" << std::endl;
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RG, GL_FLOAT, &pixels);
    for (int i = 0; i < 2*N; i += 2) {
        std::cout << " " << i/2 << ": " << pixels[i] << " " << pixels[i+1] << std::endl;
    }
    std::cout << "..." << std::endl;
    for (int i = 2*P-(2*M); i < 2*P; i += 2) {
        std::cout << " " << 2*P-(2*P-i)/2 << ": " << pixels[i] << " " << pixels[i+1] << std::endl;
    }
    std::cout << " " << std::endl;
}

int main() {
    window_setup();
    shader_setup();
    // buffer_setup();

    // Setup vertex array
    std::array<glm::vec2, P> vertices;
    for (glm::vec2& vertex : vertices) {
        vertex = glm::vec2(0,0);
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
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
    float margin = glm::min(width,height)*0.05;
    float noise_seed = 10*glfwGetTime() + glm::linearRand<float>(0, 1);
    std::array<glm::vec2, P> positions;
    for (glm::vec2& position : positions) {
        // position = glm::clamp( glm::vec2( glm::gaussRand<float>(0.5, 0.5), glm::gaussRand<float>(0.5, 0.5)), 0.0f, 1.0f );
        // position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        float noise_value;
        do {
            position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
            noise_value = glm::perlin( glm::vec3( position.x*4, position.y*4, noise_seed)) + 0.5;
            noise_value = glm::clamp( noise_value, 0.2f, 1.0f);
        } while (glm::linearRand<float>(0, 1) < noise_value);
        position *= ( glm::vec2( width, height ) - 2*margin );
        position += margin;
    }
    allocatePhysicsBuffer(positionBufferIndex1, (const char*) positions.data());

    // Texture 2,3,4 - position buffer 2, velocity buffer 1 and 2
    std::array<glm::vec2, P> emptyPhysicsBuffer;
    for (glm::vec2& position : emptyPhysicsBuffer) { position = glm::vec2(0,0); }
    allocatePhysicsBuffer(positionBufferIndex2, (const char*) emptyPhysicsBuffer.data());
    allocatePhysicsBuffer(velocityBufferIndex1, (const char*) emptyPhysicsBuffer.data());
    allocatePhysicsBuffer(velocityBufferIndex2, (const char*) emptyPhysicsBuffer.data());

    // Texture 5,6 - Trail double map
    allocateMapBuffer(trailMapIndex1, trail_width, trail_height);
    allocateMapBuffer(trailMapIndex2, trail_width, trail_height);

    // Write uniforms to shaders
    glUseProgram(screenRenderingShader);
    glUniform1i(glGetUniformLocation(screenRenderingShader, "density_map"), densityMapIndex);
    glUniform1i(glGetUniformLocation(screenRenderingShader, "density_map_downsampling"), densityMapDownsampling);
    glUseProgram(densityMapShader);
    glUniform1i(glGetUniformLocation(densityMapShader, "density_map_downsampling"), densityMapDownsampling);
    glUniform1f(glGetUniformLocation(densityMapShader, "density_alpha"), densityAlpha);
    glUniform1f(glGetUniformLocation(densityMapShader, "kernel_radius"), kernelRadius);
    glUseProgram(velocityShader);
    glUniform1f(glGetUniformLocation(velocityShader, "drag_coefficient"), dragCoefficient);
    glUniform1f(glGetUniformLocation(velocityShader, "dither_coefficient"), ditherCoefficient);
    glUniform1i(glGetUniformLocation(velocityShader, "density_map"), densityMapIndex);
    glUniform1i(glGetUniformLocation(velocityShader, "density_map_downsampling"), densityMapDownsampling);
    glUniform1i(glGetUniformLocation(velocityShader, "trail_map_downsampling"), trailMapDownsampling);
    glUseProgram(trailFirstShader);
    glUniform1f(glGetUniformLocation(trailFirstShader, "alpha"), trailAlpha);
    glUseProgram(trailSecondShader);
    glUniform1i(glGetUniformLocation(trailSecondShader, "trail_map_downsampling"), trailMapDownsampling);
    glUniform1f(glGetUniformLocation(trailSecondShader, "trail_intensity"), trailIntensity);
    glUniform1f(glGetUniformLocation(trailSecondShader, "velocity_floor"), trailVelocityFloor);
    glUniform1f(glGetUniformLocation(trailSecondShader, "kernel_radius"), trailRadius);

    double xpos; double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float mouse_position[] = {(float)xpos, (float)ypos};
    glUseProgram(velocityShader);
    glUniform2fv(glGetUniformLocation(velocityShader, "mouse_position"), 1, mouse_position);

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
        glUseProgram(velocityShader);
        glUniform2fv(glGetUniformLocation(velocityShader, "mouse_position"), 1, mouse_position);
        glUniform1i(glGetUniformLocation(positionShader, "position_buffer"), currentPositionBuffer);
        glUniform1i(glGetUniformLocation(velocityShader, "velocity_buffer"), currentVelocityBuffer);
        glUniform1i(glGetUniformLocation(velocityShader, "trail_map"), currentTrailMap);
        glUniform1i(glGetUniformLocation(velocityShader, "epoch_counter"), epoch_counter);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherVelocityBuffer]);
        glViewport(0, 0, physicsBufferWidth, physicsBufferHeight);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Position pass
        glUseProgram(positionShader);
        glUniform1i(glGetUniformLocation(positionShader, "position_buffer"), currentPositionBuffer);
        glUniform1i(glGetUniformLocation(positionShader, "velocity_buffer"), otherVelocityBuffer); // read from updated velocity buffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherPositionBuffer]);
        glViewport(0, 0, physicsBufferWidth, physicsBufferHeight); // Change the viewport to the size of the 1D texture vector
        glClear(GL_COLOR_BUFFER_BIT); // Dont need to clear it as its writing to each pixel anyway
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Density map pass
        glUseProgram(densityMapShader);
        glUniform1i(glGetUniformLocation(densityMapShader, "position_buffer"), otherPositionBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[densityMapIndex]);
        glViewport(0, 0, density_width, density_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);
        
        // Trail map
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[currentTrailMap]);
        glViewport(0, 0, trail_width, trail_height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(trailFirstShader); // First pass (alpha blend of the double buffer)
        glUniform1f(glGetUniformLocation(trailFirstShader, "alpha"), trailAlpha);
        glUniform1i(glGetUniformLocation(trailFirstShader, "previous_trail_map"), otherTrailMap);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Need to only write 4 points since vertex shader makes them into a quad over the entire screen anyway
        glUseProgram(trailSecondShader); // Second pass
        glUniform1i(glGetUniformLocation(trailSecondShader, "position_buffer"), otherPositionBuffer);
        glUniform1i(glGetUniformLocation(trailSecondShader, "velocity_buffer"), otherVelocityBuffer);
        glDrawArrays(GL_POINTS, 0, P);

        // Screen rendering pass
        glUseProgram(screenRenderingShader);
        glUniform1i(glGetUniformLocation(screenRenderingShader, "position_buffer"), otherPositionBuffer);
        glUniform1i(glGetUniformLocation(screenRenderingShader, "velocity_buffer"), otherVelocityBuffer);

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

        // float poll_events_start = glfwGetTime();
        glfwPollEvents();
        // std::cout << "poll events time: " << (glfwGetTime()-poll_events_start)*1000 << "ms" << std::endl;

        // Swap double buffers
        currentPositionBuffer = otherPositionBuffer;
        currentVelocityBuffer = otherVelocityBuffer;
        currentTrailMap = otherTrailMap;
        epoch_counter++;
    }

    // Clean up
    free(pixels);
    glBindVertexArray(0);
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

#define INFOLOG_LEN 512

void compileAndAttachShader(const GLchar* shaderSource, const GLuint shaderTypeEnum, GLuint program) {
    GLint shader = glCreateShader(shaderTypeEnum);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[INFOLOG_LEN];
        glGetShaderInfoLog(shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void linkShaderProgram(GLuint program) {
    GLint success;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[INFOLOG_LEN];
        glGetProgramInfoLog(program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
}

void shader_setup() {
    screenRenderingShader = glCreateProgram();
    compileAndAttachShader(screenVertexShaderSource, GL_VERTEX_SHADER, screenRenderingShader);
    compileAndAttachShader(screenFragmentShaderSource, GL_FRAGMENT_SHADER, screenRenderingShader);
    linkShaderProgram(screenRenderingShader);

    densityMapShader = glCreateProgram();
    compileAndAttachShader(densityVertexShaderSource, GL_VERTEX_SHADER, densityMapShader);
    compileAndAttachShader(densityFragmentShaderSource, GL_FRAGMENT_SHADER, densityMapShader);
    linkShaderProgram(densityMapShader);

    positionShader = glCreateProgram();
    compileAndAttachShader(positionVertexShaderSource, GL_VERTEX_SHADER, positionShader);
    compileAndAttachShader(positionFragmentShaderSource, GL_FRAGMENT_SHADER, positionShader);
    linkShaderProgram(positionShader);

    velocityShader = glCreateProgram();
    compileAndAttachShader(velocityVertexShaderSource, GL_VERTEX_SHADER, velocityShader);
    compileAndAttachShader(velocityFragmentShaderSource, GL_FRAGMENT_SHADER, velocityShader);
    linkShaderProgram(velocityShader);

    trailFirstShader = glCreateProgram();
    compileAndAttachShader(trailFirstVertexShaderSource, GL_VERTEX_SHADER, trailFirstShader);
    compileAndAttachShader(trailFirstFragmentShaderSource, GL_FRAGMENT_SHADER, trailFirstShader);
    linkShaderProgram(trailFirstShader);

    trailSecondShader = glCreateProgram();
    compileAndAttachShader(trailSecondVertexShaderSource, GL_VERTEX_SHADER, trailSecondShader);
    compileAndAttachShader(trailSecondFragmentShaderSource, GL_FRAGMENT_SHADER, trailSecondShader);
    linkShaderProgram(trailSecondShader);
}

void updateShaderWindowShape(int new_width, int new_height) {
    float window_shape[2] = {(float)new_width, (float)new_height};
    glUseProgram(screenRenderingShader);
    glUniform2fv(glGetUniformLocation(screenRenderingShader, "window_size"), 1, window_shape);
    glUseProgram(densityMapShader);
    glUniform2fv(glGetUniformLocation(densityMapShader, "window_size"), 1, window_shape);
    glUseProgram(positionShader);
    glUniform2fv(glGetUniformLocation(positionShader, "window_size"), 1, window_shape);
    glUseProgram(velocityShader);
    glUniform2fv(glGetUniformLocation(velocityShader, "window_size"), 1, window_shape);
    // glUseProgram(trailFirstShader);
    // glUniform2fv(glGetUniformLocation(trailFirstShader, "window_size"), 1, window_shape);
    glUseProgram(trailSecondShader);
    glUniform2fv(glGetUniformLocation(trailSecondShader, "window_size"), 1, window_shape);
}

void updateShaderPhysicsBufferShape(int new_width, int new_height) {
    float buffer_shape[2] = {(float)new_width, (float)new_height};
    glUseProgram(screenRenderingShader);
    glUniform2fv(glGetUniformLocation(screenRenderingShader, "physics_buffer_size"), 1, buffer_shape);
    glUseProgram(densityMapShader);
    glUniform2fv(glGetUniformLocation(densityMapShader, "physics_buffer_size"), 1, buffer_shape);
    glUseProgram(positionShader);
    glUniform2fv(glGetUniformLocation(positionShader, "physics_buffer_size"), 1, buffer_shape);
    glUseProgram(velocityShader);
    glUniform2fv(glGetUniformLocation(velocityShader, "physics_buffer_size"), 1, buffer_shape);
    // glUseProgram(trailFirstShader);
    // glUniform2fv(glGetUniformLocation(trailFirstShader, "physics_buffer_size"), 1, buffer_shape);
    glUseProgram(trailSecondShader);
    glUniform2fv(glGetUniformLocation(trailSecondShader, "physics_buffer_size"), 1, buffer_shape);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    FILE *f = fopen(filename.c_str(), "w");
    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    // fprintf(f, "BM\n%d %d\n%d\n", width, height, 255);
    *pixels = (GLubyte*) realloc(*pixels, format_nchannels * sizeof(GLubyte) * width * height);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur = format_nchannels * ((height - i - 1) * width + j);
            fprintf(f, "%3d %3d %3d ", (*pixels)[cur], (*pixels)[cur + 1], (*pixels)[cur + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}