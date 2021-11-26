#ifndef buffer_H
#define buffer_H

#include <glad.h>

typedef int PBindex;

enum Bwhich {
    current = 0,
    other = 1,
    screen = 2
};

typedef struct format {
    GLint format;
    GLint iformat;
} format;

#define BE_1D format { .format = GL_RED, .iformat = GL_R32F}
#define BE_2D format { .format = GL_RG, .iformat = GL_RG32F}
#define BE_3D format { .format = GL_RGB, .iformat = GL_RGB32F}
#define BE_4D format { .format = GL_RGBA, .iformat = GL_RGBA32F}

// #define PB_1D GL_R32F
// #define PB_2D GL_RG32F
// #define PB_3D GL_RGB32F
// #define PB_4D GL_RGBA32F

class Buffer {
public:
    const char* name;
    PBindex current;
    PBindex other;
    int width;
    int height;
    Buffer(const char* n, const GLuint* t, const GLuint* f) :
        name(n), width(0), height(0), textures(t), framebuffers(f), current(0), other(1), minmag_filter(GL_NONE), wrap_st(GL_NONE), dim(BE_1D) {};
    void allocate(const Bwhich which, const PBindex index, const int width, const int height, const char* data);
    void reallocate(const Bwhich which, const int width, const int height);
    void flip_buffers();
    void bind(const Bwhich which);
    // void bind();
    static void update();
    static void update(const int P);

    GLuint minmag_filter;
    GLuint wrap_st;
    format dim;

private:
    const GLuint* textures;
    const GLuint* framebuffers;
    void set_index(const Bwhich which, const PBindex index);
    PBindex* which2index(const Bwhich which);
    void check_shape(const int width, const int height);
};

#endif /* buffer_H */