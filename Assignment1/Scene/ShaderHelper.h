#pragma once
#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper
{
public:
    // core compile.link, shared by both loading methods below
    static GLuint compileShader(GLenum type, const char *src)
    {
        GLuint shader = glCreateShader(type);
        if (!shader)
        {
            LOGE("glCreateShader failed (type=0x%x)", type);
            return 0;
        }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            
            LOGE("Shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint linkProgram(GLuint vert, GLuint frag);

    // Method 1: stringify - build directly from inline source strings
    // this is what triangle.cpp already does, keep it available here
    // for any future model that prefers to embed its shader source in code
    static GLuint buildProgram(const char *vertSrc, const char *fragSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
        if (!vs)
            return 0;
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!fs)
        {
            glDeleteShader(vs);
            return 0;
        }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("Program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            prog = 0;
        }
        glDetachShader(prog, vs);
        glDetachShader(prog, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return prog;
    }

    // Method 2: file-based - load shader source from disk / APK assets
#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager *mgr, const char *path)
    {
        if (!mgr || !path) // if there is no pointer for assetmanager or there is no path given,, throw error
        {
            LOGE("loadAsset failed: Native asset manager or path is null pointer.");
            return "";
        }

        // open up the file
        AAsset *asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) // if cannot open file
        {
            LOGE("Failed to open Android asset file: %s", path);
            return "";
        }

        off_t length{AAsset_getLength(asset)};
        
        std::string fileContents(static_cast<size_t>(length), '\0'); //making the size

        AAsset_read(asset, &fileContents[0], length); // parsing file inside length
        AAsset_close(asset);

        return fileContents;
    }

    static GLuint buildProgramFromAssets(AAssetManager *mgr, const char *vertPath, const char *fragPath){

        // loading the vertex and fragment shader files cstrings
        std::string vertShdr{loadAsset(mgr, vertPath)}; 
        std::string fragShdr{loadAsset(mgr, fragPath)};

        return buildProgram(vertShdr.c_str(), fragShdr.c_str());
    }


#else
    static std::string loadFile(const char *filename)
    {
        // try opening files in this order:
        // assets/shader/<filename>
        //<filename>
        // shader/<filename>
        // assets/<filename>

        std::string sstr{"assets/shader/"};
        sstr += filename; // this is for assets/shader/filename

        std::string n_filename{sstr};
        std::ifstream ifs{n_filename};
        if (!ifs)
        {
            ifs.close();
            ifs.clear();
            // then we try
            n_filename = filename;
            LOGE("Filename: %s cannot be found!", n_filename.c_str());
            ifs.open(n_filename);
        }

        if (!ifs)
        {
            ifs.close();
            ifs.clear();
            // if still isnt valid
            sstr = std::string{"shader/"};
            sstr += filename;
             
            n_filename = std::string{sstr};
             LOGE("Filename: %s cannot be found!", n_filename.c_str());
            ifs.open(n_filename);
        }

        if (!ifs)
        {
            ifs.close();
            ifs.clear();
            // if still isnt valid
            sstr = std::string{"assets/"};
            sstr += filename;
             
            n_filename = std::string{sstr};
             LOGE("Filename: %s cannot be found!", n_filename.c_str());
            ifs.open(n_filename);
        }

        if (!ifs)
        {
            // somehow still not valid
            LOGE("Filename: %s cannot be found!", n_filename.c_str());
        }


        std::string contents{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

        //std::stringstream buffer;
        //buffer << ifs.rdbuf();
        //return buffer.str();
        //LOGE("%s!", contents.c_str());
        return contents;
    }

    static GLuint buildProgramFromFile(const char *vertFile, const char *fragFile)
    {
        std::string fragShdr = loadFile(fragFile);
        std::string vertShdr = loadFile(vertFile);

        return buildProgram(vertShdr.c_str(),fragShdr.c_str());
    }
#endif
};