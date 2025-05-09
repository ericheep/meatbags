//
//  MemoryFont.cpp
//

#include "MemoryFont.hpp"

MemoryFont::MemoryFont() {
    // Initialize FreeType
    if (FT_Init_FreeType(&ft)) {
        ofLogError() << "Could not init FreeType";
        return;
    }

    setRegular();

    // Set font size
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Load glyphs for ASCII characters
    buildGlyphs();
}

void MemoryFont::setBold() {
    // Load font from memory
    if (FT_New_Memory_Face(ft, Hack_Bold_ttf, Hack_Bold_ttf_len, 0, &face)) {
        ofLogError() << "Could not load font from memory";
        return;
    }
}

void MemoryFont::setRegular() {
    // Load font from memory
    if (FT_New_Memory_Face(ft, Hack_Regular_ttf, Hack_Regular_ttf_len, 0, &face)) {
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
        ofPixels pixels;
        pixels.setFromPixels(bmp.buffer, bmp.width, bmp.rows, OF_PIXELS_GRAY);

        glyphTextures[c].allocate(pixels);
        glyphTextures[c].loadData(pixels);

        glyphWidths[c] = face->glyph->advance.x >> 6;
        glyphHeights[c] = bmp.rows;
    }
}

void MemoryFont::setSize(int size) {
    FT_Set_Pixel_Sizes(face, 0, size);
    buildGlyphs();
}

void MemoryFont::draw(string text, float x, float y) {
    for (char c : text) {
        float localY = y;
        if (c == 'g' || c == 'y' || c == 'p' || c == 'q') localY += 3;
        
        if (glyphTextures[c].isAllocated()) {
            glyphTextures[c].draw(x, localY - glyphHeights[c]);
        }
        
        x += glyphWidths[c] ;
    }
}
