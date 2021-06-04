#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <array>
#include <vector>
#include <utility>

#include <glad/glad.h> // OpenGL functions
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

// Shaders
GLuint shaderProgram;
extern const GLchar* vertexShaderSource;
extern const GLchar* fragmentShaderSource;
#include "shader.vert"
#include "shader.frag"

// Particles
const int P = 2000;
std::array<glm::vec2, P> positions;
// std::array<float, P> positions;

void window_setup();
void shader_setup();

void particles_setup() {

}

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

    for (glm::vec2& position : positions) {
        position = glm::vec2(
            glm::linearRand<float>(0+100, (float)width-100),
            glm::linearRand<float>(0+100, (float)height-100)
        );
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint buf[1];
    glGenBuffers(1, buf);
    
    glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // glBindBuffer(GL_ARRAY_BUFFER, buf[1]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(1); 
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_PROGRAM_POINT_SIZE);

    // Write window size to uniforms
    GLuint windowWidthLocation = glGetUniformLocation(shaderProgram, "window_width");
    glUniform1f(windowWidthLocation, width);
    GLuint windowHeightLocation = glGetUniformLocation(shaderProgram, "window_height");
    glUniform1f(windowHeightLocation, height);

    GLuint textures[1];
    glGenTextures(1, textures);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int image_width = width*2;
    int image_height = height*2;
    std::vector<float> image(3*image_width*image_height);
    for(int j = 0; j<image_height;++j) {
        for(int i = 0;i<image_width;++i) {
            size_t index = j*image_width + i;
            float noise_point = glm::clamp( glm::perlin(0.006f*glm::vec2(i,j+150)), 0.0f, 1.0f);
            image[3*index + 0] = i/(float)image_width; //glm::clamp( glm::perlin(0.006f*glm::vec2(i+0,j)), 0.0f, 1.0f);
            image[3*index + 1] = j/(float)image_height; //glm::clamp( glm::perlin(0.006f*glm::vec2(i+100,j)), 0.0f, 1.0f);
            image[3*index + 2] = noise_point;
        }
    }
    
    // set texture content
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image_width, image_height, 0, GL_RGB, GL_FLOAT, &image[0]);
    // glBindImageTexture(0, textures[1], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // opengl > 4.0 ; dont know what this does <shrug>

    // Physics timing preamble
    float exp_average_physics_time = 0.0f;
    float alpha_physics_time = 0.05;

    while (!glfwWindowShouldClose(window)) {
        
        glClear(GL_COLOR_BUFFER_BIT);

        // Physics timing begin
        float physics_time_start = glfwGetTime();

        auto first = positions.begin();
        auto last = positions.end();
        for(; first != last; ++first) {
            // for(auto next = std::next(first); next != last; ++next) {
            //     glm::length(*next-*first);
            // }
            *first += glm::diskRand(1.0);
        }

        // set texture content
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image_width, image_height, 0, GL_RGB, GL_FLOAT, &image[0]);

        // Physics timing end
        float delta_physics_time = (glfwGetTime()-physics_time_start)*1000;
        exp_average_physics_time == 0.0f
            ? exp_average_physics_time = delta_physics_time
            : exp_average_physics_time = alpha_physics_time*delta_physics_time + (1-alpha_physics_time)*exp_average_physics_time;
        std::cout << "physics time: " << exp_average_physics_time << "ms" << std::endl;

        glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(), GL_DYNAMIC_DRAW);

        glDrawArrays(GL_POINTS, 0, P);
        glfwSwapBuffers(window);
        glfwPollEvents();

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

    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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

        // Tell shader that the window has a new size
        GLuint windowWidthLocation = glGetUniformLocation(shaderProgram, "window_width");
        glUniform1f(windowWidthLocation, width);
        GLuint windowHeightLocation = glGetUniformLocation(shaderProgram, "window_height");
        glUniform1f(windowHeightLocation, height);
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

void shader_setup() {
    GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    GLchar infoLog[INFOLOG_LEN];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
    /* Fragment shader */
    GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
    /* Link shaders */
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);
}