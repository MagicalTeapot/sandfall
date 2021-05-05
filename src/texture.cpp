#include "texture.h"

#include <glad/glad.h>

namespace alc {

texture::texture()
{
    glGenTextures(1, &d_texture); 
    bind();

    glTextureParameteri(d_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(d_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(d_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(d_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SIZE, SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
}

void texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, d_texture);
}

void texture::set_buffer(const std::array<glm::vec4, SIZE * SIZE>& data)
{
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SIZE, SIZE, 0, GL_RGBA, GL_FLOAT, data.data());
}

}