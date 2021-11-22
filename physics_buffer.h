#ifndef PHYSICS_BUFFER_H
#define PHYSICS_BUFFER_H

#include <glad.h>

typedef int PBindex;

#define PB_CURRENT 0
#define PB_OTHER 1

class PhysicsBuffer {
public:
    const char* name;
    PBindex current;
    PBindex other;
    PhysicsBuffer(const char* n, const int w, const int h,const GLuint* t, const GLuint* f) :
        name(n), width(w), height(h), textures(t), framebuffers(f), current(0), other(1) {};
    void allocate(const int which, const PBindex index, const char* data);
    void flip_buffers();

private:
    const int width;
    const int height;
    const GLuint* textures;
    const GLuint* framebuffers;
    void set_index(const int which, const PBindex index);
};

#endif /* PHYSICS_BUFFER_H */