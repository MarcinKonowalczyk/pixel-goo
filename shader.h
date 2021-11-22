#ifndef SHADER_H
#define SHADER_H

#include <glad.h>

#define INFOLOG_LEN 512

class Shader {
public:
    const char* name;
    Shader(const char* n) : name(n) {};

    void create();
    void use();
    void compile(const GLuint shaderTypeEnum, const GLchar* shaderSource);
    void link();

    void setUniform(const GLchar* uniform_name, const int value);
    void setUniform(const GLchar* uniform_name, const float value);
    void setUniform(const GLchar* uniform_name, const int dim, const float value[]);

    GLint getUniformLocation(const GLchar *uniform_name);

private:
    GLint program;
};

#endif /* SHADER_H */