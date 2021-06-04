#include <iostream>

#include <glad/glad.h> // OpenGL functions
#include <GLFW/glfw3.h>

// float vertices[] = {
//     -1.0f, -1.0f,
//     1.0f, -1.0f,
//     1.0f, 1.0f,
//     -1.0f, -1.0f,
//     1.0f, 1.0f,
//     -1.0f, 1.0f,
// };

// unsigned VAO;
// glGenVertexArrays(1, &VAO);
// glBindVertexArray(VAO);

// unsigned int VBO;
// glGenBuffers(1, &VBO);
// glBindBuffer(GL_ARRAY_BUFFER, VBO);
// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
// glVertexAttribPointer (
//     0,                    // Location = 0
//     2,                    // Size of vertex attribute
//     GL_FLOAT,            // Type of the data
//     GL_FALSE,            // Normalize data?
//     2 * sizeof(float),    // Stride
//     (void*)0            // Offset
// );
// glEnableVertexAttribArray(0/*Location*/);
// glBindVertexArray(0);

// GLuint vertexShader;
// vertexShader = glCreateShader(GL_VERTEX_SHADER);
// glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
// glCompileShader(vertexShader);

// int success;
// char infoLog[512];
// glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
// if (!success) {
//     glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
//     fprintf(stderr, infoLog);
//     return false;
// }

// GLuint fragmentShader;
// fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
// glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
// glCompileShader(fragmentShader);


// glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
// if (!success) {
//     glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
//     fprintf(stderr, infoLog);
//     return false;
// }

// m_shaderProgram = glCreateProgram();
// glAttachShader(m_shaderProgram, vertexShader);
// glAttachShader(m_shaderProgram, fragmentShader);
// glLinkProgram(m_shaderProgram);
// glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
// if (!success) {
//     glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
//     fprintf(stderr, infoLog);
//     return false;
// }
// glDeleteShader(vertexShader);
// glDeleteShader(fragmentShader);

// glUseProgram(m_shaderProgram);
// glBindVertexArray(VAO);

// glEnable(GL_BLEND);
// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// return true;

GLFWwindow* window;
int width = 800;
int height = 600;
const char* title = "Goolets";
bool fullscreen = false;

struct particle {
    float x;
    float y;
};
const int P = 1000; // Number of particles
particle x[P];

void window_setup();

void particles_setup() {

}

int main() {
    window_setup();

    while (!glfwWindowShouldClose(window)) {

        float start = glfwGetTime();

        // int key = glfwGetKey(window, GLFW_KEY_ESCAPE);
        // if (key) {
            // std::cout << key << std::endl;
        // }
        // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            // glfwSetWindowShouldClose(window, true);
        // }

        glClear(GL_COLOR_BUFFER_BIT);

        // float white[] = {1.0f,1.0f,1.0f};
        // glDrawPixels(10,10,GL_RGB,GL_FLOAT,white);
        glBegin(GL_TRIANGLES);
            glColor3f(0.067f, 0.455f, 0.729f); glVertex2f( 0.0f, 0.5f);
            glColor3f(0.843f, 0.329f, 0.149f); glVertex2f(-0.5f,-0.5f);
            glColor3f(0.925f, 0.690f, 0.208f); glVertex2f( 0.5f,-0.5f);
        glEnd();
        // glBegin(GL_TRIANGLES);
        // glVertex2f( -0.5f, -0.5f);
        // glVertex2f( 0.0f,   0.5f);
        // glVertex2f( 0.5f,  -0.5f);
        // glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();

        float end = glfwGetTime();
        std::cout << (end-start)*1000 << std::endl;
    }

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void window_setup() {
    // Setup window
    glfwSetErrorCallback([](int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    });

    if (!glfwInit()) { exit(EXIT_FAILURE); }

    // GL 2.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWmonitor* primaryMonitor;
    if (fullscreen) {
        primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        width = mode->width;
        height = mode->height;
    } else {
        primaryMonitor = nullptr;
    }

    std::cout << width << " " << height << std::endl;
    
    window = glfwCreateWindow(width, height, title, primaryMonitor, nullptr);

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        exit(EXIT_FAILURE);
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    std::cout << width << "->" << framebufferWidth << std::endl;
    std::cout << height << "->" << framebufferHeight << std::endl;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
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
    }
}