#!/usr/bin/env python3
"""Regression tests for the zoom / render math in src/viv.c.

The formulas are mirrored here exactly (see the cited viv.c functions).
Run:  python3 tests/zoom_math_test.py
Exit 0 = pass.

rc.7 model: the ladder is geometric (render = fit * 1.01^pos) and each
axis is capped at 16x the NATIVE size (exactly 1600% on the status bar),
never below the pos 0 fit (fill window keeps its upscale). the rc.6
ceiling of 16x-the-LARGER-of-fit-and-native let a window larger than
the image push the cap past 24x (the 2478% report). the live top
position _viv_zoom_pos_max() is the first ladder entry that reaches
the cap.
"""
import math
import sys

STEP = 1.01            # _VIV_ZOOM_MAX ladder step (viv.c: "each zoom step grows 1.01x")
ZOOM_MAX = 1024        # _VIV_ZOOM_MAX
STEPS_PER_NOTCH = 10   # _VIV_ZOOM_STEPS_PER_NOTCH


# ---------------------------------------------------------------------------
# viv.c _viv_get_render_size(): render = fit * 1.01^pos, per axis capped
# at max(16 * native, fit), each truncated with (int).
# ---------------------------------------------------------------------------
def render_axis(fit, native, pos):
    value = fit * (STEP ** pos)
    cap = max(16 * native, fit)
    if value > cap:
        value = cap
    if value < 1:            # viv.c: a deep below-fit zoom keeps one pixel
        value = 1
    return int(value)


def render(fit_w, fit_h, pos, image_w, image_h):
    return render_axis(fit_w, image_w, pos), render_axis(fit_h, image_h, pos)


def fit_size(image_w, image_h, client_w, client_h, fill_window=0,
             keep_aspect=1, allow_shrinking=1):
    """viv.c windowed best-fit size (aspect preserved, clamped to native)."""
    if not keep_aspect:
        rw = client_w if fill_window else min(client_w, image_w)
        rh = client_h if fill_window else min(client_h, image_h)
        return rw, rh
    if (client_h * image_w) / image_h < client_w:      # tall image
        rh = client_h
        rw = (client_h * image_w + image_h - 1) // image_h
    else:                                              # long image
        rw = client_w
        rh = (client_w * image_h + image_w - 1) // image_w
    if not fill_window and (rw > image_w or rh > image_h):
        return image_w, image_h
    if not allow_shrinking and (rw < image_w or rh < image_h):
        return image_w, image_h
    return rw, rh


def pos_max(fit_w, fit_h, image_w, image_h):
    """viv.c _viv_zoom_pos_max(): the first pos that reaches the cap."""
    cap_w = max(16 * image_w, fit_w)
    cap_h = max(16 * image_h, fit_h)
    for pos in range(ZOOM_MAX):
        if fit_w * (STEP ** pos) >= cap_w:
            return pos
        if fit_h * (STEP ** pos) >= cap_h:
            return pos
    return ZOOM_MAX - 1


# ---------------------------------------------------------------------------
# _viv_do_mousewheel_action(): steps = round(|delta| * STEPS_PER_NOTCH / 120)
# ---------------------------------------------------------------------------
def steps_from_delta(delta):
    if delta == 0:
        return 1
    s = (abs(delta) * STEPS_PER_NOTCH + 60) // 120
    return s if s else 1


# ---------------------------------------------------------------------------
# paint magnify path (viv.c WM_PAINT): whole-destination StretchBlt is only
# taken when the destination is fully on screen; otherwise the clip limited
# stretch bounds the work by the client area.
# ---------------------------------------------------------------------------
def paint_work_megapixels(rw, rh, client_w, client_h):
    if rw <= client_w and rh <= client_h:
        return (rw * rh) / 1e6
    return (client_w * client_h) / 1e6


# ============================================================ test geometry
GEOMETRIES = [
    # (image_w, image_h, client_w, client_h)
    (4000, 3000, 1600, 900),     # photo larger than window (the common case)
    (800, 600, 1600, 900),       # photo smaller than window
    (3840, 2160, 1920, 1080),    # 16:9 in 16:9
    (1234, 777, 1600, 900),      # awkward sizes, rounding stress
    (101, 100, 1600, 900),       # near-square tiny, ceil stress
    (6000, 400, 1600, 900),      # panorama
    (400, 6000, 1600, 900),      # tower
    (321, 241, 500, 400),        # small everything
]

failures = []


def check(name, ok, detail=""):
    if not ok:
        failures.append(f"{name}: {detail}")
        print(f"FAIL {name} {detail}")
    else:
        print(f"ok   {name} {detail}")


def t_aspect_invariant():
    """Rendered aspect must track the image aspect (<= 2px rounding) at every
    zoom level for every geometry. This is the beta.6 aspect-ratio guarantee:
    with pan&scan removed there is no code path that can decouple x from y."""
    worst = 0.0
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        # the fit itself is integer-rounded (upstream): allow 1px on the small
        # fit axis, plus 2px of render truncation on the small render axis.
        fit_allow = 1.0 / min(fw, fh)
        top = pos_max(fw, fh, iw, ih)
        for pos in range(0, top + 1, 7):
            rw, rh = render(fw, fh, pos, iw, ih)
            ar_img = iw / ih
            ar_r = rw / rh
            drift = abs(ar_r - ar_img) / ar_img
            worst = max(worst, drift)
            if drift > fit_allow + (2.0 / min(rw, rh)) + 1e-12:
                check("aspect", False, f"{iw}x{ih}@{cw}x{ch} pos {pos}: {ar_r:.4f} vs {ar_img:.4f}")
                return
    check("aspect invariant (all geometries, all levels)", True, f"worst drift {worst:.5%}")


def t_geometric_ladder():
    """The render ladder must match fit * 1.01^pos within rounding below the
    cap (beta.5 fix retained; the cap only snaps the last step)."""
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        top = pos_max(fw, fh, iw, ih)
        for pos in (0, 1, 2, 12, 70, 140, top - 1 if top >= 141 else 140):
            exact_w = fw * (STEP ** pos)
            exact_h = fh * (STEP ** pos)
            rw, rh = render(fw, fh, pos, iw, ih)
            assert abs(rw - exact_w) <= 2.0, (iw, ih, pos, rw, exact_w)
            assert abs(rh - exact_h) <= 2.0, (iw, ih, pos, rh, exact_h)
    check("geometric ladder = fit * 1.01^pos (+/-2px)", True)


def t_sixteen_x_cap():
    """The zoom ceiling is exactly 1600% of NATIVE (rc.7):
    - a photo that fits the window tops out at exactly 1600% (beta.6 showed
      a confusing 1590% = 1.01^278)
    - a photo larger than the window keeps deep zoom: 1600% of native
      (beta.6 lost this: a 4000px photo in a 1600px window capped at 477%)
    - fill window (fit upscaled past native) also tops out at 1600% - the
      rc.6 16x-the-LARGER rule let it reach 16x the fit (the 2478% report)
    - an absurd fill (fit already past 16x native) keeps the fit as its
      cap floor: the pos 0 size is a layout decision, not a zoom level"""
    # small image: fit == native -> 1600% exactly
    iw, ih, cw, ch = 800, 600, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    top = pos_max(fw, fh, iw, ih)
    rw, rh = render(fw, fh, top, iw, ih)
    pct = rw / iw * 100
    check("small image max zoom is exactly 1600%", pct == 1600.0, f"{pct:.1f}%")

    # large photo: fit < native -> still 1600% of native
    iw, ih, cw, ch = 4000, 3000, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    top = pos_max(fw, fh, iw, ih)
    rw, rh = render(fw, fh, top, iw, ih)
    pct = rw / iw * 100
    check("large photo deep zoom restored (1600%)", pct == 1600.0, f"{pct:.1f}%")

    # fill window: fit upscaled past native -> still 1600% (the 2478% bug)
    iw, ih, cw, ch = 500, 400, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch, fill_window=1)
    top = pos_max(fw, fh, iw, ih)
    rw, rh = render(fw, fh, top, iw, ih)
    pct = (rw / iw + rh / ih) / 2 * 100
    check("fill window tops out at 1600% (was 2478%-class)",
          pct == 1600.0, f"{pct:.1f}% (fit {fw}x{fh})")

    # absurd fill: fit already past 16x native -> the cap floors at the fit
    iw, ih, cw, ch = 40, 30, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch, fill_window=1)
    top = pos_max(fw, fh, iw, ih)
    rw, rh = render(fw, fh, top, iw, ih)
    check("absurd fill keeps the fit as the cap floor",
          (rw, rh) == (fw, fh), f"top {top} render {rw}x{rh} fit {fw}x{fh}")

    # the ladder is long enough for extreme cases
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        top = pos_max(fw, fh, iw, ih)
        rw, rh = render(fw, fh, top, iw, ih)
        assert rw == max(16 * iw, fw), (iw, ih, top, rw)
        assert rh == max(16 * ih, fh), (iw, ih, top, rh)
    check("cap = max(16x native, fit) for every geometry", True)


def t_pos_max_no_dead_zone():
    """Positions beyond pos_max render identically; the wheel clamps at
    pos_max instead (a dead zone of identical sizes would eat wheel events)."""
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        top = pos_max(fw, fh, iw, ih)
        r_top = render(fw, fh, top, iw, ih)
        r_beyond = render(fw, fh, top + 5, iw, ih)
        assert r_top == r_beyond, (iw, ih, r_top, r_beyond)
        # the step below the top is still growing (no dead zone below)
        if top > 0:
            r_below = render(fw, fh, top - 1, iw, ih)
            assert r_below[0] < r_top[0] or r_below[1] < r_top[1], (iw, ih)
    check("pos_max = first cap position, no dead wheel below", True)


def t_pinch_steps():
    """Pinch encoding: n steps are passed as n * (120/STEPS_PER_NOTCH) delta
    units and must decode back to exactly n steps."""
    ok = True
    for n in (1, 3, 7, 12, 40, 120):
        delta = n * (120 // STEPS_PER_NOTCH)
        if steps_from_delta(delta) != n:
            check(f"pinch n={n} roundtrip", False, f"delta {delta} -> {steps_from_delta(delta)}")
            ok = False
    if ok:
        check("pinch step roundtrip (1..120)", True)
    check("wheel notch = 10 steps", steps_from_delta(120) == 10)
    check("wheel double-flick = 20 steps", steps_from_delta(240) == 20)
    check("high-res 40 = 3 steps", steps_from_delta(40) == 3)


def t_paint_work_bound():
    """Magnified paint work must be bounded by the client area once the
    destination is (partly) off screen — the beta.6 zoom-lag fix."""
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        top = pos_max(fw, fh, iw, ih)
        for pos in (50, 150, top):
            rw, rh = render(fw, fh, pos, iw, ih)
            work = paint_work_megapixels(rw, rh, cw, ch)
            assert work <= (cw * ch) / 1e6 + 1e-9, (iw, ih, pos, work)
    # and the worst case really is huge without the fix (sanity of the model):
    iw, ih, cw, ch = 4000, 3000, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    rw, rh = render(fw, fh, pos_max(fw, fh, iw, ih), iw, ih)
    assert (rw * rh) / 1e6 > 100
    check("paint work bounded by client", True,
          f"deep zoom {rw}x{rh} -> {1600*900/1e6:.2f} MP work")


def t_status_single_percent():
    """The status format must show exactly ONE zoom percent (the beta.6 fix:
    two independent percents implied x/y zoom could diverge)."""
    for name, path in (("en", "src/localization_en_us.h"),
                       ("zh", "src/localization_zh_cn.h")):
        n = 0
        for line in open(path, "rb"):
            if b"POS_ZOOM_FORMAT" in line:
                n = line.count(b"%d%%")
        check(f"{name} status has exactly one percent", n == 1, f"found {n}")


def t_pinch_follows_fingers():
    """2x finger spread must roughly double the render (beta.5 guarantee kept)."""
    iw, ih, cw, ch = 4000, 3000, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    pos2 = int(math.log(2.0) / math.log(STEP))
    rw0, rh0 = render(fw, fh, 0, iw, ih)
    rw2, rh2 = render(fw, fh, pos2, iw, ih)
    check("2x fingers -> ~2x render", abs(rw2 / rw0 - 2.0) < 0.02,
          f"{rw2 / rw0:.3f}")


def t_status_percent_is_native_relative():
    """The displayed percent = render / native: 1:1 reads 100%, fit of a large
    photo reads below 100, and the status call passes exactly one int (the
    beta.7 fix for the negative garbage percent)."""
    # 1:1 at pos 0 for a small image (fit == native): 100%
    iw, ih, cw, ch = 800, 600, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    rw, rh = render(fw, fh, 0, iw, ih)
    pct = (rw / iw + rh / ih) / 2 * 100
    check("fit of a small image displays 100%", abs(pct - 100.0) < 1.0, f"{pct:.1f}%")

    # large photo at fit: below 100%
    iw, ih, cw, ch = 4000, 3000, 1600, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    rw, rh = render(fw, fh, 0, iw, ih)
    pct = (rw / iw + rh / ih) / 2 * 100
    check("fit of a large photo displays < 100%", 0 < pct < 100, f"{pct:.1f}%")




# ---------------------------------------------------------------------------
# beta.8: the 1:1 exit binary searches (equivalence + measurement bound)
# and the ladder-top cache signature.
# ---------------------------------------------------------------------------
def linear_exit_ascending(old_rw, fit_w, fit_h, image_w, image_h):
    """viv.c beta.7 linear scan: first pos in [0, ZOOM_MAX) with rw > old_rw,
    else ZOOM_MAX."""
    for pos in range(ZOOM_MAX):
        rw, _rh = render(fit_w, fit_h, pos, image_w, image_h)
        if rw > old_rw:
            return pos
    return ZOOM_MAX


def binary_exit_ascending(old_rw, fit_w, fit_h, image_w, image_h, counter):
    """viv.c beta.8 binary search (mirrored exactly)."""
    lo, hi = 0, ZOOM_MAX
    while lo < hi:
        mid = lo + (hi - lo) // 2
        counter[0] += 1
        rw, _rh = render(fit_w, fit_h, mid, image_w, image_h)
        if rw > old_rw:
            hi = mid
        else:
            lo = mid + 1
    return lo


def linear_exit_descending(old_rw, fit_w, fit_h, image_w, image_h, top):
    """viv.c beta.7 linear scan from the ladder top: largest pos with
    rw < old_rw, else -1."""
    for pos in range(top, -1, -1):
        rw, _rh = render(fit_w, fit_h, pos, image_w, image_h)
        if rw < old_rw:
            return pos
    return -1


def binary_exit_descending(old_rw, fit_w, fit_h, image_w, image_h, top, counter):
    """viv.c beta.8 binary search (mirrored exactly)."""
    lo, hi = 0, top + 1
    while lo < hi:
        mid = lo + (hi - lo) // 2
        counter[0] += 1
        rw, _rh = render(fit_w, fit_h, mid, image_w, image_h)
        if rw < old_rw:
            lo = mid + 1
        else:
            hi = mid
    return lo - 1


def t_binary_search_equivalence():
    """beta.8: the binary-search 1:1 exits must return exactly what the beta.7
    linear scans returned, and must measure O(log n) sizes, not O(n)."""
    worst_calls = 0
    for (iw, ih, cw, ch) in GEOMETRIES:
        fw, fh = fit_size(iw, ih, cw, ch)
        top = pos_max(fw, fh, iw, ih)
        r_top = render(fw, fh, top, iw, ih)[0]
        # exits from several old sizes: fit, native, mid ladder, cap and the
        # cap +/-1 (a size no ladder step equals), plus degenerate 0/1.
        olds = [fw, fh, iw, ih, 0, 1,
                render(fw, fh, top // 2, iw, ih)[0],
                r_top, r_top + 1, r_top - 1]
        for old_rw in olds:
            lin = linear_exit_ascending(old_rw, fw, fh, iw, ih)
            ctr = [0]
            binr = binary_exit_ascending(old_rw, fw, fh, iw, ih, ctr)
            worst_calls = max(worst_calls, ctr[0])
            assert lin == binr, ("asc", iw, ih, old_rw, lin, binr)
            lin = linear_exit_descending(old_rw, fw, fh, iw, ih, top)
            ctr = [0]
            binr = binary_exit_descending(old_rw, fw, fh, iw, ih, top, ctr)
            worst_calls = max(worst_calls, ctr[0])
            assert lin == binr, ("desc", iw, ih, old_rw, lin, binr)
    check("1:1 exit binary search == linear scan (all geometries)", True,
          f"worst {worst_calls} measurements, linear worst is {ZOOM_MAX}")


def t_pos_max_cache_signature():
    """beta.8: the ladder-top cache returns the measured value whenever the
    dependency signature (image, viewport, fill/aspect/shrink settings)
    matches, and re-measures when any input changes."""
    seq = [
        (800, 600, 1600, 900, 0, 1, 1),
        (800, 600, 1600, 900, 0, 1, 1),      # same -> cache hit
        (800, 600, 1024, 768, 0, 1, 1),      # viewport change
        (4000, 3000, 1024, 768, 0, 1, 1),    # image change
        (4000, 3000, 1024, 768, 1, 1, 1),    # fill_window change
        (4000, 3000, 1024, 768, 1, 0, 1),    # keep_aspect change
        (4000, 3000, 1024, 768, 1, 0, 0),    # allow_shrinking change
        (4000, 3000, 1024, 768, 1, 0, 0),    # same -> cache hit
        (0, 0, 1024, 768, 1, 0, 0),          # no image
        (4000, 3000, 1024, 768, 1, 0, 0),    # image back
    ]
    cache = {}
    hits = 0
    for (iw, ih, cw, ch, fill, aspect, shrink) in seq:
        if iw and ih:
            fw, fh = fit_size(iw, ih, cw, ch, fill, aspect, shrink)
            measured = pos_max(fw, fh, iw, ih)
        else:
            measured = ZOOM_MAX - 1  # viv.c: no image keeps the full range
        sig = (iw, ih, cw, ch, fill, aspect, shrink)
        if sig in cache:
            assert cache[sig] == measured, (sig, cache[sig], measured)
            hits += 1
        else:
            cache[sig] = measured
    # states 0-1, 6-7 and 9 (returns to the 6-7 signature) are repeats.
    check("ladder-top cache signature model (same sig -> same top)", hits == 3,
          f"{hits} hits across {len(seq)} states")



# ---------------------------------------------------------------------------
# rc.1: percent based button stepping. mirrors _viv_zoom_in (snap math),
# _viv_zoom_set_percent (binary search + closest pick + force rule) and the
# 1:1 special case for an exact 100%.
# ---------------------------------------------------------------------------
def percent_of(fit_w, fit_h, image_w, image_h, pos):
    rw, rh = render(fit_w, fit_h, pos, image_w, image_h)
    if not image_w or not image_h or not rw or not rh:
        return 100
    return int((((rw / image_w) + (rh / image_h)) / 2.0) * 100.0 + 0.5)


def snap_target(percent, out):
    """the _viv_zoom_in target math, mirrored exactly.
    rc.13 (the field report): the snap is direction strict - an out click
    lands on the multiple strictly below, an in click on the one strictly
    above, so the first click after a wheel stop (or from a fresh best
    fit) always moves in the click direction. the rc.1 nearest-multiple
    snap stepped 31 -> 30 on a zoom-in click and the field read it as
    "zoom needs two clicks to work"."""
    if out:
        target = ((percent - 1) // 10) * 10
    else:
        target = ((percent // 10) * 10) + 10
    if target < 1:
        target = 1
    return target


def pos_for_percent(target, pm, fit_w, fit_h, image_w, image_h, strict=0):
    """the _viv_zoom_pos_for_percent search, mirrored exactly.
    strict=1 returns the first position that reaches the target
    (no closest-pick below) - used by the force fallback.
    rc.13: the search starts at the ladder floor (negative when shrinking
    is allowed), mirroring the C lo = _viv_zoom_pos_floor()."""
    lo, hi = pos_floor(), pm + 1
    while lo < hi:
        mid = lo + ((hi - lo) // 2)
        if percent_of(fit_w, fit_h, image_w, image_h, mid) >= target:
            hi = mid
        else:
            lo = mid + 1
    if lo > pm:
        lo = pm
    if (lo > 0) and (not strict):
        below = percent_of(fit_w, fit_h, image_w, image_h, lo - 1)
        at = percent_of(fw, fh, iw, ih, lo) if False else percent_of(fit_w, fit_h, image_w, image_h, lo)
        if (target - below) < (at - target):
            lo = lo - 1
    return lo


def button_click(state, out, pm, fit_w, fit_h, image_w, image_h):
    """state = (pos, is_1to1). one click of the zoom in/out button,
    mirroring _viv_zoom_in + _viv_zoom_set_percent (with force)."""
    pos, is_1to1 = state
    percent = 100 if is_1to1 else percent_of(fit_w, fit_h, image_w, image_h, pos)
    # floor guard: a zoom out click at the true ladder floor is a no-op
    # (rc.13: pos 0 is the bestfit anchor, not the floor - the below-fit
    # extension reaches 278 steps under it, and the old pos==0 gate ate
    # the first out click on every freshly opened image).
    if out and (not is_1to1) and pos <= pos_floor():
        return (pos, is_1to1)
    old_percent = percent
    target = snap_target(percent, out)
    if target == 100:
        return (0, True)          # exact 100% enters 1:1
    if is_1to1:
        is_1to1 = False
        old_pos = 0
    else:
        old_pos = pos
    new_pos = pos_for_percent(target, pm, fit_w, fit_h, image_w, image_h)
    # force rule: when the exact target is undisplayable (landed != target)
    # and the landing does not move in the click direction, jump to the
    # next multiple of 10 in the click direction.
    landed = percent_of(fit_w, fit_h, image_w, image_h, new_pos)
    force = 1 if not out else -1
    if (not is_1to1) and landed != target and (
            (force > 0 and new_pos <= old_pos) or
            (force < 0 and new_pos >= old_pos)):
        nxt = (old_percent // 10) * 10 + (10 if not out else -10)
        if nxt < 1:
            nxt = 1
        new_pos = pos_for_percent(nxt, pm, fit_w, fit_h, image_w, image_h, strict=1)
    # sparse ladder zones (past ~1400% one position is worth ~14 points):
    # a 10 point target can sit between two positions and even the strict
    # jump lands on the old one. the click still owes the user a move:
    # step one position in the click direction (rc.13).
    if (force > 0 and new_pos <= old_pos) or (force < 0 and new_pos >= old_pos):
        new_pos = old_pos + (1 if force > 0 else -1)
    # the live clamp: [floor, pm] (the old model clamped to 0 and hid the
    # whole below-fit range).
    new_pos = clamp_pos(new_pos)
    if new_pos > pm:
        new_pos = pm
    return (new_pos, is_1to1)


def t_percent_stepping():
    # unit checks of the snap math (the user visible contract)
    cases = [
        (34, False, 40), (34, True, 30),
        (37, False, 40), (37, True, 30),
        (35, False, 40), (35, True, 30),
        (30, False, 40), (30, True, 20),
        (100, False, 110), (100, True, 90),
        (9, False, 10), (9, True, 1),
        (1, False, 10), (1, True, 1),
        (1600, True, 1590), (1447, False, 1450),
    ]
    for percent, out, want in cases:
        got = snap_target(percent, out)
        check(f"snap {percent} {'out' if out else 'in'} -> {want}", got == want, f"got {got}")

    geometries = [
        ("photo 4000x3000 in 800x600", 4000, 3000, 800, 600),
        ("exact fit 800x600", 800, 600, 800, 600),
        ("small icon 100x100 in 800x600", 100, 100, 800, 600),
        ("panorama 12000x300 in 800x600", 12000, 300, 800, 600),
        ("tall 600x4000 in 800x600", 600, 4000, 800, 600),
    ]
    for name, iw, ih, cw, ch in geometries:
        fw, fh = fit_size(iw, ih, cw, ch)
        pm = pos_max(fw, fh, iw, ih)
        pmin = percent_of(fw, fh, iw, ih, 0)
        pmax = percent_of(fw, fh, iw, ih, pm)

        # from every 128th position: one click must respect the spec:
        # - the result stays inside the live ladder
        # - from a multiple of 10 the click moves one step in its direction
        #   (or is already at the domain edge / enters 1:1)
        # - from a non multiple the click lands on the nearest multiple
        #   (possibly moving toward it against the click direction - the
        #   literal spec) or makes visible progress where the ladder is too
        #   sparse to display multiples (~14% apart past 1400%)
        # rc.13 contract: a button click always moves in its direction -
        # up for zoom in (unless at the top), down for zoom out (unless at
        # the below-fit floor). the old nearest-multiple snap could move
        # against the click; the field read that as a dead first click.
        # the live domain is [floor, pm]: an out click may land below the
        # best fit (the below-fit extension).
        bad = 0
        for pos in range(0, pm + 1, max(1, pm // 128)):
            new_pos, is_1to1 = button_click((pos, False), False, pm, fw, fh, iw, ih)
            if new_pos < pos_floor() or new_pos > pm or (is_1to1 and new_pos != 0):
                bad += 1000
            elif not is_1to1:
                if new_pos <= pos and pos != pm:
                    bad += 1
        check(f"{name}: every zoom in click moves up", bad == 0, str(bad))

        bad = 0
        for pos in range(0, pm + 1, max(1, pm // 128)):
            new_pos, _ = button_click((pos, False), True, pm, fw, fh, iw, ih)
            if new_pos < pos_floor() or new_pos > pm:
                bad += 1000
            elif new_pos >= pos and pos != pos_floor():
                bad += 1
        check(f"{name}: every zoom out click moves down", bad == 0, str(bad))

        # the domain is respected
        check(f"{name}: ladder domain percent {pmin}..{pmax}",
              0 < pmin <= 100 or pmin >= 1)
        check(f"{name}: top reaches the cap region", pmax >= 100)

        # repeated IN from the floor: the first click lands on a multiple of
        # 10 (or makes progress in a sparse zone), then steps of 10
        state = (0, False)
        prev = percent_of(fw, fh, iw, ih, 0)
        ok_seq = True
        last_mult = None
        for _ in range(40):
            state = button_click(state, False, pm, fw, fh, iw, ih)
            pos, is_1to1 = state
            cur = 100 if is_1to1 else percent_of(fw, fh, iw, ih, pos)
            if cur <= prev:
                ok_seq = False
                break
            # the ladder renders in 1.01x steps, so every multiple of 10 is
            # displayable only below ~100% (delta per position <= 1). above
            # that the clicks still step one ~10% multiple, but the shown
            # integer can be a few points off the exact multiple.
            if (cur % 10) == 0:
                if (last_mult is not None and cur - last_mult != 10
                        and last_mult <= 100):
                    ok_seq = False      # dense zone: exact 10 chains
                    break
                last_mult = cur
            gain = cur - prev
            if (prev % 10) == 0:
                # from a multiple: one ~10% step
                if not (5 <= gain <= (25 if cur > 1000 else 15)):
                    ok_seq = False
                    break
            else:
                # from a non multiple: the snap lands on (or past) the
                # nearest multiple - forward, possibly small, never big.
                if not (0 < gain <= (25 if cur > 1000 else 15)):
                    ok_seq = False
                    break
            prev = cur
        check(f"{name}: repeated zoom in climbs in ~10% steps, exact below 100%", ok_seq,
              f"last {prev} mult {last_mult}")

        # 1:1 entry/exit through the buttons
        state = button_click(state, False, pm, fw, fh, iw, ih)
        # walk to exactly 100 by clicking OUT from 1:1 model is overkill:
        # entering 1:1 happens at target 100; verify exit clicks move.
        state = (0, True)   # in 1:1
        s1 = button_click(state, False, pm, fw, fh, iw, ih)
        s2 = button_click(state, True, pm, fw, fh, iw, ih)
        check(f"{name}: 1:1 in/out clicks leave the mode zoomed",
              (not s1[1]) and (not s2[1]) and s1[0] >= pos_floor() and s2[0] >= pos_floor())




# ---------------------------------------------------------------------------
# rc.42: the below-fit extension. _VIV_ZOOM_SHRINK_STEPS = 278 (viv.c):
# the ladder reaches down to fit * 1.01^-278 ~= fit/16 while shrinking is
# allowed; the option off keeps the historic best-fit floor.
# ---------------------------------------------------------------------------
SHRINK_STEPS = 278  # _VIV_ZOOM_SHRINK_STEPS

def pos_floor(allow_shrinking=1):
    """viv.c _viv_zoom_pos_floor()."""
    return -SHRINK_STEPS if allow_shrinking else 0

def clamp_pos(zoom_pos, allow_shrinking=1):
    """viv.c _viv_clamp_zoom_pos() (the top clamps separately)."""
    floor = pos_floor(allow_shrinking)
    if zoom_pos <= floor:
        return floor
    return zoom_pos

def t_below_fit_range():
    """The field report: pinch-out could never zoom below the best fit - a
    fill-window upscale locked a 200% minimum and a windowed fit locked the
    range at the shrink size. the extension mirrors the 16x native cap."""
    # a small image in a big window with fill window on: the fit is a 2x
    # upscale (the 200% floor from the report).
    iw, ih, cw, ch = 600, 400, 1200, 800
    fw, fh = fit_size(iw, ih, cw, ch, fill_window=1)
    assert (fw, fh) == (1200, 800), (fw, fh)
    rw, rh = render(fw, fh, 0, iw, ih)
    check("fill window fit is the 200% upscale", rw == 1200 and rh == 800,
          f"{rw}x{rh}")
    # pinch out to the floor: the render shrinks below native and lands
    # near fit/16.
    floor = pos_floor(1)
    rw, rh = render(fw, fh, floor, iw, ih)
    check("the floor renders about fit/16",
          abs(rw - fw / 16) < 4 and abs(rh - fh / 16) < 4, f"{rw}x{rh}")
    check("the floor is below native", rw < iw and rh < ih, f"{rw}x{rh}")
    # the percent readout along the way passes through 100% (native): the
    # position whose render is the native size.
    pos_native = 0
    for pos in range(floor, 1):
        rw, rh = render(fw, fh, pos, iw, ih)
        if rw <= iw:
            pos_native = pos
            break
    check("the ladder passes through native below the fit", pos_native < 0,
          f"native at pos {pos_native}")
    # the clamp: zooming far below the floor settles on the floor.
    check("the clamp settles at the floor", clamp_pos(floor - 500) == floor)
    check("the clamp keeps valid negatives", clamp_pos(-50) == -50)
    # the option off: the historic floor (the fit) stays.
    check("no shrinking keeps the fit floor", pos_floor(0) == 0)
    check("no shrinking clamps negatives to the fit", clamp_pos(-50, 0) == 0)
    # a tiny image: the deepest zoom keeps a one pixel render.
    rw, rh = render(10, 8, floor, 2, 1)
    check("the deepest zoom of a tiny image keeps 1px", rw >= 1 and rh >= 1,
          f"{rw}x{rh}")
    # the field report's second case: a large image whose fit is 75% - the
    # pinch range used to stop dead at the fit, now it continues to ~fit/16.
    iw, ih, cw, ch = 4000, 3000, 1200, 900
    fw, fh = fit_size(iw, ih, cw, ch)
    rw, rh = render(fw, fh, floor, iw, ih)
    check("a large image shrinks to about fit/16 at the floor",
          abs(rw - fw / 16) < 4, f"{rw} vs {fw/16:.1f}")



# ---------------------------------------------------------------------------
# rc.13: the field report. "zoom needs two clicks to work" on non-win11
# machines: a fresh large photo sits at a non-multiple best fit percent
# (31%), the nearest-multiple snap stepped the first zoom-in click DOWN
# to 30 (invisible), and the first zoom-out click from best fit hit the
# dead pos==0 gate. both clicks must now move on the FIRST press.
# ---------------------------------------------------------------------------
def t_field_report_first_click():
    # 2580x1935 photo in an 800x600 viewport: best fit renders 800x600,
    # the native-relative percent is ~31 (the report's exact scenario).
    iw, ih, cw, ch = 2580, 1935, 800, 600
    fw, fh = fit_size(iw, ih, cw, ch)
    check("the report geometry fits the viewport", (fw, fh) == (800, 600),
          f"{fw}x{fh}")
    pm = pos_max(fw, fh, iw, ih)
    p0 = percent_of(fw, fh, iw, ih, 0)
    check("best fit sits at the reported ~31%", 28 <= p0 <= 34, str(p0))

    # first zoom-in click: the percent must go UP onto the multiple above
    # (31 -> 40), never down to 30.
    npos, n1to1 = button_click((0, False), False, pm, fw, fh, iw, ih)
    np = percent_of(fw, fh, iw, ih, npos)
    check("the first zoom-in click enlarges (31 -> 40, not 30)",
          (not n1to1) and np > p0 and (np % 10) == 0, f"{p0} -> {np}")

    # first zoom-out click from best fit: the percent must go DOWN (the
    # below-fit ladder answers; the pos==0 gate used to eat the click).
    opos, _ = button_click((0, False), True, pm, fw, fh, iw, ih)
    op = percent_of(fw, fh, iw, ih, opos)
    check("the first zoom-out click from best fit shrinks",
          op < p0 and opos < 0 and opos >= pos_floor(),
          f"{p0} -> {op} (pos {opos})")

    # the sparse zone stall: past ~1400% one ladder position is worth ~14
    # points, so a 10 point target can land on the old position. the
    # single-position fallback still owes the user a move.
    # climb near the top first
    pos = pm - 2 if pm >= 2 else pm
    p = percent_of(fw, fh, iw, ih, pos)
    npos2, _ = button_click((pos, False), True, pm, fw, fh, iw, ih)
    check("a zoom out click in the sparse zone still moves down",
          npos2 < pos or pos == pos_floor(), f"pos {pos} -> {npos2} at {p}%")


if __name__ == "__main__":
    t_aspect_invariant()
    t_geometric_ladder()
    t_sixteen_x_cap()
    t_pos_max_no_dead_zone()
    t_pinch_steps()
    t_paint_work_bound()
    t_status_single_percent()
    t_pinch_follows_fingers()
    t_status_percent_is_native_relative()
    t_binary_search_equivalence()
    t_pos_max_cache_signature()
    t_percent_stepping()
    t_below_fit_range()
    t_field_report_first_click()
    print()
    if failures:
        print(f"{len(failures)} FAILURE(S)")
        sys.exit(1)
    print("ALL ZOOM MATH TESTS PASS")
