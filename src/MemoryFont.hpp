//
//  MemoryFont.hpp
//
#pragma once

#ifndef MemoryFont_hpp
#define MemoryFont_hpp

#include "ofMain.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Hack-Regular.h"
#include "Hack-Bold.h"

class MemoryFont : public ofTrueTypeFont {
public:
    MemoryFont();
    void draw(string text, float x, float y);
    void setSize(int size);
    void setBold();
    void setRegular();
    
    FT_Library ft;
    FT_Face face;
    ofTexture glyphTextures[128];  // ASCII characters
    int glyphWidths[128];
    int glyphHeights[128];
private:
    void buildGlyphs();
};

#endif /* MemoryFont_hpp */
