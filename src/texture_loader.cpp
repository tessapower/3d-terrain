#pragma once

#include "texture_loader.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_image.hpp"


void texture_loader::loadTextures(GLuint shader) {
	// Load texture data
	cgra::rgba_image texture1_data(CGRA_SRCDIR + std::string("\\res\\textures\\Grass_001_COLOR.jpg"));
	cgra::rgba_image texture2_data(CGRA_SRCDIR + std::string("\\res\\textures\\Mud_001_baseColor.jpg"));
	cgra::rgba_image texture3_data(CGRA_SRCDIR + std::string("\\res\\textures\\Rock_044_BaseColor.jpg"));
	// Load normal maps
	cgra::rgba_image norm_texture1_data(CGRA_SRCDIR + std::string("\\res\\textures\\Grass_001_NORM.jpg"));
	cgra::rgba_image norm_texture2_data(CGRA_SRCDIR + std::string("\\res\\textures\\Mud_001_normal.jpg"));
	cgra::rgba_image norm_texture3_data(CGRA_SRCDIR + std::string("\\res\\textures\\Rock_044_Normal.jpg"));
	// Load height maps
	cgra::rgba_image height_texture1_data(CGRA_SRCDIR + std::string("\\res\\textures\\Grass_001_DISP.png"));
	cgra::rgba_image height_texture2_data(CGRA_SRCDIR + std::string("\\res\\textures\\Mud_001_height.png"));
	cgra::rgba_image height_texture3_data(CGRA_SRCDIR + std::string("\\res\\textures\\Rock_044_Height.png"));
	// Load leaves textures
	cgra::rgba_image leaves_data(CGRA_SRCDIR + std::string("\\res\\textures\\Stylized_Leaves_002_basecolor.jpg"));
	cgra::rgba_image height_leaves_data(CGRA_SRCDIR + std::string("\\res\\textures\\Stylized_Leaves_002_height.png"));
	// Load bark texture
	cgra::rgba_image bark_data(CGRA_SRCDIR + std::string("\\res\\textures\\Bark_06_basecolor.jpg"));

	// Upload textures and store handles
	m_texture1 = texture1_data.uploadTexture();
	m_texture2 = texture2_data.uploadTexture();
	m_texture3 = texture3_data.uploadTexture();
	m_normTex1 = norm_texture1_data.uploadTexture();
	m_normTex2 = norm_texture2_data.uploadTexture();
	m_normTex3 = norm_texture3_data.uploadTexture();
	m_heightTex1 = height_texture1_data.uploadTexture();
	m_heightTex2 = height_texture2_data.uploadTexture();
	m_heightTex3 = height_texture3_data.uploadTexture();
	m_leavesTexture = leaves_data.uploadTexture();
	m_leavesHeight = height_leaves_data.uploadTexture();
	m_barkTexture = bark_data.uploadTexture();

	bindTexturesToShader(shader);
}


void texture_loader::bindTexturesToShader(GLuint shader) {
	// Bind texture color uniforms
	bindTextureUniform(shader, "uTexture1", GL_TEXTURE0, m_texture1);
	bindTextureUniform(shader, "uTexture2", GL_TEXTURE1, m_texture2);
	bindTextureUniform(shader, "uTexture3", GL_TEXTURE2, m_texture3);

	// Bind texture normal map uniforms
	bindTextureUniform(shader, "uNormTex1", GL_TEXTURE3, m_normTex1);
	bindTextureUniform(shader, "uNormTex2", GL_TEXTURE4, m_normTex2);
	bindTextureUniform(shader, "uNormTex3", GL_TEXTURE5, m_normTex3);

	// Bind height map uniforms
	bindTextureUniform(shader, "uHeightTexture1", GL_TEXTURE6, m_heightTex1);
	bindTextureUniform(shader, "uHeightTexture2", GL_TEXTURE7, m_heightTex2);
	bindTextureUniform(shader, "uHeightTexture3", GL_TEXTURE8, m_heightTex3);

	// Bind leaves texture uniforms
	bindTextureUniform(shader, "uLeavesTexture", GL_TEXTURE9, m_leavesTexture);
	bindTextureUniform(shader, "uLeavesHeight", GL_TEXTURE10, m_leavesHeight);

	// Bind bark texture uniform
	bindTextureUniform(shader, "uBarkTexture", GL_TEXTURE11, m_barkTexture);
}


void texture_loader::bindTextureUniform(GLuint shader, const char* uniformName, GLenum textureUnit, GLuint textureHandle) {
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glUniform1i(glGetUniformLocation(shader, uniformName), textureUnit - GL_TEXTURE0);
}