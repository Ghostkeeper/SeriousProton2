#include <sp2/graphics/shader.h>
#include <sp2/graphics/opengl.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/graphics/texture.h>


namespace sp {

std::map<string, Shader*> Shader::cached_shaders;
Shader* Shader::bound_shader;

Shader* Shader::get(string name)
{
    auto it = cached_shaders.find(name);
    if (it != cached_shaders.end())
    {
        return it->second;
    }
    
    Shader* new_shader;
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    if (stream)
    {
        LOG(Info, "Loading shader:", name);
        string vertex_shader;
        string fragment_shader;
        int type = -1;
        while(stream->tell() != stream->getSize())
        {
            string line = stream->readLine();
            if (line == "[VERTEX]")
                type = 1;
            else if (line == "[FRAGMENT]")
                type = 2;
            else if (type == 1)
                vertex_shader += line + "\n";
            else if (type == 2)
                fragment_shader += line + "\n";
        }
        new_shader = new Shader(name, std::move(vertex_shader), std::move(fragment_shader));
    }
    else
    {
        new_shader = new Shader(name);
        LOG(Warning, "Failed to find shader:", name);
    }
    cached_shaders[name] = new_shader;
    return new_shader;
}

Shader::Shader(string name)
: name(name)
{
    program = 0;
}

Shader::Shader(string name, string&& vertex_shader, string&& fragment_shader)
: name(name), vertex_shader(std::move(vertex_shader)), fragment_shader(std::move(fragment_shader))
{
    program = 0xffffffff;
}

bool Shader::bind()
{
    if (bound_shader == this)
        return false;

    bound_shader = this;

    if (program == 0xffffffff)
    {
        program = glCreateProgram();

        unsigned int vertex_shader_handle = compileShader(&vertex_shader[0], GL_VERTEX_SHADER);
        if (!vertex_shader_handle)
            return;
        unsigned int fragment_shader_handle = compileShader(&fragment_shader[0], GL_FRAGMENT_SHADER);
        if (!fragment_shader_handle)
        {
            glDeleteShader(vertex_shader_handle);
            return;
        }
        glLinkProgram(program);
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char log[1024];
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            LOG(Error, "Error in shader:", name, log);
            glDeleteProgram(program);
            glDeleteShader(vertex_shader_handle);
            glDeleteShader(fragment_shader_handle);
            program = 0;
            return;
        }
        glDetachShader(program, vertex_shader_handle);
        glDetachShader(program, fragment_shader_handle);
        
        LOG(Info, "Compiled shader:", name);
    }

    glUseProgram(program);
    return true;
}

unsigned int Shader::compileShader(const char* code, int type)
{
    int success;
    unsigned int shader_handle = glCreateShader(type);
    glShaderSource(shader_handle, 1, &code, nullptr);
    glCompileShader(shader_handle);
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader_handle, sizeof(log), nullptr, log);
        LOG(Error, "Error in shader:", name, log);
        glDeleteShader(shader_handle);
        glDeleteProgram(program);
        program = 0;
        return 0;
    }
    glAttachShader(program, shader_handle);
    return shader_handle;
}

void Shader::setUniform(const string& s, const Matrix4x4f& matrix)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");
    
    int location = getUniformLocation(s);
    if (location == -1)
        return;
    glUniformMatrix4fv(location, 1, false, matrix.data);
}

void Shader::setUniform(const string& s, const Matrix4x4d& matrix)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");

    int location = getUniformLocation(s);
    if (location == -1)
        return;
    if (glUniformMatrix4dv)
    {
        glUniformMatrix4dv(location, 1, false, matrix.data);
    }
    else
    {
        float data[16];
        for(int n=0; n<16; n++)
            data[n] = matrix.data[n];
        glUniformMatrix4fv(location, 1, false, data);
    }
}

void Shader::setUniform(const string& s, const Vector2f& v)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");

    int location = getUniformLocation(s);
    if (location == -1)
        return;
    glUniform2fv(location, 1, &v.x);
}

void Shader::setUniform(const string& s, const Vector3f& v)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");

    int location = getUniformLocation(s);
    if (location == -1)
        return;
    glUniform3fv(location, 1, &v.x);
}

void Shader::setUniform(const string& s, const Color& c)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");

    int location = getUniformLocation(s);
    if (location == -1)
        return;
    glUniform4fv(location, 1, &c.r);
}

void Shader::setUniform(const string& s, Texture* texture)
{
    sp2assert(bound_shader == this, "Shader needs to be bound before uniforms can be set");
    
    if (!texture)
        return;
    
    int location = getUniformLocation(s);
    if (location == -1)
        return;
    //TODO: This assumes we bind only 1 texture.
    glUniform1i(location, 0);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
}

int Shader::getUniformLocation(const string& s)
{
    auto it = uniform_mapping.find(s);
    if (it != uniform_mapping.end())
        return it->second;
        
    int location = glGetUniformLocation(program, s.c_str());
    if (location == -1)
        LOG(Debug, "Failed to find uniform:", s, "in:", name);
    uniform_mapping[s] = location;
    return location;
}

void Shader::unbind()
{
    glUseProgram(0);
    bound_shader = nullptr;
}

};//namespace sp
