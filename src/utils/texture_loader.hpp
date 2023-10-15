#pragma once

#include "opengl.hpp"

/*

Loads in the following textures:
https://3dtextures.me/2018/01/05/grass-001-2/
https://3dtextures.me/2020/01/31/mud-001/
https://3dtextures.me/2022/03/03/rock-044/
https://3dtextures.me/2020/09/08/stylized-leaves-002/
https://3dtextures.me/2021/05/05/bark-006-2/

Code Author: Shekinah Pratap

*/

class texture_loader {
public:
	texture_loader() { }
	void loadTextures(GLuint shader);
	void bindTexturesToShader(GLuint shader);
	void bindTextureUniform(GLuint shader, const char* uniformName, GLenum textureUnit, GLuint textureHandle);
private:
	GLuint m_texture1, m_texture2, m_texture3, m_normTex1, m_normTex2, m_normTex3, m_heightTex1, m_heightTex2, m_heightTex3, m_leavesTexture, m_leavesHeight, m_barkTexture;
};
