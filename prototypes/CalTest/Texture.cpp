#include <cassert>

#include "SDL_opengl.h"

#include "Texture.h"

uint Texture::nInstances=0;
std::map<std::string,uint> Texture::textures;

uint Texture::Load(const std::string& fname)
{
    uint hTex;
    hTex=textures[fname];
    if (hTex!=0)
        return hTex;

    FILE* f=fopen(fname.c_str(),"rb");
    if (!f)
    {
        printf("Unable to load %s\r\n",fname.c_str());
        return 0;
    }

    int w,h,d;
    fread(&w,4,1,f);    fread(&h,4,1,f);    fread(&d,1,4,f);
    char* data=new char[w*h*d];
    char* p=data+((h-1)*w*d);
    // RAW is stored bottom-to-top.  Flip it as we read.
    for (int y=0; y<h; y++)
    {
        assert(p>=data);
        fread(p,w*d,1,f);
        p-=w*d;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1,&hTex);
    glBindTexture(GL_TEXTURE_2D,hTex);
    glTexImage2D(GL_TEXTURE_2D,0,d==3?GL_RGB:GL_RGBA,w,h,0,d==3?GL_RGB:GL_RGBA,GL_UNSIGNED_BYTE,data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    delete[] data;
    fclose(f);

    textures[fname]=hTex;
    return hTex;
}