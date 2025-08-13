//
//  MemoryFont.cpp
//

#include "MemoryFont.hpp"

MemoryFont::MemoryFont() {
    // initialize FreeType
    if (FT_Init_FreeType(&ft)) {
        ofLogError() << "Could not init FreeType";
        return;
    }
    
    setRegular();
    
    // set default font size
    FT_Set_Pixel_Sizes(face, 0, 48);
    
    // load glyphs for ASCII characters
    buildGlyphs();
}

void MemoryFont::setBold() {
    if (FT_New_Memory_Face(ft, FiraMono_Bold_ttf, FiraMono_Bold_ttf_len, 0, &face)) {
        ofLogError() << "Could not load font from memory";
        return;
    }
}

void MemoryFont::setRegular() {
    if (FT_New_Memory_Face(ft, FiraMono_Regular_ttf, FiraMono_Regular_ttf_len, 0, &face)) {
        ofLogError() << "Could not load font from memory";
        return;
    }
}

void MemoryFont::setMedium() {
    if (FT_New_Memory_Face(ft, FiraMono_Medium_ttf, FiraMono_Medium_ttf_len, 0, &face)) {
        ofLogError() << "Could not load font from memory";
        return;
    }
}

void MemoryFont::buildGlyphs() {
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            ofLogError() << "Could not load glyph " << c;
            continue;
        }
        
        FT_Bitmap& bmp = face->glyph->bitmap;
        
        // Skip if bitmap has no data
        if (!bmp.buffer || bmp.width == 0 || bmp.rows == 0) {
            glyphWidths[c] = face->glyph->advance.x >> 6;
            glyphTopOffsets[c] = face->glyph->bitmap_top;
            continue;
        }
        
        ofPixels pixels;
        pixels.setFromPixels(bmp.buffer, bmp.width, bmp.rows, OF_PIXELS_GRAY);
        
        glyphTextures[c].allocate(pixels);
        glyphTextures[c].loadData(pixels);
        
        glyphWidths[c] = face->glyph->advance.x >> 6;
        glyphTopOffsets[c] = face->glyph->bitmap_top;
    }
}

void MemoryFont::setSize(int size) {
    for (int i = 0; i < 128; i++) {
        if (glyphTextures[i].isAllocated()) {
            glyphTextures[i].clear();
        }
    }
    
    FT_Set_Pixel_Sizes(face, 0, size);
    buildGlyphs();
}

float MemoryFont::getStringWidth(string text) {
    float stringWidth = 0;
    for (char ch : text) {
        unsigned char c = (unsigned char)ch;
        if (c >= 128) continue;
        
        stringWidth += glyphWidths[c];
    }
    
    return stringWidth;
}

void MemoryFont::draw(string text, float x, float y) {
    for (char ch : text) {
        unsigned char c = (unsigned char)ch;
        if (c >= 128) continue;
        
        // only draw if the texture exists (e.g., not a space)
        if (glyphTextures[c].isAllocated()) {
            glyphTextures[c].draw(x, y - glyphTopOffsets[c]);
        }
        
        x += glyphWidths[c];
    }
}
