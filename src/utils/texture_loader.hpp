#pragma once

#include "opengl.hpp"

class texture_loader {
public:
	texture_loader() { }
	void loadTextures(GLuint shader);
	void bindTexturesToShader(GLuint shader);
	void bindTextureUniform(GLuint shader, const char* uniformName, GLenum textureUnit, GLuint textureHandle);
private:
	GLuint m_texture1, m_texture2, m_texture3, m_normTex1, m_normTex2, m_normTex3, m_heightTex1, m_heightTex2, m_heightTex3, m_leavesTexture, m_leavesHeight, m_barkTexture;
};
