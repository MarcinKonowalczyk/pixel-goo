#include "shader.h"

// #include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

void Shader::create() {
    program = glCreateProgram();
}

void Shader::use() {
    glUseProgram(this->program);
}

void Shader::compile(const GLuint shaderTypeEnum, const GLchar* shaderSource) {
    GLuint shader = glCreateShader(shaderTypeEnum);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[INFOLOG_LEN];
        glGetShaderInfoLog(shader, INFOLOG_LEN, nullptr, infoLog);
        fprintf(stderr, "[%s] Compilation error\n%s\n", this->name, infoLog);
        exit(EXIT_FAILURE);
    }

    // Attach shader to the program
    glAttachShader(this->program, shader);
    glDeleteShader(shader);
}

void Shader::link() {
    glLinkProgram(this->program);

    GLint success;
    glGetProgramiv(this->program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[INFOLOG_LEN];
        glGetProgramInfoLog(this->program, INFOLOG_LEN, nullptr, infoLog);
        fprintf(stderr, "[%s] Link error\n%s\n", this->name, infoLog);
        exit(EXIT_FAILURE);
    }
}

void Shader::setUniform(const GLchar* uniform_name, const int value) {
    this->use();
    glUniform1i(this->getUniformLocation(uniform_name), value);
}

void Shader::setUniform(const GLchar* uniform_name, const float value) {
    this->use();
    glUniform1f(this->getUniformLocation(uniform_name), value);
}

void Shader::setUniform(const GLchar* uniform_name, const int dim, const float value[]) {
    this->use();
    switch (dim) {
        case 2: {
            glUniform2fv(this->getUniformLocation(uniform_name), 1, value);
        } break;
        case 3: {
            glUniform3fv(this->getUniformLocation(uniform_name), 1, value);
        } break;
        case 4: {
            glUniform4fv(this->getUniformLocation(uniform_name), 1, value);
        } break;
        default:
            fprintf(stderr, "[%s] Cannot set only set vec{2|3|4} uniforms", this->name);
            exit(EXIT_FAILURE);
    }
}

GLint Shader::getUniformLocation(const GLchar *uniform_name) {
    GLint location = glGetUniformLocation(this->program, uniform_name);
    if (location < 0) {
        fprintf(stderr, "[%s] Uniform \"%s\" not found in the program (location = %d)", this->name, uniform_name, location);
        exit(EXIT_FAILURE);
    }
    return location;
}
