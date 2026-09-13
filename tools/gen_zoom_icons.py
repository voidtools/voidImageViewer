#!/usr/bin/env python3
# Generate zoomin.ico / zoomout.ico for voidImageViewer touch/zoom feature.
# Style: matches existing toolbar icons (black outline + blue fill).
# Sizes: 16, 32, 48. 32bpp ARGB + AND mask. 8x supersampling.

import struct
import math

SIZES = [16, 32, 48]

# palette (matches upstream 1to1 icon tones)
OUTLINE = (0x00, 0x00, 0x00)        # black
FILL_TOP = (0x9e, 0xcd, 0xe7)       # light blue
FILL_BOTTOM = (0x71, 0xb9, 0xe2)    # mid blue

SS = 8  # supersample factor


def render_icon(size, is_zoomin):
    """Returns list of rows (top-down) of (r,g,b,a) accumulators."""
    acc = [[(0, 0, 0, 0)] * size for _ in range(size)]

    # geometry in logical units (fractions of size)
    cx, cy = 0.40, 0.40
    lens_r = 0.28
    stroke = max(1.0, size / 16.0)  # 16->1, 32->2, 48->3 pixels
    handle_w = 0.11
    handle_cap = 0.03  # rounded cap radius extension

    # handle segment: from lens edge at 45deg to bottom-right corner area
    hx0 = cx + lens_r * 0.7071
    hy0 = cy + lens_r * 0.7071
    hx1 = 0.86
    hy1 = 0.86

    # plus / minus geometry
    arm = 0.11     # half-length of cross arm
    thick = 0.06   # half-thickness

    total = size * SS

    def lens_fill_color(fy):
        # fy: 0 at lens top, 1 at lens bottom
        t = min(max(fy, 0.0), 1.0)
        return tuple(int(FILL_TOP[i] + (FILL_BOTTOM[i] - FILL_TOP[i]) * t) for i in range(3))

    for sy in range(total):
        y = (sy + 0.5) / total
        row_i = sy // SS
        for sx in range(total):
            x = (sx + 0.5) / total
            dx = x - cx
            dy = y - cy
            dist = math.sqrt(dx * dx + dy * dy)

            color = None

            # plus / minus (drawn on top)
            if dist <= lens_r * 0.78:
                ax = abs(x - cx)
                ay = abs(y - cy)
                if is_zoomin:
                    in_cross = (ax <= arm and ay <= thick) or (ax <= thick and ay <= arm)
                else:
                    in_cross = (ax <= arm and ay <= thick) and not (ax <= thick and ay <= arm)
                if in_cross:
                    color = OUTLINE

            if color is None:
                # handle: distance to segment
                segdx = hx1 - hx0
                segdy = hy1 - hy0
                seglen2 = segdx * segdx + segdy * segdy
                t = ((x - hx0) * segdx + (y - hy0) * segdy) / seglen2
                t = min(max(t, 0.0), 1.0)
                pxproj = hx0 + t * segdx
                pyproj = hy0 + t * segdy
                dseg = math.sqrt((x - pxproj) ** 2 + (y - pyproj) ** 2)
                if dseg <= handle_w / 2:
                    color = OUTLINE
                else:
                    # lens
                    if dist <= lens_r:
                        if dist >= lens_r - stroke / size:
                            color = OUTLINE
                        else:
                            # fill with vertical gradient inside lens
                            fy = (y - (cy - lens_r)) / (2 * lens_r)
                            color = lens_fill_color(fy)

            # accumulate into supersampled cell
            ci = sx // SS
            r_, g_, b_, a_ = acc[row_i][ci]
            if color is not None:
                acc[row_i][ci] = (
                    r_ + color[0],
                    g_ + color[1],
                    b_ + color[2],
                    a_ + 1,
                )

    # normalize supersampling
    out = []
    n = SS * SS
    for row in acc:
        orow = []
        for (r_, g_, b_, a_) in row:
            a = a_ / n
            if a > 0:
                ai = min(255, int(a * 255 + 0.5))
                r__ = min(255, int(r_ / a_ + 0.5))
                g__ = min(255, int(g_ / a_ + 0.5))
                b__ = min(255, int(b_ / a_ + 0.5))
                orow.append((r__, g__, b__, ai))
            else:
                orow.append((0, 0, 0, 0))
        out.append(orow)
    return out


def make_ico(filename, is_zoomin):
    images = []
    for size in SIZES:
        rows = render_icon(size, is_zoomin)
        # build BMP payload: pixel data bottom-up BGRA + AND mask
        row_bytes = ((size + 31) // 32) * 4
        pixel_data = bytearray()
        and_data = bytearray()
        for y in range(size - 1, -1, -1):
            for x in range(size):
                r_, g_, b_, a = rows[y][x]
                pixel_data += bytes((b_, g_, r_, a))
        for y in range(size - 1, -1, -1):
            row = bytearray(row_bytes)
            for x in range(size):
                a = rows[y][x][3]
                if a == 0:
                    byte_i = x // 8
                    row[byte_i] |= 0x80 >> (x % 8)
            and_data += row

        mask_size = row_bytes * size
        bmp = struct.pack(
            '<IiiHHIIiiII',
            40, size, size * 2, 1, 32, 0,
            len(pixel_data) + mask_size, 0, 0, 0, 0,
        )
        bmp += bytes(pixel_data)
        bmp += bytes(and_data)
        images.append((size, bmp))

    # write ICO
    out = struct.pack('<HHH', 0, 1, len(images))
    offset = 6 + 16 * len(images)
    entries = b''
    for size, bmp in images:
        entries += struct.pack('<BBBBHHII', size % 256, size % 256, 0, 0, 1, 32, len(bmp), offset)
        offset += len(bmp)
    out += entries
    for _, bmp in images:
        out += bmp

    with open(filename, 'wb') as f:
        f.write(out)
    print(f"wrote {filename} ({len(out)} bytes)")


if __name__ == '__main__':
    make_ico('res/zoomout.ico', False)
    make_ico('res/zoomin.ico', True)
    print("done")
