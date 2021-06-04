#include "mingl.hpp"

#include "glad/glad.h" // OpenGL functions
#include "GLFW/glfw3.h"

#include <iostream>

const char* vertexShaderSource =
R"(#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
})";

// Fragment shader always colours in white
const char* fragmentShaderSource =
R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1,1,1,1); })";

bool MinGL::init(unsigned width, unsigned height, const char* title) {

    // GLFW
    // Setup window
    glfwSetErrorCallback([](int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    });

    if (!glfwInit()) { return false; }

    // GL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        return false;
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    std::cout << width << "->" << framebufferWidth << std::endl;
    std::cout << height << "->" << framebufferHeight << std::endl;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    float vertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f,
    };

    unsigned VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer (
        0,                    // Location = 0
        2,                    // Size of vertex attribute
        GL_FLOAT,            // Type of the data
        GL_FALSE,            // Normalize data?
        2 * sizeof(float),    // Stride
        (void*)0            // Offset
    );
    glEnableVertexAttribArray(0/*Location*/);
    glBindVertexArray(0);

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        fprintf(stderr, infoLog);
        return false;
    }

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);


    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        fprintf(stderr, infoLog);
        return false;
    }

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        fprintf(stderr, infoLog);
        return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(m_shaderProgram);
    glBindVertexArray(VAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

bool MinGL::windowShouldClose() const {
    return (bool)glfwWindowShouldClose(window);
}

void MinGL::pollEvents() const {
    glfwPollEvents();
}

void MinGL::processInput() const {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void MinGL::putPixel(int x, int y) const {
    // glEnable(GL_SCISSOR_TEST);
    // glScissor(x, y, 1, 1); /// position of pixel
    // glDrawArrays(GL_TRIANGLES, 0/*Starting Index*/, 6/*# of vertices*/);
    glDrawArrays(GL_POINTS, 0/*Starting Index*/, 6/*# of vertices*/);
    // glDisable(GL_SCISSOR_TEST);
    // glBegin(GL_POINTS);
    // glColor3f(1,1,1);
    // glVertex2i(x,y);
    // glEnd();
}

void MinGL::flush(float r, float g, float b, float a) {
    glfwSwapBuffers(window);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void MinGL::shutdown() const {
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow* MinGL::getWindow() {
    return window;
}
