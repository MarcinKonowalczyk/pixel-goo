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

// Particles
const int P = 10;
std::array<glm::vec3, P> positions;

void window_setup();
void shader_setup();
void updateShaderWidthHeightUniforms(int width, int height);

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

    std::array<glm::vec2, P> vertices;
    // int i = 0;
    for (glm::vec2& vertex : vertices) {
        // vertex = glm::vec2(0+i,0+i);
        vertex = glm::vec2(0,0);
        // i += 10;
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

    glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Write window size to uniforms
    updateShaderWidthHeightUniforms(width, height);

    GLuint textures[2];
    glGenTextures(2, textures);

    // Texture 1
    int positionMapTextureIndex = 0;
    glActiveTexture(GL_TEXTURE0 + positionMapTextureIndex);
    glBindTexture(GL_TEXTURE_1D, textures[0]);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float margin = glm::min(width,height)*0.05;
    // for (glm::vec3& position : positions) {
    //     // 
    //     position = glm::vec3( glm::gaussRand<float>(0.5, 0.5), glm::gaussRand<float>(0.5, 0.5) , 0.0 );
    //     position = glm::clamp(position, 0.0f,1.0f);
    //     // position = glm::vec2( glm::linearRand<float>(0, 1), glm::linearRand<float>(0, 1) );
    //     position *= ( glm::vec3( width, height, 1.0 ) - 2*margin );
    //     position += margin;
    // }
    int i = 0;
    for (glm::vec3& position : positions) {
        // 
        position = glm::vec3(0+i, 0+i, 0.0);
        i += 40;
    }

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, P, 0, GL_RGB, GL_FLOAT, positions.data());


    // Texture 2
    int densityMapTextureIndex = 1;
    glActiveTexture(GL_TEXTURE0 + densityMapTextureIndex);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(fbo_status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }

    GLuint location;
    glUseProgram(screenRenderingShader);
    location = glGetUniformLocation(screenRenderingShader, "density_map");
    glUniform1i(location, densityMapTextureIndex);
    location = glGetUniformLocation(screenRenderingShader, "position_map");
    glUniform1i(location, positionMapTextureIndex);

    // Physics timing preamble
    float exp_average_physics_time = 0.0f;
    float alpha_physics_time = 0.05;

    while (!glfwWindowShouldClose(window)) {
        
        // Physics timing begin
        float physics_time_start = glfwGetTime();

        // auto first = positions.begin();
        // auto last = positions.end();
        // for(; first != last; ++first) {
        //     // for(auto next = std::next(first); next != last; ++next) {
        //     //     glm::length(*next-*first);
        //     // }
        //     *first += glm::diskRand(3.0);
        // }

        // for (glm::vec3& position : positions) {
        //     position += glm::diskRand(3.0);
        // }
        
        // glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(), GL_DYNAMIC_DRAW);

        // Physics timing end
        float delta_physics_time = (glfwGetTime()-physics_time_start)*1000;
        exp_average_physics_time == 0.0f
            ? exp_average_physics_time = delta_physics_time
            : exp_average_physics_time = alpha_physics_time*delta_physics_time + (1-alpha_physics_time)*exp_average_physics_time;
        
        
        std::cout << "physics time: " << exp_average_physics_time << "ms" << std::endl;

        // Density map pass
        // glUseProgram(densityMapShader);
        // glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // glClear(GL_COLOR_BUFFER_BIT);

        // glDrawArrays(GL_POINTS, 0, P);

        // Screen rendering pass
        // glUseProgram(densityMapShader);
        glUseProgram(screenRenderingShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_POINTS, 0, P);

        // Swap draw and screen buffer
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
}

void updateShaderWidthHeightUniforms(int new_width, int new_height) {
    glUseProgram(screenRenderingShader);
    glUniform1f(glGetUniformLocation(screenRenderingShader, "window_width"), new_width);
    glUniform1f(glGetUniformLocation(screenRenderingShader, "window_height"), new_height);
    glUseProgram(densityMapShader);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_width"), new_width);
    glUniform1f(glGetUniformLocation(densityMapShader, "window_height"), new_height);
}