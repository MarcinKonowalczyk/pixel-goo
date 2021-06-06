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
int width = 800;
int height = 600;
const char* title = "Goolets";
bool fullscreen = false;

// Screen shader
GLuint screenRenderingShader;
extern const GLchar* screenVertexShaderSource;
extern const GLchar* screenFragmentShaderSource;
#include "screen.vert"
#include "screen.frag"

// Density Map shader
GLuint densityMapShader;
extern const GLchar* densityVertexShaderSource;
extern const GLchar* densityFragmentShaderSource;
#include "density.vert"
#include "density.frag"

// Position shader
GLuint positionShader;
extern const GLchar* positionVertexShaderSource;
extern const GLchar* positionFragmentShaderSource;
#include "position.vert"
#include "position.frag"

// Particles
const int P = 16384;
std::array<glm::vec2, P> positions;

void window_setup();
void shader_setup();
void updateShaderWidthHeightUniforms(int width, int height);

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

    // Setup vertex array
    std::array<glm::vec2, P> vertices;
    for (glm::vec2& vertex : vertices) {
        vertex = glm::vec2(0,0);
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint buf[1];
    glGenBuffers(1, buf);
    
    glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    updateShaderWidthHeightUniforms(width, height);

    GLuint textures[3];
    glGenTextures(3, textures);
    GLuint framebuffers[3];
    glGenFramebuffers(3, framebuffers);

    // Texture 0 - Density map
    const int densityMapTextureIndex = 0;
    glActiveTexture(GL_TEXTURE0 + densityMapTextureIndex);
    glBindTexture(GL_TEXTURE_2D, textures[densityMapTextureIndex]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    // Bind the density map to a frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[densityMapTextureIndex]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[densityMapTextureIndex], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }

    GLint max_renderbuffer_size;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size); 
    if (P > max_renderbuffer_size) {
        std::cerr << "number of particles (" << P << ") larger than renderbuffer size (" << max_renderbuffer_size << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Texture 1 - Position buffer 1
    const int positionBuffer_1 = 1;
    glActiveTexture(GL_TEXTURE0 + positionBuffer_1);
    glBindTexture(GL_TEXTURE_1D, textures[positionBuffer_1]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float margin = glm::min(width,height)*0.05;
    for (glm::vec2& position : positions) {
        // 
        position = glm::clamp( glm::vec2( glm::gaussRand<float>(0.5, 0.5), glm::gaussRand<float>(0.5, 0.5)), 0.0f, 1.0f );
        // position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        position *= ( glm::vec2( width, height ) - 2*margin );
        position += margin;
    }
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, P, 0, GL_RG, GL_FLOAT, positions.data());

    // Bind the position map to a frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[positionBuffer_1]);
    glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, textures[positionBuffer_1], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }

    // Texture 2 - position buffer 2
    const int positionBuffer_2 = 2;
    glActiveTexture(GL_TEXTURE0 + positionBuffer_2);
    glBindTexture(GL_TEXTURE_1D, textures[positionBuffer_2]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::array<glm::vec2, P> positions2;
    for (glm::vec2& position : positions2) {
        // position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
        position = glm::vec2(0,0);
    }
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, P, 0, GL_RG, GL_FLOAT, positions2.data());

    // Bind the velocity map to a frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[positionBuffer_2]);
    glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, textures[positionBuffer_2], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }

    // Tell shader about the positions of the textures
    glUseProgram(screenRenderingShader);
    glUniform1i(glGetUniformLocation(screenRenderingShader, "density_map"), densityMapTextureIndex);

    int currentPositionBuffer = positionBuffer_1; // Start by using buffer 1
    int otherPositionBuffer = positionBuffer_2;

    // glBindTexture(GL_TEXTURE_1D, textures[currentPositionBuffer]);
    // DEBUG_printPositionTexture("[before] currentPositionBuffer", 5);
    glBindTexture(GL_TEXTURE_1D, textures[otherPositionBuffer]);
    DEBUG_printPositionTexture("[before] otherPositionBuffer", 2, 10);


    glUseProgram(positionShader);
    glUniform1i(glGetUniformLocation(positionShader, "position_buffer"), currentPositionBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherPositionBuffer]);
    glViewport(0, 0, P, 1);
    // glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // glDrawArrays(GL_POINTS, 0, P);

    // glBindTexture(GL_TEXTURE_1D, textures[currentPositionBuffer]);
    // DEBUG_printPositionTexture("[after] currentPositionBuffer", 5);
    glBindTexture(GL_TEXTURE_1D, textures[otherPositionBuffer]);
    DEBUG_printPositionTexture("[after] otherPositionBuffer", 2, 10);
    

    exit(1); // Stop

    // Physics timing preamble
    float exp_average_physics_time = 0.0f;
    float alpha_physics_time = 0.05;

    while (!glfwWindowShouldClose(window)) {
        
        // Physics timing begin
        float physics_time_start = glfwGetTime();
        
        // TODO:
        // glUseProgram(velocityShader);
        // glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[positionBuffer_2]);
        // glClear(GL_COLOR_BUFFER_BIT);
        // glDrawArrays(GL_POINTS, 0, P);

        // WIP:

        if (currentPositionBuffer == positionBuffer_1) {otherPositionBuffer = positionBuffer_2;} else {otherPositionBuffer = positionBuffer_1;}

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[currentPositionBuffer]);
        // DEBUG_printPositionTexture("current position buffer");


        glUseProgram(positionShader);
        glUniform1i(glGetUniformLocation(positionShader, "position_buffer"), currentPositionBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[otherPositionBuffer]);
        // DEBUG_printPositionTexture("other position buffer before shader run");

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);

        // DEBUG_printPositionTexture("other position buffer after shader run:");
        

        exit(1); // Stop

        // for (glm::vec2& position : positions) {
        //     position += glm::diskRand(3.0);
        // }
        // glActiveTexture(GL_TEXTURE0 + currentPositionBuffer);
        // glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, P, 0, GL_RG, GL_FLOAT, positions.data());

        // Physics timing end
        float delta_physics_time = (glfwGetTime()-physics_time_start)*1000;
        exp_average_physics_time == 0.0f
            ? exp_average_physics_time = delta_physics_time
            : exp_average_physics_time = alpha_physics_time*delta_physics_time + (1-alpha_physics_time)*exp_average_physics_time;
        std::cout << "physics time: " << exp_average_physics_time << "ms" << std::endl;
        
        // Density map pass
        glUseProgram(densityMapShader);
        glUniform1i(glGetUniformLocation(densityMapShader, "position_map"), currentPositionBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[densityMapTextureIndex]);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);

        // Screen rendering pass
        // glUseProgram(densityMapShader);
        glUseProgram(screenRenderingShader);
        glUniform1i(glGetUniformLocation(screenRenderingShader, "position_map"), currentPositionBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, P);

        // Swap draw and screen buffer
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Swap position buffers
        currentPositionBuffer = otherPositionBuffer;
    }

    // Clean up
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
#endif

    GLFWmonitor* primaryMonitor;
    if (fullscreen) {
        primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        width = mode->width;
        height = mode->height;
    } else {
        primaryMonitor = nullptr;
    }

    window = glfwCreateWindow(width, height, title, primaryMonitor, nullptr);

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        exit(EXIT_FAILURE);
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int new_width, int new_height) {
        glViewport(0, 0, new_width, new_height);
        width = new_width;
        height = new_height;
        updateShaderWidthHeightUniforms(width, height);
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
}

void updateShaderWidthHeightUniforms(int new_width, int new_height) {
    glUseProgram(screenRenderingShader);
    glUniform1f(glGetUniformLocation(screenRenderingShader, "window_width"), new_width);
    glUniform1f(glGetUniformLocation(screenRenderingShader, "window_height"), new_height);
    glUseProgram(densityMapShader);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_width"), new_width);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_height"), new_height);
    glUseProgram(positionShader);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_width"), new_width);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_height"), new_height);
}