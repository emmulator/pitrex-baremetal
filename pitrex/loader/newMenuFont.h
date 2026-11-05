// The minimalist vector font, an option for the new menu (loader.ini: NEW_MENU_FONT = MINIMALIST).
// It is the font of uvmc2-menu: every letter is drawn with as few strokes as possible, so a screen with a lot
// of text takes less beam time and flickers less. The built-in PiTrex font stays the default.
//
// A glyph is pen paths separated by spaces; a path is a run of "xy" digit pairs on a grid 0-4 wide and 0-6 high,
// y up, origin at the bottom left: "460340 0333" is E (in to the middle and out again, then the bar). Each
// character takes 6 grid steps (4 and a gap of 2). One grid step is 1.5 * textSize * 15 vector units, so
// a text of size N is as wide as the same text in the built-in font and 3/4 as high.
// newMenuFontMinimalist (loader.ini NEW_MENU_FONT = MINIMALIST) is a global of the includer.
// The part that needs the PiTrex library (nmPrintMinimalist) is only compiled where it exists; the glyph table is tested on
// a PC (new_menu/tools/test_minimalist_font.c).
#ifndef NEW_MENU_FONT_H
#define NEW_MENU_FONT_H

#define NM_MIN_FIRST 33
#define NM_MIN_LAST  95
static const char *const nmMinGlyphs[NM_MIN_LAST - NM_MIN_FIRST + 1] = {
  /* ! */ "2623 2021",
  /* " */ "1614 3634",
  /* # */ "1036 3016 0242 0444",
  /* $ */ "0004152524444000"   /* a folder: the menu marks entries that open with it */,
  /* % */ "0046 0605 4041",
  /* & */ "40163602103042",
  /* ' */ "2624",
  /* ( */ "36141230",
  /* ) */ "16343210",
  /* * */ "1135 1531 0343",
  /* + */ "0343 2125",
  /* , */ "2110",
  /* - */ "0343",
  /* . */ "2021",
  /* / */ "0046",
  /* 0 */ "0006464000",
  /* 1 */ "152620",
  /* 2 */ "0526450040",
  /* 3 */ "064300 0343",
  /* 4 */ "30360242",
  /* 5 */ "4606034100",
  /* 6 */ "26004013",
  /* 7 */ "064610",
  /* 8 */ "0646004006",
  /* 9 */ "33064620",
  /* : */ "2425 2122",
  /* ; */ "2425 2110",
  /* < */ "450341",
  /* = */ "0242 0444",
  /* > */ "054301",
  /* ? */ "0526452322 2021",
  /* @ */ "321234314146060040",
  /* A */ "002640",
  /* B */ "000645034100",
  /* C */ "460340",
  /* D */ "00064300",
  /* E */ "460340 0333",
  /* F */ "000646 0333",
  /* G */ "4606004023",
  /* H */ "0006 4640 0343",
  /* I */ "2026",
  /* J */ "464002",
  /* K */ "0006 460340",
  /* L */ "060040",
  /* M */ "0006234640",
  /* N */ "00064046",
  /* O */ "2643200326",
  /* P */ "00064403",
  /* Q */ "2643200326 2240",
  /* R */ "0006440340",
  /* S */ "46054100",
  /* T */ "0646 2026",
  /* U */ "06004046",
  /* V */ "062046",
  /* W */ "0610233046",
  /* X */ "0046 0640",
  /* Y */ "062346 2320",
  /* Z */ "06460040",
  /* [ */ "36161030",
  /* \ */ "0640",
  /* ] */ "16363010",
  /* ^ */ "042644",
  /* _ */ "0040",
};

// The glyph of a character (lower case is drawn as upper case); 0 for a space or a character the font does not have.
static const char *nmMinGlyph(int c)
{
  if (c >= 'a' && c <= 'z') c -= 32;
  if (c < NM_MIN_FIRST || c > NM_MIN_LAST) return 0;
  return nmMinGlyphs[c - NM_MIN_FIRST];
}

#ifdef NM_MIN_DRAW
// The same call as v_printString: x and y in Vectrex units (the baseline of the text), size, brightness.
static void nmPrintMinimalist(int8_t x, int8_t y, char *string, uint8_t textSize, uint8_t brightness)
{
  extern uint8_t intensityMin, intensityMax;
  extern float intensityMul;
  if (!usePipeline) { v_printString(x, y, string, textSize, brightness); return; }
  SET_OPTIMAL_SCALE(180 * textSize, 180 * textSize);       // the scale v_printString picks for this size
  const int step = textSize * 45 / 2;
  int ox = x * 128, oy = y * 128;
  brightness = brightness * intensityMul;
  if (brightness != 0)
  {
    if (brightness > intensityMax) brightness = intensityMax;
    if (brightness < intensityMin) brightness = intensityMin;
  }
  for (; *string; string++, ox += 6 * step)
  {
    const char *g = nmMinGlyph((unsigned char) *string);
    if (!g) continue;
    while (*g)
    {
      int px = ox + (g[0] - '0') * step, py = oy + (g[1] - '0') * step;
      g += 2;
      while (*g && *g != ' ')
      {
        int nx = ox + (g[0] - '0') * step, ny = oy + (g[1] - '0') * step;
        v_directDraw32Hinted(px, py, nx, ny, brightness, (commonHints | PL_BASE_FORCE_USE_FIX_SIZE));
        px = nx; py = ny;
        g += 2;
      }
      if (*g == ' ') g++;
    }
  }
}
#endif

#endif
