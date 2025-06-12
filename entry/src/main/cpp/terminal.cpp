// for standalone build to test on Linux:
// g++ terminal.cpp -I/usr/include/freetype2 -DSTANDALONE -lfreetype -lGLESv2 -lglfw -o terminal

#include "terminal.h"
#include <GLES3/gl32.h>
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <deque>
#include <fcntl.h>
#include <map>
#include <poll.h>
#include <pty.h>
#include <set>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef STANDALONE
// https://stackoverflow.com/questions/5343190/how-do-i-replace-all-instances-of-a-string-with-another-string
std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void custom_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string new_fmt = fmt;
    new_fmt = ReplaceString(new_fmt, "%{public}", "%");
    new_fmt += "\n";
    vfprintf(stderr, new_fmt.c_str(), args);
}
#define OH_LOG_ERROR(tag, fmt, ...) custom_printf(fmt, __VA_ARGS__)
#define OH_LOG_INFO(tag, fmt, ...) custom_printf(fmt, __VA_ARGS__)
#define OH_LOG_WARN(tag, fmt, ...) custom_printf(fmt, __VA_ARGS__)
#else
#include "hilog/log.h"
#undef LOG_TAG
#define LOG_TAG "testTag"
#endif

// font weight
enum font_weight {
    regular = 0,
    bold = 1,
    NUM_WEIGHT,
};

// maintain terminal status
struct term_style {
    // font weight
    font_weight weight = regular;
    // blinking
    bool blink = false;
    // foreground color
    float fg_red = 0.0;
    float fg_green = 0.0;
    float fg_blue = 0.0;
    // background color
    float bg_red = 1.0;
    float bg_green = 1.0;
    float bg_blue = 1.0;
};

// character in terminal
struct term_char {
    uint32_t ch = ' ';
    term_style style;
};

// docs for escape codes:
// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
// https://vt100.net/docs/vt220-rm/chapter4.html
// https://espterm.github.io/docs/VT100%20escape%20codes.html
// https://ecma-international.org/wp-content/uploads/ECMA-48_5th_edition_june_1991.pdf
// https://xtermjs.org/docs/api/vtfeatures/

static int fd = -1;

static int MAX_HISTORY_LINES = 5000;
static std::deque<std::vector<term_char>> history;
enum escape_states {
    state_idle,
    state_esc,
    state_csi,
    state_osc,
    state_dcs,
};
static escape_states escape_state = state_idle;
enum utf8_states {
    state_initial,
    state_2byte_2,        // expected 2nd byte of 2-byte sequence
    state_3byte_2_e0,     // expected 2nd byte of 3-byte sequence starting with 0xe0
    state_3byte_2_non_e0, // expected 2nd byte of 3-byte sequence starting with non-0xe0
    state_3byte_3,        // expected 3rd byte of 3-byte sequence
    state_4byte_2_f0,     // expected 2nd byte of 4-byte sequence starting with 0xf0
    state_4byte_2_f1_f3,  // expected 2nd byte of 4-byte sequence starting with 0xf1 to 0xf3
    state_4byte_2_f4,     // expected 2nd byte of 4-byte sequence starting with 0xf4
    state_4byte_3,        // expected 3rd byte of 4-byte sequence
    state_4byte_4,        // expected 4th byte of 4-byte sequence
};
static utf8_states utf8_state = state_initial;
static uint32_t current_utf8 = 0;
static std::string escape_buffer;
static term_style current_style;
static int width = 0;
static int height = 0;
static bool show_cursor = true;
// DECAWM, Autowrap Mode
// https://vt100.net/docs/vt510-rm/DECAWM.html
static bool autowrap = true;
// DECSCNM, Reverse Video
static bool reverse_video = false;
// DECOM, Origin Mode
static bool origin_mode = false;
static int tab_size = 8;
// columns where tab stops
static std::vector<bool> tab_stops;
// save/restore feature
static int save_row = 0;
static int save_col = 0;
static term_style save_style;
static GLint surface_location = -1;
static GLint render_pass_location = -1;
#ifdef STANDALONE
static int font_height = 24;
static int font_width = 12;
static int max_font_width = 24;
static int baseline_height = 5;
#else
static int font_height = 48;
static int font_width = 24;
static int max_font_width = 48;
static int baseline_height = 10;
#endif
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// terminal content
static std::vector<std::vector<term_char>> terminal;
// terminal size
static int term_col = 0;
static int term_row = 0;
// cursor location
static int row = 0;
static int col = 0;
// scroll offset in y axis
static float scroll_offset = 0;
// DECSTBM, scrolling region
static int scroll_top = 0;
static int scroll_bottom = term_row - 1;

// https://learnopengl.com/In-Practice/Text-Rendering
struct ivec2 {
    int x;
    int y;

    ivec2(int x, int y) {
        this->x = x;
        this->y = y;
    }
    ivec2() { this->x = this->y = 0; }
};

// glyph info
struct character {
    // location within the large texture
    float left;
    float right;
    float top;
    float bottom;
    // x, y offset from origin for bearing etc.
    int xoff;
    int yoff;
    // glyph size
    int width;
    int height;
};

// record info for each character
// map from (codepoint, font weight) to character
static std::map<std::pair<uint32_t, enum font_weight>, struct character> characters;
// code points to load from the font
static std::set<uint32_t> codepoints_to_load;
// do we need to reload font due to missing glyphs?
static bool need_reload_font = false;
// id of texture for glyphs
static GLuint texture_id;

static void ResizeTo(int new_term_row, int new_term_col, bool update_viewport = true) {
    int old_term_col = term_col;
    term_row = new_term_row;
    term_col = new_term_col;

    // update scroll margin
    scroll_top = 0;
    scroll_bottom = term_row - 1;

    // update viewport
    if (update_viewport) {
        width = term_col * font_width;
        height = term_row * font_height;
    }

    terminal.resize(term_row);
    for (int i = 0; i < term_row; i++) {
        terminal[i].resize(term_col);
    }

    if (row > term_row - 1) {
        row = term_row - 1;
    }

    if (col > term_col - 1) {
        col = term_col - 1;
    }

    tab_stops.resize(term_col);
    for (int i = old_term_col;i < term_col;i += tab_size) {
        tab_stops[i] = true;
    }
}


static std::vector<std::string> splitString(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}


static void DropFirstRowIfOverflow() {
    if (row == scroll_bottom + 1) {
        // drop first row in scrolling margin
        assert(scroll_top < scroll_bottom);
        history.push_back(terminal[scroll_top]);
        terminal.erase(terminal.begin() + scroll_top);
        terminal.insert(terminal.begin() + scroll_bottom, std::vector<term_char>());
        terminal[scroll_bottom].resize(term_col);
        row--;

        while (history.size() > MAX_HISTORY_LINES) {
            history.pop_front();
        }
    }
}

static void InsertUtf8(uint32_t codepoint) {
    assert(row >= 0 && row < term_row);
    assert(col >= 0 && col < term_col);
    terminal[row][col].ch = codepoint;
    terminal[row][col].style = current_style;
    col++;
    if (col == term_col) {
        if (autowrap) {
            // wrap to next line
            col = 0;
            row++;
            DropFirstRowIfOverflow();
        } else {
            col = term_col - 1;
        }
    }
}

// clamp cursor to valid range
static void ClampCursor() {
    // clamp col
    if (col < 0) {
        col = 0;
    } else if (col > term_col - 1) {
        col = term_col - 1;
    }

    // clamp row
    if (origin_mode) {
        // limit cursor to scroll region
        if (row < scroll_top) {
            row = scroll_top;
        } else if (row > scroll_bottom) {
            row = scroll_bottom;
        }
    } else {
        // limit cursor to terminal
        if (row < 0) {
            row = 0;
        } else if (row > term_row - 1) {
            row = term_row - 1;
        }
    }
}

// set absolute cursor location
static void SetCursor(int new_row, int new_col) {
    if (origin_mode) {
        // origin mode, home position is the scrolling top
        row = new_row + scroll_top;
        col = new_col;
    } else {
        row = new_row;
        col = new_col;
    }
    ClampCursor();
}

// move cursor in relative position
static void MoveCursor(int row_diff, int col_diff) {
    ClampCursor();
    SetCursor(row + row_diff, col + col_diff);
}

void WriteFull(uint8_t *data, size_t length) {
    if (fd == -1) {
        return;
    }

    // pretty print
    std::string hex;
    for (int i = 0; i < length; i++) {
        if (data[i] >= 127 || data[i] < 32) {
            char temp[8];
            snprintf(temp, sizeof(temp), "\\x%02x", data[i]);
            hex += temp;
        } else {
            hex += (char)data[i];
        }
    }
    OH_LOG_INFO(LOG_APP, "Send: %{public}s", hex.c_str());

    int written = 0;
    while (written < length) {
        int size = write(fd, (uint8_t *)data + written, length - written);
        assert(size >= 0);
        written += size;
    }
}


// CAUTION: clobbers temp
#define read_int_or_default(def)                                                                                       \
    (temp = 0, (escape_buffer != "" ? sscanf(escape_buffer.c_str(), "%d", &temp) : temp = (def)), temp)

static void HandleCSI(uint8_t current) {
    int temp = 0;
    if (current >= 0x40 && current <= 0x7E) {
        // final byte in [0x40, 0x7E]
        if (current == 'A') {
            // CSI Ps A, CUU, move cursor up # lines
            int line = read_int_or_default(1);
            if (row >= scroll_top) {
                // do not move past scrolling margin
                MoveCursor(-std::min(line, row - scroll_top), 0);
            } else {
                // we are out of scrolling region, move nevertheless
                MoveCursor(-line, 0);
            }
        } else if (current == 'B') {
            // CSI Ps B, CUD, move cursor down # lines
            int line = read_int_or_default(1);
            if (row <= scroll_bottom) {
                // do not move past scrolling margin
                MoveCursor(std::min(line, scroll_bottom - row), 0);
            } else {
                // we are out of scrolling region, move nevertheless
                MoveCursor(line,  0);
            }
        } else if (current == 'C') {
            // CSI Ps C, CUF, move cursor right # columns
            col += read_int_or_default(1);
            ClampCursor();
        } else if (current == 'D') {
            // CSI Ps D, CUB, move cursor left # columns
            col -= read_int_or_default(1);
            ClampCursor();
        } else if (current == 'E') {
            // CSI Ps E, CNL, move cursor to the beginning of next line, down # lines
            row += read_int_or_default(1);
            col = 0;
            ClampCursor();
        } else if (current == 'F') {
            // CSI Ps F, CPL, move cursor to the beginning of previous line, up # lines
            row -= read_int_or_default(1);
            col = 0;
            ClampCursor();
        } else if (current == 'G') {
            // CSI Ps G, CHA, move cursor to column #
            col = read_int_or_default(1);
            // convert from 1-based to 0-based
            col--;
            ClampCursor();
        } else if (current == 'H') {
            std::vector<std::string> parts = splitString(escape_buffer, ";");
            if (parts.size() == 2) {
                // CSI Ps ; PS H, CUP, move cursor to x, y
                sscanf(parts[0].c_str(), "%d", &row);
                sscanf(parts[1].c_str(), "%d", &col);
                // convert from 1-based to 0-based
                row--;
                col--;
                ClampCursor();
            } else if (escape_buffer == "") {
                // CSI H, HOME, move cursor upper left corner
                row = col = 0;
            } else {
                goto unknown;
            }
        } else if (current == 'J') {
            // CSI Ps J, ED, erase in display
            if (escape_buffer == "" || escape_buffer == "0") {
                // CSI J, CSI 0 J
                // erase below
                for (int i = col; i < term_col; i++) {
                    terminal[row][i] = term_char();
                }
                for (int i = row + 1; i < term_row; i++) {
                    std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                }
            } else if (escape_buffer == "1") {
                // CSI 1 J
                // erase above
                for (int i = 0; i < row; i++) {
                    std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                }
                for (int i = 0; i <= col; i++) {
                    terminal[row][i] = term_char();
                }
            } else if (escape_buffer == "2") {
                // CSI 2 J
                // erase all
                for (int i = 0; i < term_row; i++) {
                    std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                }
            } else {
                goto unknown;
            }
        } else if (current == 'K') {
            // CSI Ps K, EL, erase in line
            if (escape_buffer == "" || escape_buffer == "0") {
                // CSI K, CSI 0 K
                // erase to right
                for (int i = col; i < term_col; i++) {
                    terminal[row][i] = term_char();
                }
            } else if (escape_buffer == "1") {
                // CSI 1 K
                // erase to left
                for (int i = 0; i <= col; i++) {
                    terminal[row][i] = term_char();
                }
            } else if (escape_buffer == "2") {
                // CSI 2 K
                // erase whole line
                for (int i = 0; i < term_col; i++) {
                    terminal[row][i] = term_char();
                }
            } else {
                goto unknown;
            }
        } else if (current == 'L') {
            // CSI Ps L, Insert Ps blank lines at active row
            int line = read_int_or_default(1);
            if (row < scroll_top || row > scroll_bottom) {
                // outside the scroll margins, do nothing
            } else {
                // insert lines from current row, add new rows from scroll bottom
                for (int i = scroll_bottom;i >= row;i --) {
                    if (i - line >= row) {
                        terminal[i] = terminal[i - line];
                    } else {
                        std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                    }
                }
                // set to first column
                col = 0;
            }
        } else if (current == 'M') {
            // CSI Ps M, Delete Ps lines at active row
            int line = read_int_or_default(1);
            if (row < scroll_top || row > scroll_bottom) {
                // outside the scroll margins, do nothing
            } else {
                // delete lines from current row, add new rows from scroll bottom
                for (int i = row;i <= scroll_bottom;i ++) {
                    if (i + line <= scroll_bottom) {
                        terminal[i] = terminal[i + line];
                    } else {
                        std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                    }
                }
                // set to first column
                col = 0;
            }
        } else if (current == 'P') {
            // CSI Ps P, DCH, delete # characters, move right to left
            int del = read_int_or_default(1);
            for (int i = col; i < term_col; i++) {
                if (i + del < term_col) {
                    terminal[row][i] = terminal[row][i + del];
                } else {
                    terminal[row][i] = term_char();
                }
            }
        } else if (current == 'S') {
            // CSI Ps S, SU, Scroll up Ps lines
            int line = read_int_or_default(1);
            for (int i = scroll_top; i <= scroll_bottom; i++) {
                if (i + line <= scroll_bottom) {
                    terminal[i] = terminal[i + line];
                } else {
                    std::fill(terminal[i].begin(), terminal[i].end(), term_char());
                }
            }
        } else if (current == 'X') {
            // CSI Ps X, ECH, erase # characters, do not move others
            int del = read_int_or_default(1);
            for (int i = col; i < col + del && i < term_col; i++) {
                terminal[row][i] = term_char();
            }
        } else if (current == 'c' && (escape_buffer == "" || escape_buffer == "0")) {
            // CSI Ps c, Send Device Attributes, Primary DA
            // mimic xterm
            // send CSI ? 1 ; 2 c: I am VT100 with Advance Video Option
            uint8_t send_buffer[] = {0x1b, '[', '?', '1', ';', '2', 'c'};
            WriteFull(send_buffer, sizeof(send_buffer));
        } else if (current == 'c' && (escape_buffer == ">" || escape_buffer == ">0")) {
            // CSI > Ps c, Send Device Attributes, Secondary DA
            // mimic xterm
            // send CSI > 0 ; 2 7 6 ; 0 c: I am VT100
            uint8_t send_buffer[] = {0x1b, '[', '>', '0', ';', '2', '7', '6', ';', '0', 'c'};
            WriteFull(send_buffer, sizeof(send_buffer));
        } else if (current == 'd' && escape_buffer != "") {
            // CSI Ps d, VPA, move cursor to row #
            sscanf(escape_buffer.c_str(), "%d", &row);
            // convert from 1-based to 0-based
            row--;
            ClampCursor();
        } else if (current == 'f') {
            std::vector<std::string> parts = splitString(escape_buffer, ";");
            if (parts.size() == 2) {
                // CSI Ps ; PS f, CUP, move cursor to x, y
                sscanf(parts[0].c_str(), "%d", &row);
                sscanf(parts[1].c_str(), "%d", &col);
                // convert from 1-based to 0-based
                row--;
                col--;
                ClampCursor();
            } else {
                goto unknown;
            }
        } else if (current == 'g') {
            int mode = read_int_or_default(0);
            if (mode == 0) {
                // CSI g, CSI 0 g, clear tab stop at the current position
                tab_stops[col] = false;
            } else if (mode == 3) {
                // CSI 3 g, clear all tab stops
                std::fill(tab_stops.begin(), tab_stops.end(), false);
            } else {
                goto unknown;
            }
        } else if (current == 'h' && escape_buffer.size() > 0 && escape_buffer[0] == '?') {
            // CSI ? Pm h, DEC Private Mode Set (DECSET)
            std::vector<std::string> parts = splitString(escape_buffer.substr(1), ";");
            for (auto part : parts) {
                if (part == "1") {
                    // CSI ? 1 h, Application Cursor Keys (DECCKM)
                    // TODO
                } else if (part == "3") {
                    // CSI ? 3 h, Enable 132 Column mode, DECCOLM
                    ResizeTo(term_row, 132);
                    ResizeWidth(132 * font_width);
                } else if (part == "5") {
                    // CSI ? 5 h, Reverse Video (DECSCNM)
                    reverse_video = true;
                } else if (part == "6") {
                    // CSI ? 6 h, Origin Mode (DECOM)
                    origin_mode = true;
                } else if (part == "7") {
                    // CSI ? 7 h, Set autowrap
                    autowrap = true;
                } else if (part == "12") {
                    // CSI ? 12 h, Start blinking cursor
                    // TODO
                } else if (part == "25") {
                    // CSI ? 25 h, DECTCEM, make cursor visible
                    show_cursor = true;
                } else if (part == "1000") {
                    // CSI ? 1000 h, Send Mouse X & Y on button press and release
                    // TODO
                } else if (part == "1002") {
                    // CSI ? 1002 h, Use Cell Motion Mouse Tracking
                    // TODO
                } else if (part == "1006") {
                    // CSI ? 1006 h, Enable SGR Mouse Mode
                    // TODO
                } else if (part == "2004") {
                    // CSI ? 2004 h, set bracketed paste mode
                    // TODO
                } else {
                    OH_LOG_WARN(LOG_APP, "Unknown CSI ? Pm h: %{public}s %{public}c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'l' && escape_buffer.size() > 0 && escape_buffer[0] == '?') {
            // CSI ? Pm l, DEC Private Mode Reset (DECRST)
            std::vector<std::string> parts = splitString(escape_buffer.substr(1), ";");
            for (auto part : parts) {
                if (part == "3") {
                    // CSI ? 3 l, 80 Column Mode (DECCOLM)
                    ResizeTo(term_row, 80);
                    ResizeWidth(80 * font_width);
                } else if (part == "5") {
                    // CSI ? 5 l, Normal Video (DECSCNM)
                    reverse_video = false;
                } else if (part == "6") {
                    // CSI ? 6 l, Normal Cursor Mode (DECOM)
                    origin_mode = false;
                } else if (part == "7") {
                    // CSI ? 7 l, Reset autowrap
                    autowrap = false;
                } else if (part == "12") {
                    // CSI ? 12 l, Stop blinking cursor
                    // TODO
                } else if (part == "25") {
                    // CSI ? 25 l, Hide cursor (DECTCEM)
                    show_cursor = true;
                } else if (part == "2004") {
                    // CSI ? 2004 l, reset bracketed paste mode
                    // TODO
                } else {
                    OH_LOG_WARN(LOG_APP, "Unknown CSI ? Pm l: %{public}s %{public}c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'm' && (escape_buffer.size() == 0 || escape_buffer[0] != '>')) {
            // CSI Pm m, Character Attributes (SGR)

            // set color
            std::vector<std::string> parts = splitString(escape_buffer, ";");
            for (auto part : parts) {
                int param = 0;
                sscanf(part.c_str(), "%d", &param);
                if (param == 0) {
                    // reset all attributes to their defaults
                    current_style = term_style();
                } else if (param == 1) {
                    // set bold, CSI 1 m
                    current_style.weight = font_weight::bold;
                } else if (param == 4) {
                    // set underline, CSI 4 m
                    // TODO
                } else if (param == 5) {
                    // set blink, CSI 5 m
                    current_style.blink = true;
                } else if (param == 7) {
                    // inverse
                    std::swap(current_style.fg_red, current_style.bg_red);
                    std::swap(current_style.fg_green, current_style.bg_green);
                    std::swap(current_style.fg_blue, current_style.bg_blue);
                } else if (param == 10) {
                    // reset to primary font
                    current_style = term_style();
                } else if (param == 30) {
                    // black foreground
                    current_style.fg_red = 0.0;
                    current_style.fg_green = 0.0;
                    current_style.fg_blue = 0.0;
                } else if (param == 31) {
                    // red foreground
                    current_style.fg_red = 1.0;
                    current_style.fg_green = 0.0;
                    current_style.fg_blue = 0.0;
                } else if (param == 32) {
                    // green foreground
                    current_style.fg_red = 0.0;
                    current_style.fg_green = 1.0;
                    current_style.fg_blue = 0.0;
                } else if (param == 33) {
                    // yellow foreground
                    current_style.fg_red = 1.0;
                    current_style.fg_green = 1.0;
                    current_style.fg_blue = 0.0;
                } else if (param == 34) {
                    // blue foreground
                    current_style.fg_red = 0.0;
                    current_style.fg_green = 0.0;
                    current_style.fg_blue = 1.0;
                } else if (param == 35) {
                    // magenta foreground
                    current_style.fg_red = 1.0;
                    current_style.fg_green = 0.0;
                    current_style.fg_blue = 1.0;
                } else if (param == 36) {
                    // cyan foreground
                    current_style.fg_red = 0.0;
                    current_style.fg_green = 1.0;
                    current_style.fg_blue = 1.0;
                } else if (param == 37) {
                    // white foreground
                    current_style.fg_red = 1.0;
                    current_style.fg_green = 1.0;
                    current_style.fg_blue = 1.0;
                } else if (param == 39) {
                    // default foreground
                    current_style.fg_red = 0.0;
                    current_style.fg_green = 0.0;
                    current_style.fg_blue = 0.0;
                } else if (param == 40) {
                    // black background
                    current_style.bg_red = 0.0;
                    current_style.bg_green = 0.0;
                    current_style.bg_blue = 0.0;
                } else if (param == 41) {
                    // black background
                    current_style.bg_red = 1.0;
                    current_style.bg_green = 0.0;
                    current_style.bg_blue = 0.0;
                } else if (param == 42) {
                    // green background
                    current_style.bg_red = 0.0;
                    current_style.bg_green = 1.0;
                    current_style.bg_blue = 0.0;
                } else if (param == 43) {
                    // yellow background
                    current_style.bg_red = 1.0;
                    current_style.bg_green = 1.0;
                    current_style.bg_blue = 0.0;
                } else if (param == 44) {
                    // blue background
                    current_style.bg_red = 0.0;
                    current_style.bg_green = 0.0;
                    current_style.bg_blue = 1.0;
                } else if (param == 45) {
                    // magenta background
                    current_style.bg_red = 1.0;
                    current_style.bg_green = 0.0;
                    current_style.bg_blue = 1.0;
                } else if (param == 46) {
                    // cyan background
                    current_style.bg_red = 0.0;
                    current_style.bg_green = 1.0;
                    current_style.bg_blue = 1.0;
                } else if (param == 47) {
                    // white background
                    current_style.bg_red = 1.0;
                    current_style.bg_green = 1.0;
                    current_style.bg_blue = 1.0;
                } else if (param == 49) {
                    // default background
                    current_style.bg_red = 1.0;
                    current_style.bg_green = 1.0;
                    current_style.bg_blue = 1.0;
                } else if (param == 90) {
                    // bright black foreground
                    current_style.fg_red = 0.5;
                    current_style.fg_green = 0.5;
                    current_style.fg_blue = 0.5;
                } else {
                    OH_LOG_WARN(LOG_APP, "Unknown CSI Pm m: %{public}s %{public}c",
                                escape_buffer.c_str(), current);
                }
            }
        } else if (current == 'm' && escape_buffer.size() > 0 && escape_buffer[0] == '>') {
            // CSI > Pp m, XTMODKEYS, set/reset key modifier options
            // TODO
        } else if (current == 'n' && escape_buffer == "6") {
            // CSI Ps n, DSR, Device Status Report
            // Ps = 6: Report Cursor Position (CPR)
            // send ESC [ row ; col R
            char send_buffer[128] = {};
            snprintf(send_buffer, sizeof(send_buffer), "\x1b[%d;%dR", row + 1, col + 1);
            int len = strlen(send_buffer);
            WriteFull((uint8_t *)send_buffer, len);
        } else if (current == 'r') {
            // CSI Ps ; Ps r, Set Scrolling Region [top;bottom]
            std::vector<std::string> parts = splitString(escape_buffer, ";");
            if (parts.size() == 2) {
                int new_top = 1;
                int new_bottom = term_row;
                sscanf(parts[0].c_str(), "%d", &new_top);
                sscanf(parts[1].c_str(), "%d", &new_bottom);
                // convert to 1-based
                new_top --;
                new_bottom --;
                if (new_bottom > new_top) {
                    scroll_top = new_top;
                    scroll_bottom = new_bottom;

                    // move cursor to new home position
                    row = scroll_top;
                    col = 0;
                }
            } else {
                goto unknown;
            }
        } else if (current == '@' &&
                ((escape_buffer.size() > 0 && escape_buffer[escape_buffer.size() - 1] >= '0' &&
                    escape_buffer[escape_buffer.size() - 1] <= '9') ||
                    escape_buffer == "")) {
            // CSI Ps @, ICH, Insert Ps (Blank) Character(s)
            int count = read_int_or_default(1);
            for (int i = term_col - 1; i >= col; i--) {
                if (i - col < count) {
                    terminal[row][col].ch = ' ';
                } else {
                    terminal[row][col] = terminal[row][col - count];
                }
            }
        } else {
unknown:
            // unknown
            OH_LOG_WARN(LOG_APP, "Unknown escape sequence in CSI: %{public}s %{public}c",
                        escape_buffer.c_str(), current);
        }
        escape_state = state_idle;
    } else if (current >= 0x20 && current <= 0x3F) {
        // parameter bytes in [0x30, 0x3F],
        // or intermediate bytes in [0x20, 0x2F]
        escape_buffer += current;
    } else {
        // invalid byte
        // unknown
        OH_LOG_WARN(LOG_APP, "Unknown escape sequence in CSI: %{public}s %{public}c",
                    escape_buffer.c_str(), current);
        escape_state = state_idle;
    }
}

static void *TerminalWorker(void *) {
    pthread_setname_np(pthread_self(), "terminal worker");

    int temp = 0;
    // poll from fd, and render
    struct timeval tv;
    while (1) {
        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLIN;
        int res = poll(fds, 1, 100);

        uint8_t buffer[1024];
        if (res > 0) {
            ssize_t r = read(fd, buffer, sizeof(buffer) - 1);
            if (r > 0) {
                // pretty print
                std::string hex;
                for (int i = 0; i < r; i++) {
                    if (buffer[i] >= 127 || buffer[i] < 32) {
                        char temp[8];
                        snprintf(temp, sizeof(temp), "\\x%02x", buffer[i]);
                        hex += temp;
                    } else {
                        hex += (char)buffer[i];
                    }
                }
                OH_LOG_INFO(LOG_APP, "Got: %{public}s", hex.c_str());

                // parse output
                pthread_mutex_lock(&lock);
                for (int i = 0; i < r; i++) {
                    if (escape_state == state_esc) {
                        if (buffer[i] == '[') {
                            // ESC [ = CSI
                            escape_state = state_csi;
                        } else if (buffer[i] == ']') {
                            // ESC ] = OSC
                            escape_state = state_osc;
                        } else if (buffer[i] == '=') {
                            // ESC =, enter alternate keypad mode
                            // TODO
                            escape_state = state_idle;
                        } else if (buffer[i] == '>') {
                            // ESC >, exit alternate keypad mode
                            // TODO
                            escape_state = state_idle;
                        } else if (buffer[i] == 'A') {
                            // ESC A, cursor up
                            row --;
                            ClampCursor();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'B') {
                            // ESC B, cursor down
                            row ++;
                            ClampCursor();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'C') {
                            // ESC C, cursor right
                            col ++;
                            ClampCursor();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'D') {
                            // ESC D, cursor left
                            col --;
                            ClampCursor();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'E') {
                            // ESC E, goto to the beginning of next row
                            row ++;
                            col = 0;
                            ClampCursor();
                            escape_state = state_idle;
                        } else if (buffer[i] == 'H') {
                            // ESC H, place tab stop at the current position
                            tab_stops[col] = true;
                            escape_state = state_idle;
                        } else if (buffer[i] == 'M') {
                            // ESC M, move cursor one line up, scrolls down if at the top margin
                            if (row == scroll_top) {
                                // shift rows down
                                for (int i = scroll_bottom;i > scroll_top;i--) {
                                    terminal[i] = terminal[i-1];
                                }
                                std::fill(terminal[scroll_top].begin(), terminal[scroll_top].end(), term_char());
                            } else {
                                row --;
                                ClampCursor();
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == 'P') {
                            // ESC P = DCS
                            escape_state = state_dcs;
                        } else if (i + 1 < r && buffer[i] == '#' && buffer[i + 1] == '8') {
                            // ESC # 8, DECALN fill viewport with a test pattern (E)
                            for (int i = 0;i < term_row;i++) {
                                for (int j = 0;j < term_col;j++) {
                                    terminal[i][j] = term_char();
                                    terminal[i][j].ch = 'E';
                                }
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] == '7') {
                            // ESC 7, save cursor
                            save_row = row;
                            save_col = col;
                            save_style = current_style;
                            escape_state = state_idle;
                        } else if (buffer[i] == '8') {
                            // ESC 8, restore cursor
                            row = save_row;
                            col = save_col;
                            ClampCursor();
                            current_style = save_style;
                            escape_state = state_idle;
                        } else if (buffer[i] == '(') {
                            // ESC ( C, Designate G0 Character Set
                            // TODO
                            i++;
                            escape_state = state_idle;
                        } else if (buffer[i] == ')') {
                            // ESC ) C, Designate G1 Character Set
                            // TODO
                            i++;
                            escape_state = state_idle;
                        } else {
                            // unknown
                            OH_LOG_WARN(LOG_APP, "Unknown escape sequence after ESC: %{public}s %{public}c",
                                        escape_buffer.c_str(), buffer[i]);
                            escape_state = state_idle;
                        }
                    } else if (escape_state == state_csi) {
                        HandleCSI(buffer[i]);
                    } else if (escape_state == state_osc) {
                        if (buffer[i] == '\x07') {
                            // OSC Ps ; Pt BEL
                            std::vector<std::string> parts = splitString(escape_buffer, ";");
                            if (parts.size() == 3 && parts[0] == "52" && parts[1] == "c" && parts[2] != "?") {
                                // OSC 52 ; c ; BASE64 BEL
                                // copy to clipboard
                                std::string base64 = parts[2];
                                OH_LOG_INFO(LOG_APP, "Copy to pasteboard in native: %{public}s",
                                            base64.c_str());
                                Copy(base64);
                            } else if (parts.size() == 3 && parts[0] == "52" && parts[1] == "c" && parts[2] == "?") {
                                // OSC 52 ; c ; ? BEL
                                // paste from clipboard
                                RequestPaste();
                                OH_LOG_INFO(LOG_APP, "Request Paste from pasteboard: %{public}s", escape_buffer.c_str());
                            }
                            escape_state = state_idle;
                        } else if (i + 1 < r && buffer[i] == '\x1b' && buffer[i + 1] == '\\') {
                            // ST is ESC \
                            // OSC Ps ; Pt ST
                            i += 1;
                            std::vector<std::string> parts = splitString(escape_buffer, ";");
                            if (parts.size() == 2 && parts[0] == "10" && parts[1] == "?") {
                                // OSC 10 ; ? ST
                                // report foreground color: black
                                // send OSI 10 ; r g b : 0 / 0 / 0 ST
                                uint8_t send_buffer[] = {0x1b, ']', '1', '0', ';', 'r', 'g', 'b', ':', '0', '/', '0', '/', '0', '\x1b', '\\'};
                                WriteFull(send_buffer, sizeof(send_buffer));
                            } else if (parts.size() == 2 && parts[0] == "11" && parts[1] == "?") {
                                // OSC 11 ; ? ST
                                // report background color: white
                                // send OSI 11 ; r g b : f / f / f ST
                                uint8_t send_buffer[] = {0x1b, ']', '1', '0', ';', 'r', 'g', 'b', ':', 'f', '/', 'f', '/', 'f', '\x1b', '\\'};
                                WriteFull(send_buffer, sizeof(send_buffer));
                            }
                            escape_state = state_idle;
                        } else if (buffer[i] >= ' ' && buffer[i] < 127) {
                            // printable character
                            escape_buffer += buffer[i];
                        } else {
                            // unknown
                            OH_LOG_WARN(LOG_APP, "Unknown escape sequence in OSC: %{public}s %{public}c",
                                        escape_buffer.c_str(), buffer[i]);
                            escape_state = state_idle;
                        }
                    } else if (escape_state == state_dcs) {
                        if (i + 1 < r && buffer[i] == '\x1b' && buffer[i] == '\\') {
                            // ST is ESC \
                            i += 1;
                            escape_state = state_idle;
                        } else if (buffer[i] >= ' ' && buffer[i] < 127) {
                            // printable character
                            escape_buffer += buffer[i];
                        } else {
                            // unknown
                            OH_LOG_WARN(LOG_APP, "Unknown escape sequence in DCS: %{public}s %{public}c",
                                        escape_buffer.c_str(), buffer[i]);
                            escape_state = state_idle;
                        }
                    } else if (escape_state == state_idle) {
                        // escape state is idle
                        if (utf8_state == state_initial) {
                            if (buffer[i] >= ' ' && buffer[i] <= 0x7f) {
                                // printable
                                InsertUtf8(buffer[i]);
                            } else if (buffer[i] >= 0xc2 && buffer[i] <= 0xdf) {
                                // 2-byte utf8
                                utf8_state = state_2byte_2;
                                current_utf8 = (uint32_t)(buffer[i] & 0x1f) << 6;
                            } else if (buffer[i] == 0xe0) {
                                // 3-byte utf8 starting with e0
                                utf8_state = state_3byte_2_e0;
                                current_utf8 = (uint32_t)(buffer[i] & 0x0f) << 12;
                            } else if (buffer[i] >= 0xe1 && buffer[i] <= 0xef) {
                                // 3-byte utf8 starting with non-e0
                                utf8_state = state_3byte_2_non_e0;
                                current_utf8 = (uint32_t)(buffer[i] & 0x0f) << 12;
                            } else if (buffer[i] == 0xf0) {
                                // 4-byte utf8 starting with f0
                                utf8_state = state_4byte_2_f0;
                                current_utf8 = (uint32_t)(buffer[i] & 0x07) << 18;
                            } else if (buffer[i] >= 0xf1 && buffer[i] <= 0xf3) {
                                // 4-byte utf8 starting with f1 to f3
                                utf8_state = state_4byte_2_f1_f3;
                                current_utf8 = (uint32_t)(buffer[i] & 0x07) << 18;
                            } else if (buffer[i] == 0xf4) {
                                // 4-byte utf8 starting with f4
                                utf8_state = state_4byte_2_f4;
                                current_utf8 = (uint32_t)(buffer[i] & 0x07) << 18;
                            } else if (buffer[i] == '\r') {
                                col = 0;
                            } else if (buffer[i] == '\n') {
                                // CUD1=\n, cursor down by 1
                                row += 1;
                                DropFirstRowIfOverflow();
                            } else if (buffer[i] == '\b') {
                                // CUB1=^H, cursor backward by 1
                                if (col > 0) {
                                    col -= 1;
                                }
                            } else if (buffer[i] == '\t') {
                                // goto next tab stop
                                col ++;
                                while (col < term_col && !tab_stops[col]) {
                                    col ++;
                                }
                                ClampCursor();
                            } else if (buffer[i] == 0x1b) {
                                escape_buffer = "";
                                escape_state = state_esc;
                            }
                        } else if (utf8_state == state_2byte_2) {
                            // expecting the second byte of 2-byte utf-8
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (buffer[i] & 0x3f);
                                InsertUtf8(current_utf8);
                            }
                            utf8_state = state_initial;
                        } else if (utf8_state == state_3byte_2_e0) {
                            // expecting the second byte of 3-byte utf-8 starting with 0xe0
                            if (buffer[i] >= 0xa0 && buffer[i] <= 0xbf) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 6;
                                utf8_state = state_3byte_3;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_3byte_2_non_e0) {
                            // expecting the second byte of 3-byte utf-8 starting with non-0xe0
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 6;
                                utf8_state = state_3byte_3;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_3byte_3) {
                            // expecting the third byte of 3-byte utf-8 starting with 0xe0
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (buffer[i] & 0x3f);
                                InsertUtf8(current_utf8);
                            }
                            utf8_state = state_initial;
                        } else if (utf8_state == state_4byte_2_f0) {
                            // expecting the second byte of 4-byte utf-8 starting with 0xf0
                            if (buffer[i] >= 0x90 && buffer[i] <= 0xbf) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 12;
                                utf8_state = state_4byte_3;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_4byte_2_f1_f3) {
                            // expecting the second byte of 4-byte utf-8 starting with 0xf0 to 0xf3
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 12;
                                utf8_state = state_4byte_3;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_4byte_2_f4) {
                            // expecting the second byte of 4-byte utf-8 starting with 0xf4
                            if (buffer[i] >= 0x80 && buffer[i] <= 0x8f) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 12;
                                utf8_state = state_4byte_3;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_4byte_3) {
                            // expecting the third byte of 4-byte utf-8
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (uint32_t)(buffer[i] & 0x3f) << 6;
                                utf8_state = state_4byte_4;
                            } else {
                                utf8_state = state_initial;
                            }
                        } else if (utf8_state == state_4byte_4) {
                            // expecting the third byte of 4-byte utf-8
                            if (buffer[i] >= 0x80 && buffer[i] <= 0xbf) {
                                current_utf8 |= (buffer[i] & 0x3f);
                                InsertUtf8(current_utf8);
                            }
                            utf8_state = state_initial;
                        } else {
                            assert(false && "unreachable utf8 state");
                        }
                    } else {
                        assert(false && "unreachable escape state");
                    }
                }
                pthread_mutex_unlock(&lock);
            }
        }

        // check if anything to paste
        std::string paste = GetPaste();
        if (paste.size() > 0) {
            // send OSC 52 ; c ; BASE64 ST
            OH_LOG_INFO(LOG_APP, "Paste from pasteboard: %{public}s",
                        paste.c_str());
            std::string resp = "\x1b]52;c;" + paste + "\x1b\\";
            WriteFull((uint8_t *)resp.c_str(), resp.size());
        }
    }
}

void Start() {
    if (fd != -1) {
        return;
    }

    // setup terminal, default to 80x24
    ResizeTo(80, 24);

    // fork & create pty
    struct winsize ws = {};
    ws.ws_col = term_col;
    ws.ws_row = term_row;

    int pid = forkpty(&fd, nullptr, nullptr, &ws);
    if (!pid) {
#ifdef STANDALONE
        execl("/bin/bash", "/bin/bash", nullptr);
#else
        // override HOME to /storage/Users/currentUser since it is writable
        const char *home = "/storage/Users/currentUser";
        setenv("HOME", home, 1);
        setenv("PWD", home, 1);
        // set LD_LIRBARY_PATH for shared libraries
        setenv("LD_LIBRARY_PATH", "/data/app/base.org/base_1.0/lib", 1);
        // override TMPDIR for tmux
        setenv("TMUX_TMPDIR", "/data/storage/el2/base/cache", 1);
        chdir(home);
        execl("/data/app/bin/bash", "/data/app/bin/bash", nullptr);
#endif
    }

    // set as non blocking
    int res = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    assert(res == 0);

    pthread_t terminal_thread;
    pthread_create(&terminal_thread, NULL, TerminalWorker, NULL);
}

void SendData(uint8_t *data, size_t length) {
    if (fd == -1) {
        return;
    }

    // reset scroll offset to bottom
    scroll_offset = 0.0;

    WriteFull(data, length);
}

// load font
// texture contains all glyphs of all weights:
// fixed width of max_font_width, variable height based on face->glyph->bitmap.rows
// glyph goes in vertical, possibly not filling the whole row space:
//    0.0       1.0
// 0.0 +------+--+
//     | 0x00 |  |
// 0.5 +------+--+
//     | 0x01    |
// 1.0 +------+--+
static void LoadFont() {
    need_reload_font = false;

    FT_Library ft;
    FT_Error err = FT_Init_FreeType(&ft);
    assert(err == 0);

    std::vector<std::pair<const char *, font_weight>> fonts = {
#ifdef STANDALONE
        {"../../../../../fonts/ttf/Inconsolata-Regular.ttf", font_weight::regular},
        {"../../../../../fonts/ttf/Inconsolata-Bold.ttf", font_weight::bold},
#else
        {"/data/storage/el2/base/haps/entry/files/Inconsolata-Regular.ttf", font_weight::regular},
        {"/data/storage/el2/base/haps/entry/files/Inconsolata-Bold.ttf", font_weight::bold},
#endif
    };

    // save glyph for all characters of all weights
    // only one channel
    std::vector<uint8_t> bitmap;
    int row_stride = max_font_width;
    int bitmap_height = 0;

    for (auto pair : fonts) {
        const char *font = pair.first;
        font_weight weight = pair.second;

        FT_Face face;
        err = FT_New_Face(ft, font, 0, &face);
        assert(err == 0);
        FT_Set_Pixel_Sizes(face, 0, font_height);
        // Note: in 26.6 fractional pixel format
        OH_LOG_INFO(LOG_APP,
                    "Ascender: %{public}d Descender: %{public}d Height: %{public}d XMin: %{public}ld XMax: %{public}ld "
                    "YMin: %{public}ld YMax: %{public}ld XScale: %{public}ld YScale: %{public}ld",
                    face->ascender, face->descender, face->height, face->bbox.xMin, face->bbox.xMax, face->bbox.yMin,
                    face->bbox.yMax, face->size->metrics.x_scale, face->size->metrics.y_scale);

        for (uint32_t c : codepoints_to_load) {
            // load character glyph
            assert(FT_Load_Char(face, c, FT_LOAD_RENDER) == 0);

            OH_LOG_INFO(LOG_APP,
                        "Weight: %{public}d Char: %{public}c(%{public}d) Glyph: %{public}d %{public}d Left: "
                        "%{public}d "
                        "Top: %{public}d "
                        "Advance: %{public}ld",
                        weight, c, c, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap_left,
                        face->glyph->bitmap_top, face->glyph->advance.x);

            // copy to bitmap
            int old_bitmap_height = bitmap_height;
            int new_bitmap_height = bitmap_height + face->glyph->bitmap.rows;
            bitmap.resize(row_stride * new_bitmap_height);
            bitmap_height = new_bitmap_height;

            assert(face->glyph->bitmap.width <= row_stride);
            for (int i = 0; i < face->glyph->bitmap.rows; i++) {
                for (int j = 0; j < face->glyph->bitmap.width; j++) {
                    // compute offset in the large texture
                    int off = old_bitmap_height * row_stride;
                    bitmap[i * row_stride + j + off] = face->glyph->bitmap.buffer[i * face->glyph->bitmap.width + j];
                }
            }

            // compute location within the texture
            // first pass: store pixels
            character character = {
                .left = 0,
                .right = (float)face->glyph->bitmap.width - 1,
                .top = (float)old_bitmap_height,
                .bottom = (float)new_bitmap_height - 1,
                .xoff = face->glyph->bitmap_left,
                .yoff = (int)(baseline_height + face->glyph->bitmap_top - face->glyph->bitmap.rows),
                .width = (int)face->glyph->bitmap.width,
                .height = (int)face->glyph->bitmap.rows,
            };
            characters[{c, weight}] = character;
        }


        FT_Done_Face(face);
    }

    FT_Done_FreeType(ft);

    // now bitmap contains all glyphs
    // second pass: convert pixels to uv coordinates
    for (auto &pair : characters) {
        // https://stackoverflow.com/questions/35454432/finding-image-pixel-coordinates-integers-from-uv-values-floats-of-obj-file
        pair.second.left /= row_stride - 1;
        pair.second.right /= row_stride - 1;
        pair.second.top = (pair.second.top + 0.5) / bitmap_height;
        pair.second.bottom = (pair.second.bottom + 0.5) / bitmap_height;
    }

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // generate texture
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, row_stride, bitmap_height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static GLuint program_id;
static GLuint vertex_array;
// vec4 vertex
static GLuint vertex_buffer;
// vec3 textColor
static GLuint text_color_buffer;
// vec3 backGroundColor
static GLuint background_color_buffer;

static void Draw() {
    // blink every 0.5s
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    // clear buffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update surface size
    pthread_mutex_lock(&lock);
    int aligned_width = width / font_width * font_width;
    int aligned_height = height / font_height * font_height;
    glUniform2f(surface_location, aligned_width, aligned_height);
    glViewport(0, height - aligned_height, aligned_width, aligned_height);

    // set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // bind our vertex array
    glBindVertexArray(vertex_array);

    int max_lines = height / font_height;
    // vec4 vertex
    static std::vector<GLfloat> vertex_pass0_data;
    static std::vector<GLfloat> vertex_pass1_data;
    // vec3 textColor
    static std::vector<GLfloat> text_color_data;
    // vec3 backgroundColor
    static std::vector<GLfloat> background_color_data;

    vertex_pass0_data.clear();
    vertex_pass0_data.reserve(term_row * term_col * 24);
    vertex_pass1_data.clear();
    vertex_pass1_data.reserve(term_row * term_col * 24);
    text_color_data.clear();
    text_color_data.reserve(term_row * term_col * 18);
    background_color_data.clear();
    background_color_data.reserve(term_row * term_col * 18);

    // ensure at least one line shown, for very large scroll_offset
    int scroll_rows = scroll_offset / font_height;
    if ((int)history.size() + max_lines - 1 - scroll_rows < 0) {
        scroll_offset = ((int)history.size() + max_lines - 1) * font_height;
        scroll_rows = scroll_offset / font_height;
    }

    for (int i = 0; i < max_lines; i++) {
        // (aligned_height - font_height) is terminal[0] when scroll_offset is zero
        float x = 0.0;
        float y = aligned_height - (i + 1) * font_height;
        int i_row = i - scroll_rows;
        std::vector<term_char> ch;
        if (i_row >= 0 && i_row < term_row) {
            ch = terminal[i_row];
        } else if (i_row < 0 && (int)history.size() + i_row >= 0) {
            ch = history[history.size() + i_row];
        } else {
            continue;
        }

        int cur_col = 0;
        for (auto c : ch) {
            uint32_t codepoint = c.ch;
            auto key = std::pair<uint32_t, enum font_weight>(c.ch, c.style.weight);
            auto it = characters.find(key);
            if (it == characters.end()) {
                // reload font to locate it
                OH_LOG_WARN(LOG_APP, "Missing character: %{public}d of weight %{public}d", c.ch, c.style.weight);
                need_reload_font = true;
                codepoints_to_load.insert(c.ch);

                // we don't have the character, fallback to space
                it = characters.find(std::pair<uint32_t, enum font_weight>(' ', c.style.weight));
                assert(it != characters.end());
            }

            character ch = it->second;
            float xpos = x;
            float ypos = y;
            float w = font_width;
            float h = font_height;

            // 1-2
            // | |
            // 3-4
            // (xpos    , ypos + h): 1
            // (xpos + w, ypos + h): 2
            // (xpos    , ypos    ): 3
            // (xpos + w, ypos    ): 4

            // pass 0: draw background
            GLfloat g_vertex_pass0_data[24] = {// first triangle: 1->3->4
                                               xpos, ypos + h, 0.0, 0.0, xpos, ypos, 0.0, 0.0, xpos + w, ypos, 0.0, 0.0,
                                               // second triangle: 1->4->2
                                               xpos, ypos + h, 0.0, 0.0, xpos + w, ypos, 0.0, 0.0, xpos + w, ypos + h,
                                               0.0, 0.0};
            vertex_pass0_data.insert(vertex_pass0_data.end(), &g_vertex_pass0_data[0], &g_vertex_pass0_data[24]);

            // pass 1: draw text
            xpos = x + ch.xoff;
            ypos = y + ch.yoff;
            w = ch.width;
            h = ch.height;
            GLfloat g_vertex_pass1_data[24] = {// first triangle: 1->3->4
                                               xpos, ypos + h, ch.left, ch.top, xpos, ypos, ch.left, ch.bottom,
                                               xpos + w, ypos, ch.right, ch.bottom,
                                               // second triangle: 1->4->2
                                               xpos, ypos + h, ch.left, ch.top, xpos + w, ypos, ch.right, ch.bottom,
                                               xpos + w, ypos + h, ch.right, ch.top};
            vertex_pass1_data.insert(vertex_pass1_data.end(), &g_vertex_pass1_data[0], &g_vertex_pass1_data[24]);

            GLfloat g_text_color_buffer_data[18];
            GLfloat g_background_color_buffer_data[18];

            if (i_row == row && cur_col == col && show_cursor) {
                // cursor
                for (int i = 0; i < 6; i++) {
                    g_text_color_buffer_data[i * 3 + 0] = 1.0 - c.style.fg_red;
                    g_text_color_buffer_data[i * 3 + 1] = 1.0 - c.style.fg_green;
                    g_text_color_buffer_data[i * 3 + 2] = 1.0 - c.style.fg_blue;
                    g_background_color_buffer_data[i * 3 + 0] = 1.0 - c.style.bg_red;
                    g_background_color_buffer_data[i * 3 + 1] = 1.0 - c.style.bg_green;
                    g_background_color_buffer_data[i * 3 + 2] = 1.0 - c.style.bg_blue;
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    g_text_color_buffer_data[i * 3 + 0] = c.style.fg_red;
                    g_text_color_buffer_data[i * 3 + 1] = c.style.fg_green;
                    g_text_color_buffer_data[i * 3 + 2] = c.style.fg_blue;
                    g_background_color_buffer_data[i * 3 + 0] = c.style.bg_red;
                    g_background_color_buffer_data[i * 3 + 1] = c.style.bg_green;
                    g_background_color_buffer_data[i * 3 + 2] = c.style.bg_blue;
                }
            }

            if (reverse_video) {
                // invert all colors
                for (int i = 0; i < 18; i++) {
                    g_text_color_buffer_data[i] = 1.0 - g_text_color_buffer_data[i];
                    g_background_color_buffer_data[i] = 1.0 - g_background_color_buffer_data[i];
                }
            }

            if (c.style.blink && current_msec % 1000 > 500) {
                // for every 1s, in 0.5s, text color equals to back ground color
                for (int i = 0; i < 18; i++) {
                    g_text_color_buffer_data[i] = g_background_color_buffer_data[i];
                }
            }

            text_color_data.insert(text_color_data.end(), &g_text_color_buffer_data[0], &g_text_color_buffer_data[18]);
            background_color_data.insert(background_color_data.end(), &g_background_color_buffer_data[0],
                                         &g_background_color_buffer_data[18]);

            x += font_width;
            cur_col++;
        }
    }
    pthread_mutex_unlock(&lock);

    // draw in two pass
    glBindBuffer(GL_ARRAY_BUFFER, text_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * text_color_data.size(), text_color_data.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, background_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * background_color_data.size(), background_color_data.data(),
                 GL_STREAM_DRAW);

    // first pass
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_pass0_data.size(), vertex_pass0_data.data(), GL_STREAM_DRAW);
    glUniform1i(render_pass_location, 0);
    glDrawArrays(GL_TRIANGLES, 0, vertex_pass0_data.size() / 4);

    // second pass
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_pass1_data.size(), vertex_pass1_data.data(), GL_STREAM_DRAW);
    glUniform1i(render_pass_location, 1);
    glDrawArrays(GL_TRIANGLES, 0, vertex_pass1_data.size() / 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
    glFinish();
    AfterDraw();
}


static void *RenderWorker(void *) {
    pthread_setname_np(pthread_self(), "render worker");

    BeforeDraw();

    // build vertex and fragment shader
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    char const *vertex_source = "#version 320 es\n"
                                "\n"
                                "in vec4 vertex;\n"
                                "in vec3 textColor;\n"
                                "in vec3 backgroundColor;\n"
                                "out vec2 texCoords;\n"
                                "out vec3 fragTextColor;\n"
                                "out vec3 fragBackgroundColor;\n"
                                "uniform vec2 surface;\n"
                                "void main() {\n"
                                "  gl_Position.x = vertex.x / surface.x * 2.0f - 1.0f;\n"
                                "  gl_Position.y = vertex.y / surface.y * 2.0f - 1.0f;\n"
                                "  gl_Position.z = 0.0;\n"
                                "  gl_Position.w = 1.0;\n"
                                "  texCoords = vertex.zw;\n"
                                "  fragTextColor = textColor;\n"
                                "  fragBackgroundColor = backgroundColor;\n"
                                "}";
    glShaderSource(vertex_shader_id, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader_id);

    int info_log_length;
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> vertex_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &vertex_shader_error_message[0]);
        OH_LOG_ERROR(LOG_APP, "Failed to build vertex shader: %{public}s", &vertex_shader_error_message[0]);
    }

    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    char const *fragment_source = "#version 320 es\n"
                                  "\n"
                                  "precision lowp float;\n"
                                  "in vec2 texCoords;\n"
                                  "in vec3 fragTextColor;\n"
                                  "in vec3 fragBackgroundColor;\n"
                                  "out vec4 color;\n"
                                  "uniform sampler2D text;\n"
                                  "uniform int renderPass;\n"
                                  "void main() {\n"
                                  "  if (renderPass == 0) {\n"
                                  "    color = vec4(fragBackgroundColor, 1.0);\n"
                                  "  } else {\n"
                                  "    float alpha = texture(text, texCoords).r;\n"
                                  "    color = vec4(fragTextColor, 1.0) * alpha;\n"
                                  "  }\n"
                                  "}";
    // blending is done by opengl (GL_ONE + GL_ONE_MINUS_SRC_ALPHA):
    // final = src * 1 + dest * (1 - src.a)
    // first pass: src = (fragBackgroundColor, 1.0), dest = (1.0, 1.0, 1.0, 1.0), final = (fragBackgroundColor, 1.0)
    // second pass: src = (fragTextColor * alpha, alpha), dest = (fragBackgroundColor, 1.0), final = (fragTextColor *
    // alpha + fragBackgroundColor * (1 - alpha), 1.0)
    glShaderSource(fragment_shader_id, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader_id);

    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> fragment_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &fragment_shader_error_message[0]);
        OH_LOG_ERROR(LOG_APP, "Failed to build fragment shader: %{public}s", &fragment_shader_error_message[0]);
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> link_program_error_message(info_log_length + 1);
        glGetProgramInfoLog(program_id, info_log_length, NULL, &link_program_error_message[0]);
        OH_LOG_ERROR(LOG_APP, "Failed to link program: %{public}s", &link_program_error_message[0]);
    }

    surface_location = glGetUniformLocation(program_id, "surface");
    assert(surface_location != -1);

    render_pass_location = glGetUniformLocation(program_id, "renderPass");
    assert(render_pass_location != -1);

    glUseProgram(program_id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // load font from ttf for the initial characters
    glGenTextures(1, &texture_id);
    // load common characters initially
    for (uint32_t i = 0; i < 128; i++) {
        codepoints_to_load.insert(i);
    }
    LoadFont();

    // create buffers for drawing
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    // vec4 vertex
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    GLint vertex_location = glGetAttribLocation(program_id, "vertex");
    assert(vertex_location != -1);
    glEnableVertexAttribArray(vertex_location);
    glVertexAttribPointer(vertex_location,   // attribute 0
                          4,                 // size
                          GL_FLOAT,          // type
                          GL_FALSE,          // normalized?
                          4 * sizeof(float), // stride
                          (void *)0          // array buffer offset
    );

    // vec3 textColor
    glGenBuffers(1, &text_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, text_color_buffer);
    GLint text_color_location = glGetAttribLocation(program_id, "textColor");
    assert(text_color_location != -1);
    glEnableVertexAttribArray(text_color_location);
    glVertexAttribPointer(text_color_location, // attribute 0
                          3,                   // size
                          GL_FLOAT,            // type
                          GL_FALSE,            // normalized?
                          3 * sizeof(float),   // stride
                          (void *)0            // array buffer offset
    );

    // vec3 backgroundColor
    glGenBuffers(1, &background_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, background_color_buffer);
    GLint background_color_location = glGetAttribLocation(program_id, "backgroundColor");
    assert(background_color_location != -1);
    glEnableVertexAttribArray(background_color_location);
    glVertexAttribPointer(background_color_location, // attribute 0
                          3,                         // size
                          GL_FLOAT,                  // type
                          GL_FALSE,                  // normalized?
                          3 * sizeof(float),         // stride
                          (void *)0                  // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t last_redraw_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    uint64_t last_fps_msec = last_redraw_msec;
    Draw();
    int fps = 0;
    std::vector<uint64_t> time;
    while (1) {
        gettimeofday(&tv, nullptr);
        uint64_t now_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        // even if we call faster than system settings (60Hz/120Hz), it does not get faster
        // 120 Hz, 8ms
        uint64_t deadline = last_redraw_msec + 8;
        if (now_msec < deadline) {
            usleep((deadline - now_msec) * 1000);
        }

        // redraw
        gettimeofday(&tv, nullptr);
        now_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        last_redraw_msec = now_msec;
        Draw();

        gettimeofday(&tv, nullptr);
        uint64_t msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        time.push_back(msec - now_msec);

        fps++;

        // report fps
        if (now_msec - last_fps_msec > 1000) {
            last_fps_msec = now_msec;
            uint64_t sum = 0;
            for (auto t : time) {
                sum += t;
            }
            OH_LOG_INFO(LOG_APP, "FPS: %{public}d, %{public}ld ms per draw", fps, sum / fps);
            fps = 0;
            time.clear();
        }

        if (need_reload_font) {
            LoadFont();
        }
    }
}


// on resize
void Resize(int new_width, int new_height) {
    pthread_mutex_lock(&lock);
    width = new_width;
    height = new_height;

    ResizeTo(height / font_height, width / font_width, false);
    pthread_mutex_unlock(&lock);

    struct winsize ws = {};
    ws.ws_col = term_col;
    ws.ws_row = term_row;
    ioctl(fd, TIOCSWINSZ, &ws);
}

// handle scrolling
void ScrollBy(double offset) {
    pthread_mutex_lock(&lock);
    // natural scrolling
    scroll_offset -= offset;
    if (scroll_offset < 0) {
        scroll_offset = 0.0;
    }

    pthread_mutex_unlock(&lock);
}

// start render thread
void StartRender() {
    pthread_t render_thread;
    pthread_create(&render_thread, NULL, RenderWorker, NULL);
}

#ifdef STANDALONE
#include <GLFW/glfw3.h>
static GLFWwindow *window;

void BeforeDraw() {
    glfwMakeContextCurrent(window);
}

void AfterDraw() {
    glfwSwapBuffers(window);
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ENTER) {
            uint8_t data[] = {0x0d};
            SendData(data, 1);
        } else if (key == GLFW_KEY_BACKSPACE) {
            uint8_t data[] = {0x7f};
            SendData(data, 1);
        } else if (key == GLFW_KEY_UP) {
            uint8_t data[] = {0x1b, 0x5b, 0x41};
            SendData(data, 3);
        } else if (key == GLFW_KEY_DOWN) {
            uint8_t data[] = {0x1b, 0x5b, 0x42};
            SendData(data, 3);
        } else if (key == GLFW_KEY_RIGHT) {
            uint8_t data[] = {0x1b, 0x5b, 0x43};
            SendData(data, 3);
        } else if (key == GLFW_KEY_LEFT) {
            uint8_t data[] = {0x1b, 0x5b, 0x44};
            SendData(data, 3);
        } else if (key == GLFW_KEY_TAB) {
            uint8_t data[] = {0x09};
            SendData(data, 1);
        }
    }
}

void CharCallback(GLFWwindow* window, uint32_t codepoint) {
    // TODO: encode utf8 properly
    SendData((uint8_t *)&codepoint, 1);
}

void ResizeWidth(int new_width) {
    int current_width, current_height;
    glfwGetWindowSize(window, &current_width, &current_height);
    glfwSetWindowSize(window, new_width, current_height);
}

void Copy(std::string base64) {
    // TODO
}

void RequestPaste() {
    // TODO
}

std::string GetPaste() {
    // TODO
    return "";
}

int main() {
    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    int window_width = 80 * font_width;
    int window_height = 30 * font_height;
    window =
        glfwCreateWindow(window_width, window_height, "Terminal", nullptr, nullptr);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);

    Start();
    StartRender();
    Resize(window_width, window_height);
    while (!glfwWindowShouldClose(window)) {
        // Check if any events have been activated (key pressed, mouse moved etc.)
        // and call corresponding response functions
        glfwPollEvents();
    }
}
#endif
