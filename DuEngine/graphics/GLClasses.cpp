#include "GLClasses.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "LoadTexture.hpp"
VertexBuffer::VertexBuffer(const void* data, unsigned int size) {
	gltry(glGenBuffers(1, &RenderID));
	gltry(glBindBuffer(GL_ARRAY_BUFFER, RenderID));
	gltry(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}
VertexBuffer::VertexBuffer(unsigned int size) { //Dynamic VertexBuffer
	gltry(glGenBuffers(1, &RenderID));
	gltry(glBindBuffer(GL_ARRAY_BUFFER, RenderID));
	gltry(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
}
VertexBuffer::~VertexBuffer() {
	gltry(glDeleteBuffers(1, &RenderID));
}
unsigned int VertexBuffer::BindedRenderID = 0;
void VertexBuffer::SendData(unsigned int offset, unsigned int size, const void* data) {
	Bind();
	gltry(glBufferSubData(GL_ARRAY_BUFFER, offset, (GLsizeiptr)size, data));
}
void VertexBuffer::Bind() const {
	if (RenderID != BindedRenderID) {
		gltry(glBindBuffer(GL_ARRAY_BUFFER, RenderID));
		BindedRenderID = RenderID;
	}
}
void VertexBuffer::Unbind() const {
	gltry(glBindBuffer(GL_ARRAY_BUFFER, 0));
	BindedRenderID = 0;
}

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_count(count) {
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	gltry(glGenBuffers(1, &RenderID));
	gltry(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderID));
	gltry(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long int)(count * sizeof(unsigned int)), data, GL_STATIC_DRAW));
}
IndexBuffer::IndexBuffer(unsigned int count) : m_count(count) {
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));

	gltry(glGenBuffers(1, &RenderID));
	gltry(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderID));
	gltry(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long int)(count * sizeof(unsigned int)), nullptr, GL_DYNAMIC_DRAW));
}
void IndexBuffer::SendData(unsigned int offset, unsigned int count, const unsigned int* data) {
	Bind();
	gltry(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, (GLsizeiptr)sizeof(unsigned int) * count, data));
}
IndexBuffer::~IndexBuffer() {
	gltry(glDeleteBuffers(1, &RenderID));
}
unsigned int IndexBuffer::BindedRenderID = 0;
void IndexBuffer::Bind() const {
	if (RenderID != BindedRenderID) {
		gltry(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderID));
		BindedRenderID = RenderID;
	}
}
void IndexBuffer::Unbind() const {
	gltry(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	BindedRenderID = 0;
}

unsigned int VertexBufferElement::GetSizeOfType(unsigned int type) {
	switch (type) {
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_UNSIGNED_INT:
			return sizeof(GLuint);
		case GL_INT:
			return sizeof(GLint);
		case GL_UNSIGNED_BYTE:
			return sizeof(GLbyte);
	}
	ASSERT(0);
	return 0;
}
void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
	Bind();
	vb.Bind();
	const auto& elements = layout.GetElements();
	for (unsigned int i = 0; i < elements.size(); ++i) {
		const auto& element = elements[i];
		gltry(glEnableVertexAttribArray(i));
		gltry(glVertexAttribPointer(i, (int)element.count, element.type, element.normalized, (int)layout.GetStride(), (const void*)element.offset));
	}
}
VertexArray::VertexArray() {
	gltry(glGenVertexArrays(1, &RenderID));	 // documentation
}
VertexArray::~VertexArray() {
	gltry(glDeleteVertexArrays(1, &RenderID));
}
unsigned int VertexArray::BindedRenderID = 0;
void VertexArray::Bind() const {
	if (RenderID != BindedRenderID) {
		gltry(glBindVertexArray(RenderID));	 // documentation
		BindedRenderID = RenderID;
	}
}
void VertexArray::Unbind() const {
	gltry(glBindVertexArray(0));
	BindedRenderID = 0;
}

struct Shader::ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};
Shader::Shader(const std::string& Filename) : File(Filename) {
	ShaderProgramSource source = ParseShader(Filename);
	RenderID = CreateShader(source.VertexSource, source.FragmentSource);
}
Shader::~Shader() {
	gltry(glDeleteProgram(RenderID));
}
unsigned int Shader::BindedRenderID = 0;
void Shader::Bind() const {
	if (RenderID != BindedRenderID) {
		glUseProgram(RenderID);
		BindedRenderID = RenderID;
	}
}
void Shader::Unbind() const {
	glUseProgram(0);
	BindedRenderID = 0;
}
Shader::ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
	std::ifstream stream(filepath);
	if (!stream.good()) throw std::runtime_error("Shader file not good");
	enum class ShaderType {
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};
	ShaderType type = ShaderType::NONE;
	std::string line;
	std::stringstream ss[2];
	while (getline(stream, line)) {
		if (line.find("vertex") != std::string::npos) {
			type = ShaderType::VERTEX;
		} else if (line.find("fragment") != std::string::npos) {
			type = ShaderType::FRAGMENT;
		} else {
			ss[(int)type] << line << '\n';
		}
	}
	return {ss[0].str(), ss[1].str()};
}
unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca((unsigned)length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Cant compile shader " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}
unsigned int Shader::CreateShader(const std::string& vertexshader, const std::string& fragmentshader) {
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexshader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentshader);

	glAttachShader(program, vs);  // documentation
	glAttachShader(program, fs);  // documentation
	glLinkProgram(program);		  // documentation
	glValidateProgram(program);	  // documentation

	glDeleteShader(vs);	 // documentation
	glDeleteShader(fs);	 // documentation
	return program;
}

void Shader::SetUniform1i(const std::string& name, int value) {
	gltry(glUniform1i(GetUniformLocation(name), value));
}
void Shader::SetUniform1iv(const std::string& name, unsigned int count, int* data) {
	gltry(glUniform1iv(GetUniformLocation(name), (int)count, data));
}
void Shader::SetUniform2f(const std::string& name, const vec2f& v) {
	gltry(glUniform2f(GetUniformLocation(name), v.v0, v.v1));
}
void Shader::SetUniform3f(const std::string& name, const vec3f& v) {
	gltry(glUniform3f(GetUniformLocation(name), v.v0, v.v1, v.v2));
}
void Shader::SetUniform4f(const std::string& name, const vec4f& v) {
	gltry(glUniform4f(GetUniformLocation(name), v.v0, v.v1, v.v2, v.v3));
}
void Shader::SetUniformMat4f(const std::string& name, const mat4f& mat) {
	gltry(glUniformMatrix4fv(GetUniformLocation(name), 1, 0, (const float*)&mat.matrix[0][0]));
}
int Shader::GetUniformLocation(const std::string& name) {
	if (UniformLocationCache.find(name) != UniformLocationCache.end())
		return UniformLocationCache[name];

	int location = glGetUniformLocation(RenderID, name.c_str());
	if (location < 0)
		std::cout << "ERRO: Uniform " << name << " not found" << std::endl;
	UniformLocationCache[name] = location;
	return location;
}

TextureParameters::TextureParameters(GLint mag_filter, GLint min_filter, GLint warp_s, GLint warp_t) : mag_filter(mag_filter), min_filter(min_filter), warp_s(warp_s), warp_t(warp_t) {
	//std::cout << "TextureParameters constructor" << std::endl;
}
Texture::Texture(const std::string& path, const TextureParameters& params) : File(path), Width(0), Height(0) {
	unsigned char* LocalBuffer = LoadTexture(path, &Width, &Height);
	if (Width >= MaxTextureSize || Height >= MaxTextureSize) throw std::length_error("Image bigger than GL_MAX_TEXTURE_SIZE :" + std::to_string(MaxTextureSize));
	gltry(glGenTextures(1, &RenderID));
	gltry(glBindTexture(GL_TEXTURE_2D, RenderID));

	gltry(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.min_filter));
	gltry(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.mag_filter));
	gltry(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.warp_s));
	gltry(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.warp_t));

	gltry(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, LocalBuffer));
	gltry(glBindTexture(GL_TEXTURE_2D, 0));
	free(LocalBuffer);
}
Texture::~Texture() {
	gltry(glDeleteTextures(1, &RenderID));
}

void Texture::Bind(unsigned int slot) const {
	gltry(glActiveTexture(GL_TEXTURE0 + slot));
	gltry(glBindTexture(GL_TEXTURE_2D, RenderID));
}
void Texture::Unbind() const {
	gltry(glBindTexture(GL_TEXTURE_2D, 0));
}
unsigned int Texture::GetID() const {
	return RenderID;
}
int Texture::MaxTextureSize = 16384;//At least 16384
void Texture::GetMaxTextureSize() {
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTextureSize);
}

Texture& SubTexture::getTexture() const {
	return *MainTexture;
}
SubTexture::SubTexture(const std::shared_ptr<Texture>& Texture) : MainTexture(Texture) {
	Hpos = Vpos = 0.0f;
	Hsize = Vsize = 1.0f;
}
SubTexture::SubTexture(const std::shared_ptr<Texture>& Texture, int Hindex, int Vindex, int Hnum, int Vnum) : MainTexture(Texture) {
	Vindex = Vnum - Vindex - 1;	 //image is upside down
	Hsize = 1.0f / Hnum;
	Vsize = 1.0f / Vnum;
	Hpos = Hindex * Hsize;
	Vpos = Vindex * Vsize;
}
SubTexture::~SubTexture() {}
