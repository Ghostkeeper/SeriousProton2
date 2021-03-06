#include <sp2/graphics/font.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/opengl.h>
#include <sp2/stringutil/convert.h>
#include <sp2/assert.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


namespace sp {

std::shared_ptr<MeshData> Font::createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment)
{    
    float size_scale = text_size / float(pixel_size);
    float line_spacing = getLineSpacing(pixel_size) * size_scale;

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    
    vertices.reserve(s.size() * 6);
    sp::Vector2f position;
    int previous_character_index = -1;
    int line_count = 1;
    float max_line_width = 0;
    float current_line_width = 0;
    unsigned int line_start_vertex_index = 0;
    
    for(unsigned int index=0; index<s.size(); )
    {
        if (previous_character_index > -1)
        {
            position.x += getKerning(&s[previous_character_index], &s[index]) * size_scale;
        }
        previous_character_index = index;

        if (s[index] == '\n')
        {
            position.x = 0;
            position.y -= line_spacing;
            line_count++;
            switch(alignment)
            {
            case Alignment::TopLeft:
            case Alignment::BottomLeft:
            case Alignment::Left:
                break;
            case Alignment::Center:
            case Alignment::Top:
            case Alignment::Bottom:
                if (current_line_width < max_line_width)
                {
                    float offset = (max_line_width - current_line_width) / 2.0;
                    for(unsigned int n=line_start_vertex_index; n<vertices.size(); n++)
                        vertices[n].position[0] += offset;
                }
                else
                {
                    float offset = (current_line_width - max_line_width) / 2.0;
                    for(unsigned int n=0; n<line_start_vertex_index; n++)
                        vertices[n].position[0] += offset;
                }
                break;
            case Alignment::TopRight:
            case Alignment::Right:
            case Alignment::BottomRight:
                if (current_line_width < max_line_width)
                {
                    float offset = max_line_width - current_line_width;
                    for(unsigned int n=line_start_vertex_index; n<vertices.size(); n++)
                        vertices[n].position[0] += offset;
                }
                else
                {
                    float offset = current_line_width - max_line_width;
                    for(unsigned int n=0; n<line_start_vertex_index; n++)
                        vertices[n].position[0] += offset;
                }
                break;
            }
            max_line_width = std::max(max_line_width, current_line_width);
            current_line_width = 0;
            line_start_vertex_index = vertices.size();
            index += 1;
            continue;
        }
        
        GlyphInfo glyph;
        if (!getGlyphInfo(&s[index], pixel_size, glyph))
        {
            glyph.consumed_characters = 1;
            glyph.advance = 0;
            glyph.bounds.size.x = 0;
        }

        if (glyph.bounds.size.x > 0.0)
        {
            float u0 = glyph.uv_rect.position.x;
            float v0 = glyph.uv_rect.position.y;
            float u1 = glyph.uv_rect.position.x + glyph.uv_rect.size.x;
            float v1 = glyph.uv_rect.position.y + glyph.uv_rect.size.y;
            
            float left = position.x + glyph.bounds.position.x * size_scale;
            float right = position.x + glyph.bounds.position.x * size_scale + glyph.bounds.size.x * size_scale;
            float top = position.y + glyph.bounds.position.y * size_scale;
            float bottom = position.y + glyph.bounds.position.y * size_scale - glyph.bounds.size.y * size_scale;

            Vector3f p0(left, top, 0.0f);
            Vector3f p1(right, top, 0.0f);
            Vector3f p2(left, bottom, 0.0f);
            Vector3f p3(right, bottom, 0.0f);

            indices.emplace_back(vertices.size() + 0);
            indices.emplace_back(vertices.size() + 2);
            indices.emplace_back(vertices.size() + 1);
            indices.emplace_back(vertices.size() + 2);
            indices.emplace_back(vertices.size() + 3);
            indices.emplace_back(vertices.size() + 1);

            vertices.emplace_back(p0, sp::Vector2f(u0, v0));
            vertices.emplace_back(p1, sp::Vector2f(u1, v0));
            vertices.emplace_back(p2, sp::Vector2f(u0, v1));
            vertices.emplace_back(p3, sp::Vector2f(u1, v1));
        }
        current_line_width = std::max(current_line_width, position.x + (glyph.bounds.position.x + glyph.bounds.size.x) * size_scale);
        position.x += glyph.advance * size_scale;
        index += glyph.consumed_characters;
    }

    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        break;
    case Alignment::Center:
    case Alignment::Top:
    case Alignment::Bottom:
        if (current_line_width < max_line_width)
        {
            float offset = (max_line_width - current_line_width) / 2.0;
            for(unsigned int n=line_start_vertex_index; n<vertices.size(); n++)
                vertices[n].position[0] += offset;
        }
        else
        {
            float offset = (current_line_width - max_line_width) / 2.0;
            for(unsigned int n=0; n<line_start_vertex_index; n++)
                vertices[n].position[0] += offset;
        }
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        if (current_line_width < max_line_width)
        {
            float offset = max_line_width - current_line_width;
            for(unsigned int n=line_start_vertex_index; n<vertices.size(); n++)
                vertices[n].position[0] += offset;
        }
        else
        {
            float offset = current_line_width - max_line_width;
            for(unsigned int n=0; n<line_start_vertex_index; n++)
                vertices[n].position[0] += offset;
        }
        break;
    }

    max_line_width = std::max(max_line_width, current_line_width);

    float x_offset = 0;
    float y_offset = 0;
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        x_offset = 0;
        break;
    case Alignment::Top:
    case Alignment::Center:
    case Alignment::Bottom:
        x_offset = (area_size.x - max_line_width) / 2;
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        x_offset = area_size.x - max_line_width;
        break;
    }
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        y_offset = area_size.y - line_spacing;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        y_offset = (area_size.y - line_spacing * (line_count - 1)) / 2;
        y_offset -= getBaseline(pixel_size) * size_scale * 0.5;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        y_offset = line_spacing * (line_count - 1);
        break;
    }
    for(auto& vertex : vertices)
    {
        vertex.position[0] += x_offset;
        vertex.position[1] += y_offset;
    }
    
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

class FreetypeFontTexture : public Texture
{
public:
    FreetypeFontTexture(string name, int pixel_size)
    : Texture(Type::Dynamic, name)
    {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture_size = Vector2i(pixel_size * 16, pixel_size * 16);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    FreetypeFont::GlyphInfo loadGlyph(FT_Face face, int character)
    {
        FreetypeFont::GlyphInfo info;
        info.bounds = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.uv_rect = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.advance = 0;
        info.consumed_characters = 1;
        
        int glyph_index = FT_Get_Char_Index(face, character);
        if (glyph_index == 0)
            return info;
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) != 0)
            return info;

        FT_Glyph glyph;
        if (FT_Get_Glyph(face->glyph, &glyph) != 0)
            return info;
        
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        FT_Bitmap& bitmap = FT_BitmapGlyph(glyph)->bitmap;
        
        info.advance = float(face->glyph->metrics.horiAdvance) / float(1 << 6);
        info.bounds.position.x = float(face->glyph->metrics.horiBearingX) / float(1 << 6);
        info.bounds.position.y = float(face->glyph->metrics.horiBearingY) / float(1 << 6);
        info.bounds.size.x = float(face->glyph->metrics.width) / float(1 << 6);
        info.bounds.size.y = float(face->glyph->metrics.height) / float(1 << 6);
        
        int row_index = findRowFor(Vector2i(bitmap.width + 2, bitmap.rows + 2));
        
        const uint8_t* src_pixels = bitmap.buffer;
        std::vector<uint8_t> dst_pixels;
        dst_pixels.resize(bitmap.width * bitmap.rows * 4, 255);
        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            sp2assert(false, "TODO");
        }
        else
        {
            for(unsigned int y=0; y<bitmap.rows; y++)
            {
                for(unsigned int x=0; x<bitmap.width; x++)
                    dst_pixels[(x + y * bitmap.width) * 4 + 3] = *src_pixels++;
                src_pixels += bitmap.pitch - bitmap.width;
            }
        }
        
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rows[row_index].x + 1, rows[row_index].y - 1 - bitmap.rows, bitmap.width, bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, dst_pixels.data());
        revision++;
        
        info.uv_rect.position.x = float(rows[row_index].x + 1) / float(texture_size.x);
        info.uv_rect.position.y = float(rows[row_index].y - 1 - bitmap.rows) / float(texture_size.y);
        info.uv_rect.size.x = float(bitmap.width) / float(texture_size.x);
        info.uv_rect.size.y = float(bitmap.rows) / float(texture_size.y);
        
        rows[row_index].x += bitmap.width + 2;
        FT_Done_Glyph(glyph);
        
        return info;
    }

    virtual void bind() override
    {
        glBindTexture(GL_TEXTURE_2D, handle);
    }
private:
    int findRowFor(Vector2i size)
    {
        int y = 0;
        for(unsigned int index=0; index<rows.size(); index++)
        {
            int row_height = rows[index].y - y;
            if (size.y <= row_height && size.y >= row_height / 2)
            {
                if (rows[index].x + size.x < int(texture_size.x))
                    return index;
            }
            y = rows[index].y;
        }
        rows.emplace_back(Vector2i(0, y + size.y * 1.1));
        return rows.size() - 1;
    }

    unsigned int handle;
    Vector2i texture_size;
    std::vector<Vector2i> rows; //Keep track of the bottom and right most pixel of each glyph row.
};

static unsigned long ft_stream_read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    io::ResourceStreamPtr& stream = *static_cast<io::ResourceStreamPtr*>(rec->descriptor.pointer);
    if (stream->seek(offset) == int64_t(offset))
    {
        if (count > 0)
            return stream->read(reinterpret_cast<char*>(buffer), count);
        else
            return 0;
    }
    else
        return count > 0 ? 0 : 1; // error code is 0 if we're reading, or nonzero if we're seeking
}
void ft_stream_close(FT_Stream)
{
}

FreetypeFont::FreetypeFont(string name, io::ResourceStreamPtr stream)
{
    ft_library = nullptr;
    ft_face = nullptr;

    font_resource_stream = stream;
    LOG(Info, "Loading font:", name);
    
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0)
    {
        LOG(Error, "Failed to initialize freetype library");
        return;
    }

    FT_StreamRec* stream_rec = new FT_StreamRec;
    memset(stream_rec, 0, sizeof(FT_StreamRec));
    stream_rec->base = nullptr;
    stream_rec->size = stream->getSize();
    stream_rec->pos = 0;
    stream_rec->descriptor.pointer = &font_resource_stream;
    stream_rec->read = &ft_stream_read;
    stream_rec->close = &ft_stream_close;

    // Setup the FreeType callbacks that will read our stream
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = stream_rec;
    args.driver = 0;

    // Load the new font face from the specified stream
    FT_Face face;
    if (FT_Open_Face(library, &args, 0, &face) != 0)
    {
        LOG(Error, "Failed to create font from stream:", name);
        FT_Done_FreeType(library);
        return;
    }

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        LOG(Error, "Failed to select unicode for font:", name);
        FT_Done_Face(face);
        delete stream_rec;
        FT_Done_FreeType(library);
        return;
    }

    ft_library = library;
    ft_stream_rec = stream_rec;
    ft_face = face;
}

FreetypeFont::~FreetypeFont()
{
    if (ft_face) FT_Done_Face((FT_Face)ft_face);
    if (ft_stream_rec) delete (FT_StreamRec*)ft_stream_rec;
    if (ft_library) FT_Done_FreeType((FT_Library)ft_library);
}

Texture* FreetypeFont::getTexture(int pixel_size)
{
    const auto& it = texture_cache.find(pixel_size);
    if (it != texture_cache.end())
        return it->second;
    return nullptr;
}

bool FreetypeFont::getGlyphInfo(const char* str, int pixel_size, Font::GlyphInfo& info)
{
    std::unordered_map<int, GlyphInfo>& known_glyphs = loaded_glyphs[pixel_size];

    if (known_glyphs.find(*str) == known_glyphs.end())
    {
        known_glyphs[*str] = texture_cache[pixel_size]->loadGlyph(FT_Face(ft_face), *str);
    }
    info = known_glyphs[*str];
    return true;
}

float FreetypeFont::getLineSpacing(int pixel_size)
{
    if (((FT_Face)ft_face)->size->metrics.x_ppem != pixel_size)
    {
        FT_Set_Pixel_Sizes((FT_Face)ft_face, 0, pixel_size);
    }
    if (texture_cache.find(pixel_size) == texture_cache.end())
    {
        texture_cache[pixel_size] = new FreetypeFontTexture(name, pixel_size);
    }
    return float(((FT_Face)ft_face)->size->metrics.height) / float(1 << 6);
}

float FreetypeFont::getBaseline(int pixel_size)
{
    return getLineSpacing(pixel_size) * 0.6;
}

float FreetypeFont::getKerning(const char* previous, const char* current)
{
    // Apply the kerning offset
    if (FT_HAS_KERNING((FT_Face)ft_face))
    {
        FT_Vector kerning;
        FT_Get_Kerning((FT_Face)ft_face, FT_Get_Char_Index((FT_Face)ft_face, *previous), FT_Get_Char_Index((FT_Face)ft_face, *current), FT_KERNING_DEFAULT, &kerning);
        if (!FT_IS_SCALABLE((FT_Face)ft_face))
            return float(kerning.x);
        else
            return float(kerning.x) / float(1 << 6);
    }
    return 0;
}

BitmapFont::BitmapFont(string name, io::ResourceStreamPtr stream)
: name(name)
{
    Vector2f texture_size, pixel_glyph_size;
    std::vector<string> lines;
    std::vector<string> special_lines;
    glyph_advance = sp::Vector2f(1, 1);
    while(stream->tell() != stream->getSize())
    {
        string line = stream->readLine();
        int seperator = line.find(":");
        if (seperator < 0)
        {
            if (line.strip() != "") LOG(Warning, "Ignoring line in bitmap font file:", line);
            continue;
        }
        string key = line.substr(0, seperator).strip().lower();
        string value = line.substr(seperator + 1).strip();
        if (key == "texture")
        {
            texture = texture_manager.get(value);
        }
        else if (key == "texture_size")
        {
            texture_size = stringutil::convert::toVector2f(value);
        }
        else if (key == "glyph_size")
        {
            pixel_glyph_size = stringutil::convert::toVector2f(value);
        }
        else if (key == "line")
        {
            lines.push_back(value);
        }
        else if (key == "special")
        {
            special_lines.push_back(value);
        }
        else if (key == "advance")
        {
            glyph_advance = stringutil::convert::toVector2f(value);
        }
        else
        {
            LOG(Warning, "Ignoring line in bitmap font file:", line);
        }
    }
    glyph_size = Vector2f(pixel_glyph_size.x / texture_size.x, pixel_glyph_size.y / texture_size.y);
    int y = 0;
    for(string line : lines)
    {
        int x = 0;
        for(int character : line)
        {
            glyphs[character] = Vector2f(glyph_size.x * x, glyph_size.y * y);
            x++;
        }
        y++;
    }
    for(string line : special_lines)
    {
        std::vector<string> parts = line.split(",", 3);
        if (parts.size() != 4)
        {
            LOG(Warning, "Ignoring malformed special line in font:", line);
            continue;
        }
        int x = stringutil::convert::toInt(parts[0].strip());
        int y = stringutil::convert::toInt(parts[1].strip());
        int w = stringutil::convert::toInt(parts[2].strip());
        
        specials[parts[3].strip()] = Rect2f(Vector2f(glyph_size.x * x, glyph_size.y * y), Vector2f(glyph_size.x * w, glyph_size.y));
    }
}

bool BitmapFont::getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info)
{
    for(auto it : specials)
    {
        if (strncmp(it.first.c_str(), str, it.first.length()) == 0)
        {
            info.uv_rect = it.second;
            info.bounds = {0, pixel_size * it.second.size.y / glyph_size.y, pixel_size * it.second.size.x / glyph_size.x, pixel_size * it.second.size.y / glyph_size.y};
            info.advance = pixel_size * glyph_advance.x * it.second.size.x / glyph_size.x;
            info.consumed_characters = it.first.length();
            return true;
        }
    }
    if (*str == ' ')
    {
        info.uv_rect = {0, 0, 0, 0};
        info.bounds = {0, 0, 0, 0};
        info.advance = pixel_size * glyph_advance.x;
        info.consumed_characters = 1;
        return true;
    }
    auto it = glyphs.find(*str);
    if (it == glyphs.end())
        return false;
    info.uv_rect = {it->second.x, it->second.y, glyph_size.x, glyph_size.y};
    info.bounds = {0, float(pixel_size), float(pixel_size), float(pixel_size)};
    info.advance = pixel_size * glyph_advance.x;
    info.consumed_characters = 1;
    return true;
}

float BitmapFont::getLineSpacing(int pixel_size)
{
    return glyph_advance.y * pixel_size;
}

float BitmapFont::getBaseline(int pixel_size)
{
    return getLineSpacing(pixel_size);
}

float BitmapFont::getKerning(const char* previous, const char* current)
{
    return 0;
}

Texture* BitmapFont::getTexture(int pixel_size)
{
    return texture;
}

};//namespace sp
