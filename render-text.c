#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

  return 0;
}
