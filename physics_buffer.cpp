#include "physics_buffer.h"
#include <iostream>
#include <glad.h>

// void PhysicsBuffer::allocate_both(const int index1, const int index2, const char* data1, const char* data2) {
//     allocate(index1, data1);
//     allocate(index2, data2);
// }

void PhysicsBuffer::allocate(const int which, const PBindex index, const char* data) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textures[index]);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, data);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[index], 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        exit(EXIT_FAILURE);
    }

    this->set_index(which, index);
}

void PhysicsBuffer::set_index(const int which, const PBindex index) {
        switch (which) {
        case PB_CURRENT: {
            this->current = index;
        } break;
        case PB_OTHER: {
            this->other = index;
        } break;
        default:
            fprintf(stderr, "[%s] physics buffer can only have two underlying textures", this->name);
            exit(EXIT_FAILURE);
    }
}

void PhysicsBuffer::flip_buffers() {
    PBindex temp = this->current;
    this->current = this->other;
    this->other = temp;
}