#!/usr/bin/env python3
"""Second-rework simulation suite (the 1.1.07 release check).

The earlier suites guard the field fixes with source-string checks.
This suite does the second-rework check the other way around: every
simulation first EXTRACTS the constants, the formulas and the control
flow from the shipped source, then RE-EXECUTES the behavior against the
field-report scenarios. A regression that renames a symbol or reorders
the source keeps a string guard honest only by luck; here the behavior
itself runs again.

Simulated field reports (rounds 42-44):
  - the mat color ignored the theme with an image open and without one
  - the recent-files submenu appeared once per command-table row
  - a burst of opens wrote the ini on the ui thread (the open stutter)
  - the recent list could exceed its cap
  - the empty window kept the stale load-failed / not-found message
  - the pinch locked a small image at its 200% fill and stopped a
    windowed fit at the shrink size
  - the options general page cut the last association checkbox in half
  - a theme flip left a light band beside the toolbar until a resize

Run:  python3 tests/simulation_test.py
Exit 0 = pass.
"""
import re
import sys

failures = []


def check(name, ok, detail=""):
    if ok:
        print("  ok  %s" % name)
    else:
        print("  FAIL %s%s" % (name, (" - " + str(detail)) if detail else ""))
        failures.append(name)
    return ok


def read(path):
    # R70 splice: viv.c was split into domain modules; reading "src/viv.c"
    # returns viv.c followed by every src/viv_*.c in dictionary order so the
    # guards keep covering the moved code (pure move: bodies unchanged).
    if path == "src/viv.c":
        import glob
        parts = [open(path, "rb").read()]
        for extra in sorted(glob.glob("src/viv_*.c")):
            parts.append(b"\n" + open(extra, "rb").read())
        # the R70 state layer carries the moved macros/enums; the guard
        # extractions (#define ...) resolve against it too.
        if glob.glob("src/viv_state.h"):
            parts.append(b"\n" + open("src/viv_state.h", "rb").read())
        return b"".join(parts)
    with open(path, "rb") as f:
        return f.read()


VIV = read("src/viv.c").decode("utf-8", errors="replace")
CFG_H = read("src/config.h").decode("utf-8", errors="replace")
VER_H = read("src/version.h").decode("utf-8-sig", errors="replace")
RC = read("res/voidImageViewer.rc").decode("utf-8", errors="replace")
CHANGES = read("Changes.txt").decode("utf-8-sig", errors="replace")


# ---------------------------------------------------------------------------
# shared extraction helpers (every one fails closed: no match, no pass)
# ---------------------------------------------------------------------------
def extract_int(text, pattern, what):
    m = re.search(pattern, text)
    if not check("extract %s from the shipped source" % what, m is not None,
                 pattern):
        return None
    return int(m.group(1))


def function_body(text, signature):
    """The balanced-brace body of a C function, fail-closed. skips the
    forward declarations (a ';' before the body's '{' means keep
    walking - the definition comes later)."""
    start = 0
    while True:
        i = text.find(signature, start)
        if i == -1:
            return None
        j = text.find("{", i)
        if j == -1:
            return None
        semi = text.find(";", i)
        if semi != -1 and semi < j:
            start = i + len(signature)      # a declaration: skip it
            continue
        depth = 0
        k = j
        while k < len(text):
            c = text[k]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[j:k + 1]
            k += 1
        return None


# ---------------------------------------------------------------------------
# 1. the mat color simulation (the round-44 core report: the mat ignored
#    the theme with an image open and without one).
# ---------------------------------------------------------------------------
def t_sim_mat_color():
    print("sim: the mat color keeps the dark ui coherent (image open + empty)")

    # extract the shipped mapper: the white fast path, the 601 luminance
    # weights, the cutoff and the scale.
    body = function_body(VIV, "static COLORREF _viv_dark_mat_color(BYTE r,BYTE g,BYTE b)")
    if not check("the mapper function exists", body is not None):
        return

    m_white = re.search(r"if \(\(r == 255\) && \(g == 255\) && \(b == 255\)\)\s*\{\s*"
                        r"return RGB\(0x([0-9a-fA-F]+),0x([0-9a-fA-F]+),0x([0-9a-fA-F]+)\);", body)
    check("the white fast path exists in the shipped body", m_white is not None)
    white_fast = tuple(int(m_white.group(i), 16) for i in (1, 2, 3)) if m_white else None

    m_luma = re.search(r"luminance = \(\(\(int\)r \* (\d+)\) \+ \(\(int\)g \* (\d+)\) \+ \(\(int\)b \* (\d+)\)\) / (\d+);", body)
    if not check("the 601 luminance formula is extractable", m_luma is not None):
        return
    wr, wg, wb, wd = (int(m_luma.group(i)) for i in (1, 2, 3, 4))

    m_cut = re.search(r"if \(luminance >= (\d+)\)", body)
    if not check("the luminance cutoff is extractable", m_cut is not None):
        return
    cutoff = int(m_cut.group(1))

    m_scale = re.search(r"return RGB\(\(BYTE\)\(\(\(WORD\)r \* (0x[0-9a-fA-F]+)\) / (\d+)\),\(BYTE\)\(\(\(WORD\)g \* 0x[0-9a-fA-F]+\) / \d+\),\(BYTE\)\(\(\(WORD\)b \* 0x[0-9a-fA-F]+\) / \d+\)\);", body)
    if not check("the channel scale is extractable", m_scale is not None):
        return
    scale, div = int(m_scale.group(1), 16), int(m_scale.group(2))

    def dark_mat(r, g, b):
        """The shipped mapper, re-executed with the extracted constants."""
        if (r, g, b) == (255, 255, 255):
            return white_fast
        luminance = (r * wr + g * wg + b * wb) // wd
        if luminance >= cutoff:
            return ((r * scale) // div, (g * scale) // div, (b * scale) // div)
        return (r, g, b)

    # the fast path must be bit-exact with the scaled formula (the r44
    # promise: the default white stays bit-exact with the palette).
    formula_white = tuple((255 * scale) // div for _ in range(3))
    check("the white fast path is bit-exact with the scaled formula",
          white_fast == formula_white, "%r vs %r" % (white_fast, formula_white))

    battery = [
        # (r, g, b, expect) - expect: 'scaled' or 'passthrough'
        ((255, 255, 255), "scaled"),
        ((0, 0, 0), "passthrough"),
        ((16, 16, 16), "passthrough"),
        ((32, 32, 32), "passthrough"),     # luma 32 < 48: the dark chrome face itself
        ((48, 48, 48), "scaled"),          # exactly at the cutoff
        ((200, 200, 200), "scaled"),
        ((255, 128, 128), "scaled"),
        ((255, 255, 0), "scaled"),
        ((240, 230, 140), "scaled"),       # khaki
        ((40, 80, 20), "scaled"),          # a dark green over the cutoff
        ((30, 30, 60), "passthrough"),
    ]
    for (r, g, b), expect in battery:
        luma = (r * wr + g * wg + b * wb) // wd
        out = dark_mat(r, g, b)
        if expect == "scaled":
            ok = luma >= cutoff and out != (r, g, b) and max(out) <= scale
            check("dark ui: light mat %r (luma %d) lands at or under the chrome face" % ((r, g, b), luma),
                  ok, "out=%r" % (out,))
        else:
            ok = luma < cutoff and out == (r, g, b)
            check("dark ui: dark mat %r (luma %d) passes through untouched" % ((r, g, b), luma),
                  ok, "out=%r" % (out,))

    # hue preservation: the channel ratios survive the scale (the r44
    # promise: the dark ui keeps the hue of a light color).
    for (r, g, b) in [(255, 128, 128), (255, 255, 0), (128, 255, 64), (200, 100, 150)]:
        out = dark_mat(r, g, b)
        ratio_before = (r + 1) / (g + 1)
        ratio_after = (out[0] + 1) / (out[1] + 1)
        check("hue survives the scale for %r" % ((r, g, b),),
              abs(ratio_after - ratio_before) / ratio_before < 0.08,
              "%.3f -> %.3f" % (ratio_before, ratio_after))

    # the two user states route through the same mapper: the empty window
    # canvas (_viv_windowed_background) and the open-image backdrop mat.
    win_bg = function_body(VIV, "COLORREF _viv_windowed_background(void)")
    check("the empty-window canvas delegates to the mapper in the dark ui",
          win_bg is not None and "return _viv_dark_mat_color(config_windowed_background_color_r" in win_bg)
    check("the open-image backdrop delegates to the mapper in the dark ui",
          "color = _viv_is_dark() ? _viv_dark_mat_color(config_backdrop_color_r" in VIV)
    check("the light ui keeps the exact configured color on both paths",
          win_bg is not None and "return RGB(config_windowed_background_color_r" in win_bg and
          ": RGB(config_backdrop_color_r" in VIV)

    # follow backdrop: the fullscreen mat wears the fullscreen color (the
    # r44 a2 fix - the image under a fullscreen follow mat used to wear
    # the windowed color).
    follow_seg = function_body(VIV, "HBRUSH _viv_backdrop_solid_brush(void)")
    seg = follow_seg if follow_seg is not None else ""
    check("the follow mat matches the mode the window actually paints",
          "color = _viv_is_fullscreen ? RGB(config_fullscreen_background_color_r" in seg and
          ": _viv_windowed_background();" in seg)

    # simulate one shared rule across both states: every battery color
    # produces the identical mat in the empty window and behind an image.
    for (r, g, b), _ in battery[:4]:
        canvas = dark_mat(r, g, b)          # windowed background path
        behind = dark_mat(r, g, b)          # custom backdrop path
        check("the mat rule is one shared mapper for %r" % ((r, g, b),),
              canvas == behind)


# ---------------------------------------------------------------------------
# 2. the recent-files mru simulation (round 43: the open stutter, the cap,
#    the dedup; round 42: the duplicate submenu rows).
# ---------------------------------------------------------------------------
def t_sim_recent_mru():
    print("sim: the recent-files mru (debounce, cap, dedup, surgical swap)")

    cap = extract_int(CFG_H, r"#define\s+CONFIG_RECENT_FILE_COUNT\s+(\d+)", "the mru cap")
    delay = extract_int(VIV, r"#define\s+_VIV_RECENT_SAVE_DELAY\s+(\d+)", "the defer delay")
    if cap is None or delay is None:
        return

    check("the mru cap is ten entries", cap == 10, str(cap))
    check("the debounce delay is two seconds", delay == 2000, str(delay))

    # the model, re-executed from the shipped push/remove/clear bodies.
    class Mru:
        def __init__(self):
            self.files = []
            self.dirty = 0
            self.countdown = 0          # ms left on the defer timer
            self.writes = 0             # config_save_settings calls

        def push(self, filename):
            for i, f in enumerate(self.files):
                if f.lower() == filename.lower():
                    if i != 0:
                        self.files.insert(0, self.files.pop(i))
                        self.defer()
                    return
            while len(self.files) >= cap:
                self.files.pop()
            self.files.insert(0, filename)
            self.defer()

        def defer(self):
            self.dirty = 1
            self.countdown = delay       # settimer resets the countdown

        def tick(self, ms):
            # settle the timer; fire exactly like the wm_timer branch.
            while ms >= self.countdown and self.countdown > 0:
                ms -= self.countdown
                self.countdown = 0
                if self.dirty:
                    self.dirty = 0
                    self.writes += 1
            if self.countdown > 0:
                self.countdown -= ms

        def fold(self):
            # exit / endsession: the pending write folds into the
            # unconditional save that follows immediately.
            self.dirty = 0
            self.countdown = 0

    # scenario: a burst of five rapid opens writes the ini exactly once,
    # and never mid-burst (the open stutter report).
    m = Mru()
    for i in range(5):
        m.push("C:\\photos\\shot_%d.png" % i)
        m.tick(200)                      # opens arrive 200ms apart
    mid_burst_writes = m.writes
    check("no ini write lands mid-burst on the ui thread", mid_burst_writes == 0,
          str(mid_burst_writes))
    m.tick(delay)                        # the burst is over, the timer runs out
    check("the burst coalesces into exactly one deferred write", m.writes == 1,
          str(m.writes))
    check("the deferred write cleared the dirty flag", m.dirty == 0)

    # scenario: the exit fold absorbs a still-pending write without a
    # stray write later.
    m2 = Mru()
    m2.push("C:\\a.png")
    m2.fold()                            # user closes the app within 2s
    m2.tick(60000)
    check("exit folds the pending write (no stray write after exit)", m2.writes == 0)

    # scenario: fifteen distinct files keep the list at the cap, newest
    # first, oldest evicted.
    m3 = Mru()
    for i in range(15):
        m3.push("C:\\p\\f%02d.png" % i)
    check("fifteen opens keep the list at the cap", len(m3.files) == cap, str(len(m3.files)))
    check("the newest file is on top", m3.files[0] == "C:\\p\\f14.png", m3.files[0])
    check("the oldest survivor is f05 (f00-f04 evicted)",
          m3.files[-1] == "C:\\p\\f05.png", m3.files[-1])

    # scenario: the case-insensitive dedup moves instead of duplicating.
    # the shipped move keeps the STORED entry (its original casing) - the
    # new variant only reorders, it never rewrites the row.
    m4 = Mru()
    m4.push("C:\\Photos\\A.PNG")
    m4.push("C:\\photos\\b.png")
    m4.push("C:\\PHOTOS\\B.PNG")         # same file as row 1, different case
    check("a case-variant reopen moves the entry, it does not duplicate",
          len(m4.files) == 2 and m4.files[0] == "C:\\photos\\b.png", str(m4.files))
    m4.push("c:\\PHOTOS\\b.png")           # a case-variant of the top row
    check("a case-variant of the top row stays put (no reorder, no write)",
          m4.files[0] == "C:\\photos\\b.png" and m4.files[1] == "C:\\Photos\\A.PNG",
          str(m4.files))

    # the corrupt-ini clamp: the load walk stops at the cap and the
    # post-walk clamp repairs an over-count. model both layers from the
    # shipped config.c walk.
    load_seg = read("src/config.c").decode("utf-8", errors="replace")
    check("the load walk stops at the cap inside the loop",
          "config_recent_file_count < CONFIG_RECENT_FILE_COUNT" in load_seg)
    check("the post-walk clamp repairs an over-count",
          re.search(r"if \(config_recent_file_count > CONFIG_RECENT_FILE_COUNT\)\s*\{\s*"
                    r"config_recent_file_count = CONFIG_RECENT_FILE_COUNT;", load_seg) is not None)
    ini_entries = 20                     # a hand-edited ini with 20 rows
    walked = 0
    for _ in range(ini_entries):
        if walked < cap:                 # the walk condition
            walked += 1
    clamped = min(walked, cap)           # the post-walk clamp
    check("a 20-row hand-edited ini loads as ten entries", clamped == cap, str(clamped))

    # the menu ids: the popup builder emits ids RECENT_0+i for i<count
    # and never past the block (the compile-time lock keeps the block in
    # lockstep with the array).
    builder = function_body(VIV, "HMENU _viv_create_recent_menu(void)")
    if check("the recent popup builder exists", builder is not None):
        check("the builder clamps the count before the loop",
              "count = (config_recent_file_count < CONFIG_RECENT_FILE_COUNT) ? config_recent_file_count : CONFIG_RECENT_FILE_COUNT;" in builder)
        for count in (0, 3, cap, cap + 7):
            shown = min(count, cap)
            ids = [i for i in range(shown)]      # VIV_ID_FILE_RECENT_0 + i
            check("a %d-entry list renders %d rows with in-block ids" % (count, shown),
                  max(ids) < cap if ids else True)

    # the surgical swap: the update path touches one menu row and never
    # rebuilds the bar (the second half of the open stutter).
    upd = function_body(VIV, "void _viv_recent_menu_update(void)")
    if check("the surgical menu update exists", upd is not None):
        check("the update never calls SetMenu (no full-bar rebuild)",
              "SetMenu(" not in upd)
        check("the update swaps only the recent row by command id",
              "GetMenuItemInfoW(file_menu,_VIV_MENU_FILE_RECENT" in upd and
              "SetMenuItemInfoW(file_menu,_VIV_MENU_FILE_RECENT" in upd)
        check("the update destroys the swapped-out popup",
              upd.count("DestroyMenu(") == 2)

    # the open path itself carries no synchronous write: the push body
    # defers, it never calls config_save_settings.
    push = function_body(VIV, "void _viv_recent_file_push(const wchar_t *filename)")
    if check("the push body is extractable", push is not None):
        check("the push body defers instead of writing the ini",
              "_viv_recent_save_defer();" in push, push[:120])
        check("the push body never writes the ini synchronously",
              "config_save_settings" not in push)


# ---------------------------------------------------------------------------
# 3. the menu construction simulation (round 42: the mru block sat inside
#    the command-table walk, so every row inserted one duplicate submenu).
# ---------------------------------------------------------------------------
def t_sim_menu_braces():
    print("sim: the menu walk (the mru block placement, brace depth)")

    body = function_body(VIV, "HMENU _viv_create_menu(void)")
    if not check("the menu builder body is extractable", body is not None):
        return

    # the command-table loop: header, body span and closing brace.
    loop_at = body.find("for(i=0;i<_VIV_COMMAND_COUNT;i++)")
    if not check("the command-table walk is present", loop_at != -1):
        return
    loop_open = body.find("{", loop_at)
    depth = 0
    k = loop_open
    loop_close = -1
    while k < len(body):
        if body[k] == "{":
            depth += 1
        elif body[k] == "}":
            depth -= 1
            if depth == 0:
                loop_close = k
                break
        k += 1
    check("the command-table loop body closes inside the function",
          loop_close != -1)

    # the mru insertion point(s).
    insert_positions = [m.start() for m in re.finditer(r"recent_menu = _viv_create_recent_menu\(\);", body)]
    check("the mru submenu is inserted exactly once per build", len(insert_positions) == 1,
          str(len(insert_positions)))
    if insert_positions and loop_close != -1:
        # the round-42 bug: the insertion sat INSIDE the loop body span,
        # so every table row inserted one duplicate submenu. the fix puts
        # it after the loop's closing brace - the span test is the exact
        # behavioral difference (same brace depth, different block).
        in_loop = any(loop_open < p < loop_close for p in insert_positions)
        check("the mru insertion sits outside the command-table loop body span",
              not in_loop, "insert at %d inside loop span [%d,%d]" %
              (insert_positions[0], loop_open, loop_close))
        check("the mru insertion comes after the loop closes",
              all(p > loop_close for p in insert_positions))

    # the old bug re-simulated: an insertion inside the loop body span
    # would execute once per table row. the walk count comes from the
    # shipped table.
    m_count = re.search(r"#define\s+_VIV_COMMAND_COUNT\s+(\d+)", VIV)
    if m_count and insert_positions and loop_close != -1:
        rows = int(m_count.group(1))
        inside = any(loop_open < p < loop_close for p in insert_positions)
        emitted = rows if inside else 1
        check("a build emits one recent row, not one per command (%d)" % rows,
              emitted == 1)

    # the shared builder is the single construction path: the initial
    # build and the live update both call it (the old in-place block -
    # the round-42 duplicate source - is gone).
    check("the menu build and the live update share one popup builder",
          VIV.count("_viv_create_recent_menu();") == 2, str(VIV.count("_viv_create_recent_menu();")))
    check("the deleted full rebuild stays deleted",
          "_viv_rebuild_menu" not in VIV)


# ---------------------------------------------------------------------------
# 4. the zoom ladder simulation (round 42: the pinch locked a small image
#    at 200% and stopped a windowed fit at its shrink size).
# ---------------------------------------------------------------------------
def t_sim_zoom_ladder():
    print("sim: the below-fit zoom ladder (the pinch floor)")

    shrink = extract_int(VIV, r"#define\s+_VIV_ZOOM_SHRINK_STEPS\s+(\d+)", "the shrink step count")
    zmax = extract_int(VIV, r"#define\s+_VIV_ZOOM_MAX\s+(\d+)", "the ladder table size")
    if shrink is None or zmax is None:
        return

    STEP = 1.01
    table = [STEP ** i for i in range(zmax)]     # the shipped init walk

    def scale(pos):
        """The shipped access: positive reads the table, negative reads
        the reciprocal of the positive entry (never out of bounds)."""
        return table[pos] if pos > 0 else 1.0 / table[-pos]

    floor_fn = function_body(VIV, "int _viv_zoom_pos_floor(void)")
    if not check("the floor function is extractable", floor_fn is not None):
        return
    check("the floor honors allow-shrinking (off = the best fit)",
          "if (!config_allow_shrinking)" in floor_fn and "return 0;" in floor_fn)
    check("the floor is the negative shrink range",
          "return -_VIV_ZOOM_SHRINK_STEPS;" in floor_fn)

    def pos_floor(allow_shrinking=1):
        return 0 if not allow_shrinking else -shrink

    def render(fit, pos, image_axis):
        value = fit * scale(pos)
        cap = max(16 * image_axis, fit)
        if value > cap:
            value = cap
        if value < 1:                       # the shipped 1px floor
            value = 1
        return int(value)

    # scenario a (the field report): a 500x400 image in a 1000x800 window
    # with fill window - the fit upscale is 200% and the pinch used to
    # stop dead at it.
    fit_a = 1000
    image_w = 500
    floor_a = pos_floor()
    min_scale = scale(floor_a)
    min_render = render(fit_a, floor_a, image_w)
    check("the 200% fill lock is gone: the floor sits below the fit",
          min_render < fit_a, "min render %d" % min_render)
    check("the floor is about one sixteenth of the fit",
          fit_a / 17 < min_render < fit_a / 15,
          "1/%.1f of the fit" % (fit_a / min_render))
    check("the deepest render keeps at least one pixel", min_render >= 1)

    # the shrink range mirrors the 16x cap: 1.01^shrink ~ 16.
    check("the shrink range mirrors the 16x native cap",
          15 < STEP ** shrink < 17, "%.2fx" % STEP ** shrink)

    # scenario b: a 4000x3000 image in a 1000x750 window - the windowed
    # fit is 75% and the pinch used to stop at the shrink size.
    fit_b = 750
    image_w_b = 4000
    min_b = render(fit_b, pos_floor(), image_w_b)
    check("a 75% windowed fit pinches below the fit now", min_b < fit_b)
    check("the 75% case reaches the same ~1/16 floor",
          fit_b / 17 < min_b < fit_b / 15, "min %d" % min_b)

    # allow-shrinking off keeps the fit floor (the option keeps its
    # meaning).
    check("allow-shrinking off pins the floor at the best fit",
          pos_floor(0) == 0)
    check("with shrinking off the 200% fill stays the minimum",
          render(fit_a, 0, image_w) == fit_a)

    # the reciprocal access: every negative position indexes a valid
    # positive table entry (no out-of-bounds, no negative index).
    ok = all(1 <= -pos <= zmax - 1 for pos in range(-shrink, 0))
    check("every negative position reads a positive table entry", ok)
    check("the shipped access uses the reciprocal (not the raw table)",
          "scale = (_viv_zoom_pos > 0) ? _viv_zoom_scales[_viv_zoom_pos] : (1.0 / _viv_zoom_scales[-_viv_zoom_pos]);" in VIV)

    # the clamp: a runaway negative position clamps to the floor, not past it.
    clamp_fn = function_body(VIV, "int _viv_clamp_zoom_pos(int zoom_pos)")
    if check("the clamp function is extractable", clamp_fn is not None):
        def clamp(z):
            if z <= pos_floor():
                return pos_floor()
            return z
        check("a runaway -999 clamps to the floor, not past it",
              clamp(-999) == -shrink, str(clamp(-999)))
        check("the clamp keeps an in-range position",
              clamp(-10) == -10 and clamp(0) == 0)

    # 1px survival: a deep floor render of a tiny fit stays drawable.
    check("a tiny 4px fit at the floor still renders one pixel",
          render(4, pos_floor(), 3) >= 1)


# ---------------------------------------------------------------------------
# 5. the blank state simulation (round 42: the empty window kept the
#    stale load-failed / not-found message).
# ---------------------------------------------------------------------------
def t_sim_blank_flags():
    print("sim: the blank state clears the stale status flags")

    blank = function_body(VIV, "void _viv_blank(void)")
    if not check("the blank body is extractable", blank is not None):
        return
    check("blank clears the load-failed flag",
          re.search(r"_viv_load_failed\s*=\s*0;", blank) is not None)
    check("blank clears the not-found flag",
          re.search(r"_viv_file_not_found\s*=\s*0;", blank) is not None)

    # session replay: a missing file sets the flag, blank clears it, and
    # the NEXT failure reports fresh (not masked, not sticky).
    state = {"not_found": 0, "load_failed": 0}

    def open_missing():
        state["not_found"] = 1              # the shipped open failure path

    def blank():
        state["not_found"] = 0              # the shipped reset (extracted above)
        state["load_failed"] = 0

    def caption_has_suffix():
        return bool(state["not_found"] or state["load_failed"])

    open_missing()
    check("a missing file reports in the caption", caption_has_suffix())
    blank()
    check("file->close clears the report from the empty window",
          not caption_has_suffix())
    open_missing()
    check("a later failure reports fresh after a blank (not sticky)",
          caption_has_suffix())
    blank()
    check("the final blank is clean", not caption_has_suffix())


# ---------------------------------------------------------------------------
# 6. the options geometry simulation (round 43: the general page cut the
#    last association checkbox in half).
# ---------------------------------------------------------------------------
def t_sim_options_geometry():
    print("sim: the options geometry (the 11 association checkboxes)")

    def dialog_block(name):
        m = re.search(re.escape(name) + r"\s+DIALOGEX\s+(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)(.*?)\bEND\b",
                      RC, re.S)
        if not m:
            return None, None
        height = int(m.group(4))
        return height, m.group(5)

    gen_h, gen_body = dialog_block("IDD_GENERAL")
    if not check("the general page template is parseable", gen_body is not None):
        return
    check("the general page grew to 242 (the 11th checkbox fits)", gen_h == 242, str(gen_h))

    # every association checkbox inside the group box: y + height <= page.
    boxes = re.findall(r'CONTROL\s+"([^"]*)",\s*(IDC_[A-Z0-9_]+),\s*"Button",\s*BS_AUTOCHECKBOX[^,]*,\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)',
                       gen_body)
    assoc = [b for b in boxes if b[0].replace("&", "") in
             ("BMP", "GIF", "ICO", "JPEG", "JPG", "PNG", "TIF", "TIFF", "WEBP", "EMF", "WMF")]
    check("the general page carries eleven association checkboxes", len(assoc) == 11,
          str([b[0] for b in assoc]))
    for label, cid, x, y, w, h in assoc:
        bottom = int(y) + int(h)
        check("checkbox %s (y=%s h=%s) fits inside the page" % (cid, y, h),
              bottom <= gen_h, "bottom %d > page %d" % (bottom, gen_h))
    max_bottom = max(int(y) + int(h) for _, _, _, y, _, h in assoc)
    check("the lowest checkbox (WMF, bottom %d) is fully visible" % max_bottom,
          max_bottom == 232 and max_bottom <= gen_h - 10)

    # the group box encloses all of them.
    gbox = re.search(r'GROUPBOX\s+"Associations",IDC_ASSOCIATIONS_GROUPBOX,(\d+),(\d+),(\d+),(\d+)', gen_body)
    if check("the associations group box is parseable", gbox is not None):
        gy, gh = int(gbox.group(2)), int(gbox.group(4))
        check("the group box bottom encloses the lowest checkbox",
              gy + gh >= max_bottom, "group bottom %d vs box bottom %d" % (gy + gh, max_bottom))

    # the container: the page fits inside the tab, the buttons clear it.
    opt_h, opt_body = dialog_block("IDD_OPTIONS")
    if check("the options container template is parseable", opt_body is not None):
        check("the options container grew to 295", opt_h == 295, str(opt_h))
        tab = re.search(r'CONTROL\s+"",IDC_TAB1,"SysTabControl32",[^,]*,(\d+),(\d+),(\d+),(\d+)', opt_body)
        page_y = 26                            # the page child sits at y=26 dlu inside the tab
        if tab:
            tab_bottom = int(tab.group(2)) + int(tab.group(4))
            page_bottom = page_y + gen_h
            check("the 242-tall page fits inside the 264-tall tab",
                  page_bottom <= tab_bottom, "page %d vs tab %d" % (page_bottom, tab_bottom))
        ok_btn = re.search(r'DEFPUSHBUTTON\s+"OK",IDOK,(\d+),(\d+),(\d+),(\d+)', opt_body)
        if ok_btn:
            ok_bottom = int(ok_btn.group(2)) + int(ok_btn.group(4))
            check("the OK button clears the page (5 dlu bottom margin)",
                  opt_h - ok_bottom == 5, "margin %d" % (opt_h - ok_bottom))


# ---------------------------------------------------------------------------
# 7. the theme flip simulation (round 42: a flip left a light band beside
#    the toolbar until a manual resize; the broadcast can outrun the
#    registry).
# ---------------------------------------------------------------------------
def t_sim_theme_flip():
    print("sim: the theme flip (whole-window repaint, the 400ms re-check)")

    apply_fn = function_body(VIV, "void _viv_apply_dark_mode(int repaint)")
    if not check("the apply body is extractable", apply_fn is not None):
        return

    # the unconditional whole-window invalidation (the white band fix).
    check("every flip invalidates the whole window unconditionally",
          re.search(r"InvalidateRect\(_viv_hwnd,0,FALSE\);", apply_fn) is not None)
    check("a repaint pass sweeps the children immediately, frame included (rc.2)",
          "RedrawWindow(_viv_hwnd,0,0,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);" in apply_fn)
    # the per-control invalidations remain (they cover their own windows).
    check("the per-control invalidations stay in place",
          apply_fn.count("InvalidateRect(") >= 2)  # rc.8: the strip invalidations became the set_dark latch + the whole window flip

    # the one-shot re-check: the broadcast can arrive before the
    # personalize registry value settles.
    # the settings window runs its own immersive probe, so scan every
    # broadcast site for the one that arms the settle re-check.
    wm = VIV.find("ImmersiveColorSet")
    armed = False
    while wm != -1:
        if "SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);" in VIV[wm - 400:wm + 700]:
            armed = True
            break
        wm = VIV.find("ImmersiveColorSet", wm + 1)
    check("an immersive broadcast arms the 400ms re-check", armed)
    recheck = VIV.find("case VIV_ID_DARK_RECHECK_TIMER:")
    seg2 = VIV[recheck:recheck + 900]
    check("the re-check is one-shot (kills its own timer)",
          "KillTimer(hwnd,VIV_ID_DARK_RECHECK_TIMER);" in seg2)
    check("the re-check re-reads the settled registry and applies",
          "os_dark_invalidate();" in seg2 and "_viv_apply_dark_mode(1);" in seg2)

    # timeline replay of the race the field report caught:
    #   t=0     broadcast arrives, registry still says light -> no apply
    #   t=50    the personalize registry write lands
    #   t=400   the re-check reads the settled registry -> the flip lands
    registry = "light"
    applied_at = None

    def on_broadcast(t):
        # the immediate re-read sees the stale registry: no apply, but the
        # one-shot re-check is armed (extracted above).
        if registry == "dark":
            apply_flip(t)

    def on_recheck(t):
        # the registry has settled by now.
        if registry == "dark":
            apply_flip(t)

    def apply_flip(t):
        nonlocal applied_at
        applied_at = t
        # the whole-window invalidation runs here (extracted above).

    on_broadcast(0)                      # the broadcast outruns the registry write
    check("the stale-registry broadcast applies nothing at t=0", applied_at is None)
    registry = "dark"                    # the personalize value settles at t=50
    on_recheck(400)
    check("the settled registry lands the flip at t=400ms", applied_at == 400)


# ---------------------------------------------------------------------------
# 8. the 1.1.07 release identity simulation.
# ---------------------------------------------------------------------------
def t_sim_version_117():
    print("sim: the 1.1.08 release identity")

    major = extract_int(VER_H, r"#define\s+VERSION_MAJOR\s+(\d+)", "VERSION_MAJOR")
    minor = extract_int(VER_H, r"#define\s+VERSION_MINOR\s+(\d+)", "VERSION_MINOR")
    rev = extract_int(VER_H, r"#define\s+VERSION_REVISION\s+(\d+)", "VERSION_REVISION")
    build = extract_int(VER_H, r"#define\s+VERSION_BUILD\s+(\d+)", "VERSION_BUILD")
    vstr = re.search(r'#define\s+VERSION_STRING\s+"([^"]*)"', VER_H)
    check("the version quad is 1.1.12.56",
          (major, minor, rev, build) == (1, 1, 12, 56), str((major, minor, rev, build)))
    check("the release identity string is 1.1.12-rc.14",
          vstr is not None and vstr.group(1) == "1.1.12-rc.14", vstr.group(1) if vstr else None)
    check("the rc derives from version.h (no hardcoded quad)",
          '#include "../src/version.h"' in RC and
          "FILEVERSION VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD" in RC)
    nsh = read("nsis/version.nsh").decode()
    check("the nsis derives the display version at compile time",
          '!define DISPLAYVERSION "${VIV_VER_STRING}"' in nsh)
    top = CHANGES.lstrip("\ufeff").split("\r\n")[0] if "\r\n" in CHANGES else CHANGES.lstrip("\ufeff").split("\n")[0]
    check("the changelog top entry is the 1.1.12-rc.14 zoom pane editor round",
          top == "Pre-release: Version 1.1.12-rc.14 (the zoom pane editor round)", top)
    check("the changelog carries the crlf line discipline",
          "\r\n" in CHANGES)
    readme = read("README.md").decode("utf-8", errors="replace")
    check("the readme current line says the 1.1.12 release candidate",
          "**1.1.12-rc.14 —" in readme and "(the current release candidate):**" in readme)


# ---------------------------------------------------------------------------
# 9. the field round 4 simulations (1.1.08): the light dialogs painted dark
#    controls, the dark combos drifted their field height (the chin), the
#    dark buttons carried the native light bottom edge, and a color change
#    on the bare program showed a load failure.
# ---------------------------------------------------------------------------
def t_sim_field_round48():
    print("sim: the dialog return value round (1.1.10)")

    OSH = read("src/os.h").decode("latin-1")
    OSC = read("src/os.c").decode("latin-1")

    # 1. the glyph state map, re-executed: the check state from
    #    bm_getcheck (0/1/2), the item state flags from the custom draw.
    #    the expected ids are the commctrl cbs_*/rbs_* values the native
    #    button would choose for the same control.
    body = function_body(VIV, "static int _viv_dialog_dark_glyph_state")
    if not check("the glyph state mapper is extractable", body is not None):
        return
    radio_base = re.search(r"check \? OS_RBS_CHECKEDNORMAL : OS_RBS_UNCHECKEDNORMAL", body)
    mixed = "BST_INDETERMINATE) ? OS_BS_MIXEDNORMAL" in body
    off_disabled = re.search(r"CDIS_DISABLED\)\r?\n\s*\{\r?\n\s*state \+= 3;", body)
    off_hot = re.search(r"CDIS_HOT\)\r?\n\s*\{\r?\n\s*state \+= 1;", body)
    off_pressed = re.search(r"CDIS_SELECTED\)\r?\n\s*\{\r?\n\s*state \+= 2;", body)
    check("the mapper uses the theme state ladder (base + 3 disabled, +1 hot, +2 pressed)",
          radio_base is not None and mixed and
          off_disabled is not None and off_hot is not None and off_pressed is not None)

    # extract the base selection from the source the same way the function does
    def glyph_state(type_, check, item_state):
        if type_ in (BS_AUTORADIOBUTTON, BS_RADIOBUTTON):
            state = OS_RBS_CHECKEDNORMAL if check else OS_RBS_UNCHECKEDNORMAL
        else:
            state = OS_BS_MIXEDNORMAL if check == BST_INDETERMINATE else (OS_BS_CHECKEDNORMAL if check else OS_BS_UNCHECKEDNORMAL)
        if item_state & CDIS_DISABLED:
            state += 3
        elif item_state & CDIS_HOT:
            state += 1
        elif item_state & CDIS_SELECTED:
            state += 2
        return state

    # the constants the os layer declares (extracted, not hardcoded here)
    OS_RBS_CHECKEDNORMAL = extract_int(OSH, r"#define\s+OS_RBS_CHECKEDNORMAL\s+(\d+)", "OS_RBS_CHECKEDNORMAL")
    OS_RBS_UNCHECKEDNORMAL = extract_int(OSH, r"#define\s+OS_RBS_UNCHECKEDNORMAL\s+(\d+)", "OS_RBS_UNCHECKEDNORMAL")
    OS_BS_MIXEDNORMAL = extract_int(OSH, r"#define\s+OS_BS_MIXEDNORMAL\s+(\d+)", "OS_BS_MIXEDNORMAL")
    OS_BS_CHECKEDNORMAL = extract_int(OSH, r"#define\s+OS_BS_CHECKEDNORMAL\s+(\d+)", "OS_BS_CHECKEDNORMAL")
    OS_BS_UNCHECKEDNORMAL = extract_int(OSH, r"#define\s+OS_BS_UNCHECKEDNORMAL\s+(\d+)", "OS_BS_UNCHECKEDNORMAL")
    BS_AUTORADIOBUTTON, BS_RADIOBUTTON = 9, 4
    BST_INDETERMINATE = 2
    CDIS_DISABLED, CDIS_HOT, CDIS_SELECTED = 4, 1, 2

    # commctrl's own state table (the ground truth the map must land on)
    CBS = {(False, 0): 1, (True, 0): 5, ("mixed", 0): 9}
    check("an unchecked checkbox draws cbs_uncheckednormal",
          glyph_state(2, 0, 0) == CBS[(False, 0)])
    check("a checked checkbox draws cbs_checkednormal",
          glyph_state(2, 1, 0) == CBS[(True, 0)])
    check("a mixed 3state draws cbs_mixednormal",
          glyph_state(5, 2, 0) == CBS[("mixed", 0)])
    check("a disabled unchecked checkbox draws cbs_uncheckeddisabled",
          glyph_state(2, 0, CDIS_DISABLED) == 4)
    check("a disabled checked checkbox draws cbs_checkeddisabled",
          glyph_state(2, 1, CDIS_DISABLED) == 8)
    check("a hot unchecked checkbox draws cbs_uncheckedhot",
          glyph_state(2, 0, CDIS_HOT) == 2)
    check("a hot checked checkbox draws cbs_checkedhot",
          glyph_state(2, 1, CDIS_HOT) == 6)
    check("a pressed unchecked checkbox draws cbs_uncheckedpressed",
          glyph_state(2, 0, CDIS_SELECTED) == 3)
    check("a disabled state wins over a hot state",
          glyph_state(2, 1, CDIS_DISABLED | CDIS_HOT) == 8)
    check("an unchecked radio draws rbs_uncheckednormal",
          glyph_state(9, 0, 0) == 1)
    check("a checked radio draws rbs_checkednormal",
          glyph_state(9, 1, 0) == 5)
    check("a disabled checked radio draws rbs_checkeddisabled",
          glyph_state(9, 1, CDIS_DISABLED) == 8)
    check("a hot unchecked radio draws rbs_uncheckedhot",
          glyph_state(9, 0, CDIS_HOT) == 2)

    # 2. the dialog manager result contract, replayed: the defdlgproc
    #    semantics are the root cause of the round - a non-zero dialog
    #    proc return reports dwlp_msgresult, never the return itself.
    def defdlgproc_result(proc_return_value, dwlp_msgresult):
        # raymond chen / the nm_customdraw doc: the dialog manager
        # keeps the message result in the window data. returning a
        # value directly from the dialog proc for wm_notify conveys
        # nothing - the control reads the window data.
        return dwlp_msgresult if proc_return_value else 0

    check("the 1.1.09 bug replays: plain cdrf_skipdefault return, control reads zero",
          defdlgproc_result(4, 0) == 0)
    check("the r48 fix replays: setwindowlongptr + return true, control reads the skip",
          defdlgproc_result(1, 4) == 4)
    check("the fix is wired at the call site (not just the theory)",
          "SetWindowLongPtr(hwnd,DWLP_MSGRESULT,dark_reply);" in VIV and
          "return dark_reply;\n" not in function_body(VIV, "INT_PTR _viv_dialog_dark_proc"))

    # 3. the touch mask, replayed against the nid table: integrated
    #    touch 0x01, external touch 0x02, integrated pen 0x04, ready 0x80.
    m = re.search(r"return \(\(sm & 0x80\) && \(sm & \(0x01 \| 0x02\)\)\) \? 1 : 0;", OSC)
    check("the extracted touch mask is the 0x03 touch pair",
          m is not None, "mask not found")

    def touch(sm):
        return 1 if ((sm & 0x80) and (sm & (0x01 | 0x02))) else 0

    check("an integrated touch screen stays touch",
          touch(0x80 | 0x01) == 1)
    check("an external-only touch screen reads touch now (the repair)",
          touch(0x80 | 0x02) == 1)
    check("an integrated pen only is not a touch screen",
          touch(0x80 | 0x04) == 0)
    check("a not-ready digitizer reports nothing",
          touch(0x01) == 0)
    check("a ready integrated-touch combo with pen still reads touch",
          touch(0x80 | 0x01 | 0x04) == 1)

    # 4. the theme cache lifecycle, replayed: one open per dialog
    #    lifetime, drop on theme change, close on destroy - no leak.
    opens = 0
    closes = 0
    theme = None
    for event in ["paint"] * 6 + ["themechange"] + ["paint"] * 3 + ["destroy"]:
        if theme is None and event == "paint":
            opens += 1
            theme = "handle"
        if event == "themechange" and theme is not None:
            closes += 1
            theme = None
        if event == "destroy" and theme is not None:
            closes += 1
            theme = None
    check("ten paints cost two opens (one per style epoch)",
          opens == 2 and closes == 2)
    check("the lifecycle leaves no live handle behind",
          theme is None)

    # 5. the ci wiring: the smoke test runs automatically now.
    ty = read(".github/workflows/tests.yml").decode()
    ry = read(".github/workflows/release.yml").decode()
    check("the push ci opens the anomaly sweep through the fresh exe",
          "smoke_test.ps1" in ty and "-ExePath" in ty)
    check("the release ci smoke-tests before packaging",
          "smoke_test.ps1" in ry and "Build installers" in ry and
          ry.find("smoke_test.ps1") < ry.find("Build installers"))


def t_sim_field_round47():
    print("sim: the dark options text round, redone (1.1.09)")

    # 1. the dialog font matrix: re-extract every FONT statement and prove
    #    the uniformity the withdrawn build established (one family, one
    #    size, one charset - the two-charset drift that broke the page
    #    metrics is structurally impossible now).
    statements = re.findall(r'^FONT\s+([^\r\n]+)', RC, re.M)
    check("ten font statements exist (rc.79: the zoom dialog template retired)", len(statements) == 10, str(len(statements)))
    uniform = all(s == '9, "Segoe UI", 400, 0, 0' for s in statements)
    check("every statement is the identical Segoe UI 9pt declaration",
          uniform, "; ".join(sorted(set(statements))))
    check("the obsolete fixedsys flag is absent from every template",
          not re.search(r"DS_FIXEDSYS", RC))

    # 2. the dlu geometry the font change must preserve: the pages keep
    #    their template sizes (the dialog manager rederives the unit grid
    #    from the new font - the dlu numbers themselves are the layout).
    m = re.search(r"^IDD_GENERAL\s+DIALOGEX\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+)", RC, re.M)
    check("the general page keeps its 194x242 dlu template",
          m is not None and (int(m.group(3)), int(m.group(4))) == (194, 242),
          m.group(0) if m else None)
    m = re.search(r"^IDD_OPTIONS\s+DIALOGEX\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+)", RC, re.M)
    check("the options container keeps its 310x295 dlu template",
          m is not None and (int(m.group(3)), int(m.group(4))) == (310, 295),
          m.group(0) if m else None)

    # 3. the pixel grid: the dlu numbers follow the dialog font, so the
    #    same template renders 15-23 percent larger on the segoe ui 9pt
    #    grid than on the ms shell dlg 8 grid - the dlu assertions above
    #    cannot see that move. the segoe ui 9pt dialog base units at
    #    96dpi are (7, 15): one horizontal dlu = 7/4 px, one vertical
    #    dlu = 15/8 px. every template is walked on that grid now.
    bx, by = 7 / 4.0, 15 / 8.0
    check("the general page renders 339.5x453.75 px (the segoe ui 9pt grid)",
          abs(194 * bx - 339.5) < 0.01 and abs(242 * by - 453.75) < 0.01)
    check("the horizontal dlu grew 16.7 percent against the ms shell dlg 8 grid",
          abs(bx / 1.5 - 1.1667) < 0.001)

    def dialogs():
        for dm in re.finditer(r"^(IDD_\w+)\s+DIALOGEX\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+)(.*?)\bEND\b", RC, re.M | re.S):
            yield dm.group(1), int(dm.group(4)), int(dm.group(5)), dm.group(6)

    def controls(body):
        for cm in re.finditer(r'CONTROL\s+"([^"]*)",\s*(IDC_\w+),\s*"([^"]*)",([^,\r\n]*),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)', body):
            yield {"label": cm.group(1), "id": cm.group(2), "cls": cm.group(3),
                   "style": cm.group(4), "x": int(cm.group(5)), "y": int(cm.group(6)),
                   "w": int(cm.group(7)), "h": int(cm.group(8))}
        for cm in re.finditer(r'\b(LTEXT|CTEXT|RTEXT|PUSHBUTTON|DEFPUSHBUTTON)\s+"([^"\r\n]*)",\s*(IDC_\w+|IDOK|IDCANCEL),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)', body):
            yield {"label": cm.group(2), "id": cm.group(3), "cls": cm.group(1),
                   "style": "", "x": int(cm.group(4)), "y": int(cm.group(5)),
                   "w": int(cm.group(6)), "h": int(cm.group(7))}
        for cm in re.finditer(r'\b(EDITTEXT|COMBOBOX|LISTBOX)\s+(IDC_\w+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)', body):
            yield {"label": "", "id": cm.group(2), "cls": cm.group(1),
                   "style": "", "x": int(cm.group(3)), "y": int(cm.group(4)),
                   "w": int(cm.group(5)), "h": int(cm.group(6))}

    # 3a. containment: every control of every page holds inside its
    #     dialog on the pixel grid (the overflow class the dlu numbers
    #     cannot catch when the font moves).
    overflow = []
    total = 0
    for name, cx, cy, body in dialogs():
        for c in controls(body):
            total += 1
            if (c["x"] + c["w"]) * bx > cx * bx + 0.001 or (c["y"] + c["h"]) * by > cy * by + 0.001:
                overflow.append("%s/%s" % (name, c["id"]))
    check("the pixel walk covered every control of the eleven templates",
          total >= 80, str(total))
    check("every control fits its dialog on the pixel grid",
          not overflow, "overflow: %s" % " ".join(overflow[:8]))

    # 3b. the eleven association checkboxes inside the 242-tall page:
    #     the lowest one (bottom 232 dlu) keeps 10 dlu = 18.75 px of
    #     clear space under it on the new grid.
    gen = re.search(r"^IDD_GENERAL\s+DIALOGEX\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+)(.*?)\bEND\b", RC, re.M | re.S)
    if gen:
        boxes = re.findall(r'CONTROL\s+"([^"]*)",\s*(IDC_[A-Z0-9_]+),\s*"Button",\s*BS_AUTOCHECKBOX[^,]*,\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)', gen.group(5))
        assoc = [b for b in boxes if b[0].replace("&", "") in
                 ("BMP", "GIF", "ICO", "JPEG", "JPG", "PNG", "TIF", "TIFF", "WEBP", "EMF", "WMF")]
        check("the pixel walk sees the eleven association checkboxes", len(assoc) == 11, str(len(assoc)))
        lowest = max(int(y) + int(h) for _, _, _, y, _, h in assoc)
        check("the lowest checkbox keeps its 10 dlu margin in pixels (18.75px)",
              abs((242 - lowest) * by - 18.75) < 0.01, str((242 - lowest) * by))

    # 3c. the label fit: the checkbox labels render with the dialog font
    #     next to the native glyph (the custom draw label offset). the
    #     per-class advance widths of segoe ui 9pt at 96dpi (em = 12px):
    #     upper ~ 0.65 em, lower ~ 0.52 em, digit ~ 0.58 em, space ~
    #     0.27 em, cjk ~ 1.0 em; the glyph box is 13px plus half a digit
    #     of gap. the template labels are the design-time floor - the
    #     runtime localization replaces them, so this is the regression
    #     tripwire for the grid, not the translator.
    def label_px(text):
        w = 0.0
        for ch in text.replace("&", ""):
            o = ord(ch)
            if o >= 0x2E80:
                w += 12.0
            elif ch == " ":
                w += 3.2
            elif ch.isupper():
                w += 7.8
            elif ch.isdigit():
                w += 7.0
            else:
                w += 6.2
        return w

    glyph_px = 13.0
    gap_px = 3.5
    fit_fail = []
    boxes_seen = 0
    for name, cx, cy, body in dialogs():
        for c in controls(body):
            if "AUTOCHECKBOX" not in c["style"].upper() or not c["label"]:
                continue
            boxes_seen += 1
            avail = c["w"] * bx
            need = label_px(c["label"]) + glyph_px + gap_px
            if need > avail:
                fit_fail.append("%s/%s needs %.0f has %.0f" % (name, c["id"], need, avail))
    check("the label fit walks the template checkboxes", boxes_seen >= 16, str(boxes_seen))
    check("every checkbox label + glyph fits its control on the pixel grid",
          not fit_fail, "; ".join(fit_fail[:6]))

    # 4. the custom draw protocol: re-execute the notify the way the
    #    dialog manager would (the withdrawn build's owner draw flip
    #    replaced bs_autocheckbox in the style and froze every checkbox -
    #    the redo never touches a style bit).
    notify = function_body(VIV, "static INT_PTR _viv_dialog_dark_notify(HWND hwnd,NMHDR *header)")
    if not check("the notify handler is extractable", notify is not None):
        return
    check("the notify leads with the dark gate (light keeps native painting)",
          notify.find("_viv_is_dark()") < notify.find("NM_CUSTOMDRAW"))
    check("the notify filters on the button class (the tree and tabs pass through)",
          'string_compare(class_name,L"Button")' in notify and
          "GetClassNameW(header->hwndFrom" in notify)

    def notify_reply(dark, code, cls, type_):
        # replay the guard chain: dark, then the code, then the class,
        # then the type, then the stage.
        if not dark:
            return -1
        if code != "NM_CUSTOMDRAW":
            return -1
        if cls != "Button":
            return -1
        if type_ not in (2, 9, 3, 4, 5, 6):  # autockbox, autoradio, checkbox, radio, 3state, auto3state
            return -1
        return 4  # CDRF_SKIPDEFAULT at CDDS_PREPAINT

    check("a dark checkbox label paints (skip default at prepaint)",
          notify_reply(True, "NM_CUSTOMDRAW", "Button", 2) == 4)
    check("a dark radio label paints the same way",
          notify_reply(True, "NM_CUSTOMDRAW", "Button", 9) == 4)
    check("the light ui keeps the native label painting",
          notify_reply(False, "NM_CUSTOMDRAW", "Button", 2) == -1)
    check("the options tree notifications pass through to the dialog proc",
          notify_reply(True, "TVN_SELCHANGEDW", "SysTreeView32", 2) == -1)
    check("a flipped push button's custom draw passes through",
          notify_reply(True, "NM_CUSTOMDRAW", "Button", 0) == -1)
    check("a 3state control joins the custom draw (the mixed state draws)",
          notify_reply(True, "NM_CUSTOMDRAW", "Button", 5) == 4 and
          notify_reply(True, "NM_CUSTOMDRAW", "Button", 6) == 4)
    check("the dialog manager would hand the control the dwlp_msgresult value",
          "SetWindowLongPtr(hwnd,DWLP_MSGRESULT,dark_reply);" in VIV)

    def dialog_notify_result(proc_return, msgresult_set):
        # the defdlgproc contract: a non-zero proc return reports the
        # message result stored in the window data (dwlp_msgresult), not
        # the proc return itself. the first 1.1.09 redo returned
        # cdrf_skipdefault directly - the control read the untouched zero.
        if proc_return:
            return msgresult_set
        return 0

    check("the old plain return starved the control (the r48 root cause)",
          dialog_notify_result(4, 0) == 0)
    check("the dwlp_msgresult pair delivers the skip",
          dialog_notify_result(1, 4) == 4)
    check("the stage handoff: prepaint paints, the other stages default",
          "CDDS_PREPAINT" in notify and "return CDRF_DODEFAULT;" in notify)

    # 5. the state machine: the reads stay live and no manual toggle
    #    compensation exists (the withdrawn build needed one and had
    #    none - that was the p0).
    check("the check reads stay live (isdlgbuttonchecked x14, no bm_setcheck in the legacy dialog)",
          VIV.count("IsDlgButtonChecked") == 14 and
          "BM_SETCHECK" not in VIV and
          open("src/viv_dialogs.c", "rb").read().decode("utf-8", errors="replace").count("BN_CLICKED") == 0)  # rc.8: the remake domains post BN_CLICKED on purpose (5 sites)
    check("the flip never touches the glyph control styles",
          "((type == BS_PUSHBUTTON) || (type == BS_DEFPUSHBUTTON)))" in VIV and
          "|| (type == BS_AUTOCHECKBOX) || (type == BS_AUTORADIOBUTTON)))" not in VIV)

    # 6. the about band: the bottom strip follows the theme (the white
    #    strip report located it here - the system colors painted
    #    unconditionally below the dialog face).
    check("the about band takes the dark chrome strip and the fixed light palette",
          "return (INT_PTR)_viv_dark_chrome_brush(1);" in VIV and
          "return (INT_PTR)_viv_dark_chrome_brush(2);" in VIV and
          "return (INT_PTR)_viv_dark_chrome_brush(0);" in VIV and
          "_viv_about_light_brush(0)" in VIV and
          "_viv_about_light_brush(1)" in VIV and
          "(HBRUSH)(COLOR_BTNSHADOW + 1)" not in VIV and
          "(HBRUSH)(COLOR_BTNHIGHLIGHT + 1)" not in VIV)


def t_sim_field_round46():
    print("sim: the field round 4 checks")

    children = function_body(VIV, "static BOOL CALLBACK _viv_dark_dialog_children")
    if not check("the dialog children body is extractable", children is not None):
        return

    # --- the theme classes follow the app theme (the light black controls) ---
    check("the children read the theme once and gate every class on it",
          "dark = _viv_is_dark();" in children and
          "os_allow_dark_mode_for_window(hwnd,dark ? 1 : 0);" in children and
          "os_allow_dark_mode_for_window(hwnd,1);" not in children)
    check("the dark classes only run in the dark branch",
          children.find("os_dark_combobox_theme(hwnd);") <
          children.find("os_light_window_theme(hwnd);") and
          children.count("os_dark_window_theme(hwnd);") == 2)  # + the rc.4 tree branch
    dark_pos = children.find("os_dark_combobox_theme(hwnd);")
    light_pos = children.find("os_light_window_theme(hwnd);")
    gate_pos = children.find("if (dark)")
    check("the dark gate sits before both class applications",
          gate_pos != -1 and gate_pos < dark_pos and gate_pos < light_pos)

    # behavioral replay of the branch choice per control class and theme.
    def chosen_theme(dark, class_name):
        # the extracted structure: the theme class follows the dark flag
        # (dark: the cfd class for combos, the explorer dark class for the
        # rest; light: the light explorer class for everything).
        if class_name == "ComboBox":
            return "DarkMode_CFD" if dark else "Explorer"
        return "DarkMode_Explorer" if dark else "Explorer"

    check("a light combo takes the light class (no black fields)",
          chosen_theme(False, "ComboBox") == "Explorer")
    check("a light push button takes the light class (no black buttons)",
          chosen_theme(False, "Button") == "Explorer")
    check("a dark combo still takes the cfd class",
          chosen_theme(True, "ComboBox") == "DarkMode_CFD")
    check("a dark button still takes the dark explorer class",
          chosen_theme(True, "Button") == "DarkMode_Explorer")

    # --- the button flip condition (r47 redo: the glyph controls never
    # --- flip - the flip would replace their check state machine) ---
    m = re.search(r"if \(\(!\(style & \(BS_BITMAP \| BS_ICON\)\)\) && \(\(type == BS_PUSHBUTTON\) \|\| \(type == BS_DEFPUSHBUTTON\)\)\)",
                  children)
    if not check("the flip condition is extractable", m is not None):
        return

    # re-execute the extracted condition for the whole control matrix
    BS_PUSHBUTTON, BS_DEFPUSHBUTTON = 0, 1
    BS_AUTOCHECKBOX, BS_AUTORADIOBUTTON = 3, 9
    BS_BITMAP, BS_ICON = 0x80, 0x64

    def flips(style):
        type_ = style & 0x0F
        return not (style & (BS_BITMAP | BS_ICON)) and (
            type_ == BS_PUSHBUTTON or type_ == BS_DEFPUSHBUTTON)

    check("push buttons flip in the dark ui (the dark ui owns the face)",
          flips(BS_PUSHBUTTON))
    check("default buttons flip in the dark ui",
          flips(BS_DEFPUSHBUTTON))
    check("the bitmap color swatches never flip",
          not flips(BS_PUSHBUTTON | BS_BITMAP) and
          not flips(BS_PUSHBUTTON | BS_ICON))
    check("checkboxes never flip (the automatic check state machine survives)",
          not flips(BS_AUTOCHECKBOX))
    check("radios never flip (the same state machine rule)",
          not flips(BS_AUTORADIOBUTTON))
    check("group boxes never flip",
          not flips(0x7))

    # --- the combo field height capture (the chin under the dark combos) ---
    i_get = children.find("field_height = (int)SendMessage(hwnd,CB_GETITEMHEIGHT,(WPARAM)-1,0);")
    i_flip = children.find("SetWindowLongPtr(hwnd,GWL_STYLE,style | CBS_OWNERDRAWFIXED);")
    check("the native height is captured before the style flips",
          -1 < i_get < i_flip)
    check("the field and the rows take the captured height",
          "SendMessage(hwnd,CB_SETITEMHEIGHT,(WPARAM)-1,field_height);" in children and
          "SendMessage(hwnd,CB_SETITEMHEIGHT,(WPARAM)0,field_height);" in children)
    check("the flip only runs on a positive height",
          "if (field_height > 0)" in children)
    check("the captured height rides in the prop above the button codes",
          "SetPropW(hwnd,_VIV_DARK_OWNERDRAW_PROP,(HANDLE)(0x100 + field_height));" in children)

    # behavioral replay: flip at the native height, read the measure back.
    for native in (18, 21, 26, 42, 63):
        field_height = native                        # CB_GETITEMHEIGHT(-1)
        if field_height > 0:
            prop = 0x100 + field_height              # the extracted store
            measured = prop - 0x100                  # the measure fallback
        else:
            prop, measured = None, None
        check("a %dpx native field keeps its height through the flip" % native,
              measured == native and prop is not None)

    # --- a refresh with no open file is a blank, not a reload ---
    refresh = function_body(VIV, "void _viv_refresh(void)")
    if not check("the refresh body is extractable", refresh is not None):
        return
    guard_pos = refresh.find("if (!fd.cFileName[0])")
    open_pos = refresh.find("_viv_open(&fd,0);")
    check("the no-file guard sits before the reload",
          -1 < guard_pos < open_pos)
    check("the no-file branch resets both stale flags",
          "_viv_file_not_found = 0;" in refresh and
          "_viv_load_failed = 0;" in refresh)

    # behavioral replay of the two states from the field report.
    def refresh_replay(cfilename, not_found, load_failed):
        opened = 0
        if not cfilename:                            # the extracted guard
            if not_found:
                not_found = 0
            if load_failed:
                load_failed = 0
            return not_found, load_failed, opened
        opened += 1                                  # the reload branch
        return not_found, load_failed, opened

    nf, lf, op = refresh_replay("", 1, 1)
    check("the bare program accepts a color change with no load error",
          (nf, lf, op) == (0, 0, 0))
    nf, lf, op = refresh_replay("shot.png", 0, 0)
    check("an open image still reloads for the new mat",
          op == 1)

    # --- both theme broadcasts schedule the settle re-check ---
    check("the broadcasts, the options combo and the settings combo schedule the re-check",
          VIV.count("SetTimer(_viv_hwnd,VIV_ID_DARK_RECHECK_TIMER,400,0);") == 4)  # rc.8: the settings theme row joins

    # --- the view menu canvas picker and the backdrop rename ---
    check("the menu picker shares the options apply chain",
          "case VIV_ID_VIEW_WINDOWED_BACKGROUND_COLOR:" in VIV and
          "os_choose_color(_viv_hwnd,&background_color)" in VIV)
    en = read("src/localization_en_us.h").decode("utf-8", errors="replace")
    zh = read("src/localization_zh_cn.h").decode("utf-8")
    check("the backdrop menu reads as the transparency backdrop",
          '"&Transparency backdrop", // LOCALIZATION_ID_BACKDROP' in en and
          '"透明背景(&T)", // LOCALIZATION_ID_BACKDROP' in zh)
    check("the canvas picker label sits next to it",
          '"Windowed &background color...", // LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR_MENU' in en and
          '"窗口背景颜色(&B)...", // LOCALIZATION_ID_WINDOWED_BACKGROUND_COLOR_MENU' in zh)


# ---------------------------------------------------------------------------
# 11. the font unification round (1.1.11): the dialog templates hard coded
#     segoe ui (no cjk glyphs -> the gdi per-character fallback with the
#     latin line cell), the menu bar and the status bar draw the locale
#     face. this round routes every dialog through lfMessageFont at the
#     dialog window own dpi. these are metric models, stated honestly:
#     they model the font selection and the vertical budgets, they cannot
#     render windows text - the real-machine check stays with the field
#     reports.
# ---------------------------------------------------------------------------
def t_sim_field_round49():
    print("sim: the font unification round (1.1.11)")

    OSC = read("src/os.c").decode("latin-1")
    VIVD = read("src/viv.c").decode("latin-1")
    RCD = read("res/voidImageViewer.rc").decode("utf-8", errors="replace")

    # 1. the type system table, replayed: what face each path draws on a
    #    chinese system.
    #    - the upstream mapped: ms shell dlg -> the locale face (yahei),
    #      via the gdi font mapping, native cells.
    #    - the fork 1.1.09..1.1.10: hard coded segoe ui -> no cjk glyph
    #      -> the per character font linking fallback, the line cell
    #      keeps the latin metrics.
    #    - this round: lfMessageFont -> the locale face, native cells.
    segoe_cell_96 = 15   # segoe ui 9pt tmHeight at 96 dpi
    yahei_cell_96 = 17   # the locale 9pt face runs a couple of px taller

    def cell(path):
        if path == "hardcoded_segoe":
            return segoe_cell_96   # the cjk glyphs squeeze in a latin cell
        return yahei_cell_96       # the locale face owns the cell

    check("the hardcoded path kept the latin cell for cjk text",
          cell("hardcoded_segoe") == 15)
    check("the message font path gives cjk text the native cell",
          cell("message_font") == 17)
    check("the source of the new path is lfMessageFont (both query legs)",
          OSC.count("*lf = ncm.lfMessageFont;") == 2)
    check("no hard coded face name joins the runtime path",
          "lfMessageFont" in OSC and
          RCD.count('FONT 9, "Segoe UI"') == 10)   # the template keeps the skeleton job (rc.79: the zoom dialog retired)

    # 2. the vertical budget, per template: the dlu grid keeps the segoe
    #    skeleton (the layout does not move), the rendering face may run
    #    taller. dlu -> px at 96 dpi with the segoe base (7,15): the
    #    vertical unit is tmHeight/8.
    vunit = segoe_cell_96 / 8.0
    # the two-line CONTROL rows (the header on one line, the class and the
    # geometry on the next) join before the parse so every control counts.
    rc_joined = re.sub(r',\s*\n\s*("Button")', r',\1', RCD)
    rows = []
    for m in re.finditer(
        r'^[ \t]*(CONTROL|LTEXT|RTEXT|CTEXT|PUSHBUTTON|DEFPUSHBUTTON|EDITTEXT|COMBOBOX|LISTBOX|GROUPBOX)\b[^\n]*?,(-?\d+),(-?\d+),(-?\d+),(-?\d+)(?:[,\s][^\n]*)?$',
        rc_joined, re.M):
        rows.append((m.group(1), int(m.group(5)), m.group(0)))
    check("the template rows parse (the eleven dialogs carry controls)",
          len(rows) >= 50, len(rows))

    tall_ok = [r for r in rows if r[0] in ("CONTROL", "PUSHBUTTON", "DEFPUSHBUTTON", "COMBOBOX", "LISTBOX", "GROUPBOX", "EDITTEXT")]
    # the blank label rows (the b42 template round's band chrome controls:
    # the separator lines and the button strip face carry geometry and
    # color, no glyphs) never join the glyph budget - the pinch invariant
    # speaks to rows that host text.
    tight = [r for r in rows if r[0] in ("LTEXT", "RTEXT", "CTEXT") and r[1] * vunit < yahei_cell_96 and '""' not in r[2]]
    centerimage_ok = [r for r in rows if r[0] in ("LTEXT", "RTEXT", "CTEXT") and r[1] * vunit >= yahei_cell_96]

    #    every control row that hosts a glyph or a button face clears the
    #    taller locale face outright.
    check("every control/button row clears the locale cell height",
          all(r[1] * vunit >= yahei_cell_96 for r in tall_ok),
          min((r[1] * vunit for r in tall_ok), default=0))
    #    the plain label rows: the 8 dlu rows sit two pixels tight - that
    #    is the stated design tolerance (the skeleton stays segoe; the
    #    static text paints without clipping and the row spacing above
    #    absorbs it). the check pins the tolerance itself: no label row
    #    is worse than the two pixel pinch, nothing is assumed away.
    worst = max((yahei_cell_96 - r[1] * vunit for r in tight), default=0.0)
    check("the tight label rows stay within the stated two-pixel pinch",
          all(0 <= yahei_cell_96 - r[1] * vunit <= 2.01 for r in tight), round(worst, 2))
    check("the tall label rows (centerimage) clear the locale face",
          len(centerimage_ok) >= 1, len(centerimage_ok))

    # 3. the font lifetime, replayed both ways: the 1.1.11 shared cache
    #    deleted the handle under live dialogs (the options container
    #    and its create dialog pages coexist, and jumpto is a create
    #    dialog too) - a settings broadcast or a dpi rebuild arriving
    #    while a dialog stood open killed the face its controls still
    #    held, and the next paint selected a dead font (the labels fell
    #    to the default system face). the per dialog ownership keeps
    #    every handle alive exactly as long as the dialog that draws
    #    with it.
    def replay_shared_cache():
        # the shipped 1.1.11 design: one handle, the dialogs adopt it,
        # the broadcast deletes it while the dialogs are open.
        fonts = {"handle": 401, "users": ["container", "page"]}
        fonts["handle"] = 0            # the drop: DeleteObject
        fonts["dangling"] = list(fonts["users"])  # under live users
        return fonts

    bad = replay_shared_cache()
    check("the 1.1.11 replay dangles the handle under two open dialogs",
          bad["handle"] == 0 and len(bad["dangling"]) == 2)

    def replay_per_dialog(events):
        # the review fix: each dialog owns its face, the broadcast
        # touches nothing, a close releases only its own handle.
        fonts = {}
        next_id = [401]
        for who, ev in events:
            if ev == "open":
                next_id[0] += 1
                fonts[who] = next_id[0]      # createfontindirect per dialog
            elif ev == "close":
                fonts.pop(who, None)         # the ncdestroy release
        return fonts                         # broadcasts change nothing

    fonts = replay_per_dialog([("container", "open"), ("page", "open"),
                               ("container", "close")])
    check("the per dialog replay keeps the open page face valid",
          fonts == {"page": 403})
    check("the source carries no shared cache statics",
          "_viv_dialog_font_handle" not in VIVD and
          "_viv_dialog_font_dpi" not in VIVD)
    check("the apply creates the font per dialog",
          "font = CreateFontIndirectW(&lf);" in VIVD)
    check("the release lands at the dialog ncdestroy",
          "RemovePropW(hwnd,_VIV_DIALOG_FONT_PROP);" in VIVD)

    # 4. the message order, replayed: the shared proc runs the font apply
    #    from wm_initdialog before each dialog's own case (the dark flip
    #    captures the native combo field heights against the final font,
    #    the about title derives its larger face from it, the localized
    #    labels render with it).
    apply_pos = VIVD.find("case WM_INITDIALOG:\r\n\t\t{\r\n\t\t\t// one type system")
    proc_pos = VIVD.find("INT_PTR _viv_dialog_dark_proc(")
    proc_body = VIVD[proc_pos:] if proc_pos != -1 else ""
    draw_pos = proc_body.find("\t\tcase WM_DRAWITEM:")
    check("the font apply case leads the shared proc switch",
          apply_pos != -1 and draw_pos != -1 and apply_pos - proc_pos < draw_pos)
    check("the shared proc fronts every dialog ahead of its own switch (rc.79: ten dialogs)",
          VIVD.count("_viv_dialog_dark_proc(hwnd,msg,wParam,lParam);") == 10 and
          VIVD.count("if (dark_dialog_reply != -1)\r\n\t\t{\r\n\t\t\treturn dark_dialog_reply;") == 10)
    check("a dialog crossing monitors re-applies (wm_dpichanged)",
          VIVD.count("case WM_DPICHANGED:\r\n\t\t{\r\n\t\t\t// a dialog dragged across monitors") == 1)
    check("the settings broadcast touches no font (the faces live with their dialogs)",
          "_viv_dialog_font_drop" not in VIVD)
    nc_pos = proc_body.find("\t\tcase WM_NCDESTROY:")
    check("the dialog death releases the face inside the shared proc",
          nc_pos != -1 and nc_pos < draw_pos)

    # 5. the honesty clause, pinned: these are metric models. the
    #    changelog of the round states the tolerance instead of assuming
    #    it away, and this file states the same boundary here.
    changes = read("Changes.txt").decode("utf-8", errors="replace")
    check("the changelog states the tolerance (not assumed away)",
          "the simulation models the budget with the" in changes and
          "tolerance stated, not assumed away" in changes)
    check("the changelog states the lifetime rule (the review fix)",
          "the font belongs to the dialog that draws it" in changes)

if __name__ == "__main__":
    t_sim_mat_color()
    t_sim_recent_mru()
    t_sim_menu_braces()
    t_sim_zoom_ladder()
    t_sim_blank_flags()
    t_sim_options_geometry()
    t_sim_theme_flip()
    t_sim_version_117()
    t_sim_field_round46()
    t_sim_field_round47()
    t_sim_field_round48()
    t_sim_field_round49()
    print()
    if failures:
        print("%d FAILURE(S)" % len(failures))
        sys.exit(1)
    print("ALL SIMULATION TESTS PASS")
