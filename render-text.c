#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <png.h>

typedef struct {
  char *font_file;
  char *text;
  int size;
  int anti_alias;
} conf_t;

char *parse_args(int argc, char **argv, conf_t *conf) {
  memset(conf, 0, sizeof(conf_t));
  conf->size = 72;
  if (argc < 2) {
    return "missing font file argument";
  } else if (argc < 3) {
    return "missing text argument";
  }
  conf->font_file = argv[1];
  conf->text = argv[2];
  if (argc > 3) {
    conf->size = atoi(argv[3]);
  }
  if (argc > 4) {
    conf->anti_alias = strcmp(argv[4], "yes") == 0;
  }
  return NULL;
}

char *render_glyph(FT_Face *face, conf_t conf) {
  FT_Library ft;
  FT_Error err;

  err = FT_Init_FreeType(&ft);
  if (err) return "freetype init error";

  err = FT_New_Face(ft, conf.font_file, 0, face);
  if (err == FT_Err_Unknown_File_Format)
    return "unknown font file format";
  else if (err)
    return "error reading font file";

  err = FT_Set_Pixel_Sizes(*face, 0, conf.size);
  if (err) return "error setting font size";

  FT_UInt index = FT_Get_Char_Index(*face, *conf.text);
  if (index == 0) return "no glyph found for char";

  err = FT_Load_Glyph(*face, index, FT_LOAD_DEFAULT);
  if (err) return "error loading glyph";

  err = FT_Render_Glyph((*face)->glyph, conf.anti_alias ?
                        FT_RENDER_MODE_NORMAL :
                        FT_RENDER_MODE_MONO);
  if (err) return "error rendering glyph";

  return NULL;
}

char *render_png(FT_Face face, char *out, int aa) {
  FILE *f = fopen(out, "wb");
  if (!f) return "failed to open output file";

  png_structp png_out = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_out) return "failed to create png write struct";
  png_infop png_info = png_create_info_struct(png_out);
  if (!png_info)
    return "failed to create png info struct";

  if (setjmp(png_jmpbuf(png_out)))
    return "png init io error";
  png_init_io(png_out, f);

  if (setjmp(png_jmpbuf(png_out)))
    return "IHDR write error";
  png_set_IHDR(png_out,
               png_info,
               face->glyph->bitmap.width,
               face->glyph->bitmap.rows,
               aa ? 8 : 1,
               PNG_COLOR_TYPE_GRAY,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_out, png_info);

  if (setjmp(png_jmpbuf(png_out)))
    return "png write error";

  for (int i = 0; i < face->glyph->bitmap.rows; ++i) {
    const unsigned char *rowptr = face->glyph->bitmap.buffer +
      (face->glyph->bitmap.pitch * i);
    png_write_row(png_out, rowptr);
  }

  if (setjmp(png_jmpbuf(png_out)))
    return "png end error";
  png_write_end(png_out, NULL);

  fclose(f);
  return NULL;
}

int main(int argc, char **argv) {
  conf_t conf;
  char *conf_err = parse_args(argc, argv, &conf);
  if (conf_err != NULL) {
    printf("Error: %s\n", conf_err);
    puts("Usage: render-text <font> <text> [<size> [<anti-alias>]]");
    puts("Example: render-text myfont.ttf foobar 24 yes");
    puts("         render-text myfont.ttf lorem");
    return 1;
  }

  printf("font: %s, text: %s, size: %d, anti-alias: %s\n",
         conf.font_file,
         conf.text,
         conf.size,
         conf.anti_alias ? "yes" : "no");

  FT_Face face;
  char *ft_err = render_glyph(&face, conf);
  if (ft_err != NULL) {
    printf("freetype error: %s\n", ft_err);
    return 2;
  }

  printf("bitmap rows: %d, width: %d\n",
         face->glyph->bitmap.rows,
         face->glyph->bitmap.width);

  char *png_err = render_png(face, "a.png", conf.anti_alias);
  if (png_err != NULL) {
    printf("png error: %s\n", png_err);
    return 3;
  }

  return 0;
}
