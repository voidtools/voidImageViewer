#!/usr/bin/env python3
"""anomaly sample generator for voidImageViewer smoke testing.

the third audit round asked for real evidence that the viewer survives
hostile images: "tested" has to mean "actually opens without crashing on
a real machine". this generator produces the anomaly classes the audit
listed (truncated files, lying headers, zero delay animations, extreme
frame counts, broken chunk order, trailing garbage, empty shells) plus
healthy controls and one genuine over budget canvas.

usage:  python tests/make_anomaly_samples.py [output_dir]

the output directory defaults to tests/samples next to this script. run
the windows smoke test (tests/smoke_test.ps1) against the generated set.
the generator is standard library only and runs on any platform; the
smoke test itself must run on windows because that is where the viewer
lives.

sample inventory (37):
  01-05  truncated files
  06-11  lying ihdr dimensions
  12-13  zero delay animations
  14-16  extreme frame counts
  17-20  broken png chunk order
  21-23  trailing garbage
  24-27  empty or corrupted shells
  28-33  healthy controls
  34-37  boundary and format confusion
"""

import os
import struct
import sys
import zlib

PNG_SIG = b'\x89PNG\r\n\x1a\n'


# ---------------------------------------------------------------- png tools

def png_chunk(tag, data):
    return (struct.pack('>I', len(data)) + tag + data +
            struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff))


def png_rows(width, row_count, color):
    """filtered scanlines, plain filter 0, solid color. for lying
    headers the scanline width is capped: the hostile point is data
    that does not match the claimed dimensions, not a real canvas."""
    row_width = min(width, 4096)
    row = b'\x00' + bytes(color) * row_width
    return row * row_count


def make_png(claimed_w, claimed_h, real_rows, color=(0x33, 0x66, 0x99),
             bit_depth=8, color_type=2):
    ihdr = struct.pack('>IIBBBBB', claimed_w, claimed_h, bit_depth,
                       color_type, 0, 0, 0)
    body = b''
    if real_rows > 0:
        comp = zlib.compressobj(1)
        body = comp.compress(png_rows(claimed_w, real_rows, color))
        body += comp.flush()
    out = PNG_SIG
    out += png_chunk(b'IHDR', ihdr)
    if body:
        out += png_chunk(b'IDAT', body)
    out += png_chunk(b'IEND', b'')
    return out


def full_png(width, height, color=(0x40, 0x40, 0x48)):
    """a complete, honest png: every declared row is present."""
    ihdr = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    comp = zlib.compressobj(1)
    row = b'\x00' + bytes(color) * width
    body = bytearray()
    for _ in range(height):
        body += comp.compress(row)
    body += comp.flush()
    return (PNG_SIG + png_chunk(b'IHDR', ihdr) +
            png_chunk(b'IDAT', bytes(body)) + png_chunk(b'IEND', b''))


# ---------------------------------------------------------------- gif tools

def lzw_pack(codes, min_code_size):
    """pack lzw codes at the initial code width (min + 1)."""
    width = min_code_size + 1
    acc = 0
    bits = 0
    out = bytearray()
    for code in codes:
        acc |= code << bits
        bits += width
        while bits >= 8:
            out.append(acc & 0xff)
            acc >>= 8
            bits -= 8
    if bits:
        out.append(acc & 0xff)
    return bytes(out)


def gif_frame(delay_cs, left=0, top=0):
    """one 1x1 frame: graphic control + image descriptor + tiny lzw data."""
    gce = (b'\x21\xf9\x04\x00' + struct.pack('<H', delay_cs) +
           b'\x00\x00')
    desc = b'\x2c' + struct.pack('<HHHH', left, top, 1, 1) + b'\x00'
    codes = lzw_pack([4, 0, 5], 2)      # clear, pixel 0, eoi at width 3
    data = bytes([2, len(codes)]) + codes + b'\x00'
    return gce + desc + data


def make_gif(width, height, delays):
    out = b'GIF89a' + struct.pack('<HHBBB', width, height, 0x70, 0, 0)
    out += bytes([0x00, 0x00, 0x00, 0xff, 0xff, 0xff])   # 2 color table
    if len(delays) > 1:
        out += (b'\x21\xff\x0bNETSCAPE2.0\x03\x01' +
                struct.pack('<H', 0) + b'\x00')
    for i, d in enumerate(delays):
        out += gif_frame(d)
    out += b'\x3b'
    return out


# ---------------------------------------------------------------- bmp tools

def make_bmp(width, height, color=(0x50, 0x60, 0x70)):
    row = (width * 3 + 3) & ~3
    size = 54 + row * height
    out = b'BM' + struct.pack('<IHHI', size, 0, 0, 54)
    out += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0,
                       row * height, 2835, 2835, 0, 0)
    pad = b'\x00' * (row - width * 3)
    out += (bytes(reversed(color)) + pad) * height
    return out


# ---------------------------------------------------------------- the set

def build(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    files = []

    def emit(name, data):
        path = os.path.join(out_dir, name)
        with open(path, 'wb') as f:
            f.write(data)
        files.append((name, data))

    good_png = make_png(100, 100, 100)
    good_gif = make_gif(64, 64, [10])

    # 01-05 truncated files
    emit('01_png_truncated_half.png', good_png[:len(good_png) // 2])
    emit('02_png_truncated_tail10.png', good_png[:-10])
    emit('03_png_truncated_header.png', good_png[:20])
    emit('04_gif_truncated_half.gif',
         make_gif(64, 64, [10, 10, 10])[:30])
    emit('05_png_signature_only.png', PNG_SIG)

    # 06-11 lying ihdr dimensions
    emit('06_png_claims_20000x20000.png', make_png(20000, 20000, 1))
    emit('07_png_claims_0x0.png', make_png(0, 0, 1))
    emit('08_png_claims_maxdim_x1.png', make_png(0xffffffff, 1, 1))
    emit('09_png_claims_65535x65535.png', make_png(65535, 65535, 1))
    emit('10_png_claims_10001x10001.png', make_png(10001, 10001, 2))
    emit('11_png_claims_1x_maxdim.png', make_png(1, 0xffffffff, 1))

    # 12-13 zero delay animations
    emit('12_gif_anim_all_zero_delay.gif', make_gif(64, 64, [0, 0, 0]))
    emit('13_gif_anim_zero_then_100.gif', make_gif(64, 64, [0, 0, 100]))

    # 14-16 extreme frame counts
    emit('14_gif_anim_300_frames.gif', make_gif(64, 64, [2] * 300))
    emit('15_gif_anim_300_zero_delay.gif', make_gif(64, 64, [0] * 300))
    emit('16_gif_anim_1200_frames.gif', make_gif(32, 32, [1] * 1200))

    # 17-20 broken png chunk order
    ihdr = png_chunk(b'IHDR', struct.pack('>IIBBBBB', 64, 64, 8, 2, 0, 0, 0))
    comp = zlib.compressobj(1)
    idat_body = comp.compress(png_rows(64, 64, (0x10, 0x20, 0x30)))
    idat_body += comp.flush()
    idat = png_chunk(b'IDAT', idat_body)
    iend = png_chunk(b'IEND', b'')
    emit('17_png_idat_before_ihdr.png', PNG_SIG + idat + ihdr + iend)
    emit('18_png_missing_iend.png', PNG_SIG + ihdr + idat)
    emit('19_png_ihdr_then_iend_only.png', PNG_SIG + ihdr + iend)
    emit('20_png_iend_before_idat.png', PNG_SIG + ihdr + iend + idat)

    # 21-23 trailing garbage
    emit('21_png_plus_64kb_zeros.png', good_png + b'\x00' * 65536)
    emit('22_png_plus_64kb_random.png',
         good_png + bytes(range(256)) * 256)
    emit('23_gif_plus_garbage.gif', good_gif + b'GARBAGEGARBAGE' * 64)

    # 24-27 empty or corrupted shells
    emit('24_empty_zero_bytes.png', b'')
    emit('25_gif_header_only_6bytes.gif', b'GIF89a')
    emit('26_png_bad_signature_byte.png',
         b'\x88' + good_png[1:])
    emit('27_text_with_png_name.png',
         b'this is not an image at all, only text pretending')

    # 28-33 healthy controls
    emit('28_control_png_100x100.png', good_png)
    emit('29_control_png_4000x3000.png', full_png(4000, 3000))
    emit('30_control_gif_single_frame.gif', good_gif)
    emit('31_control_gif_anim_normal.gif',
         make_gif(64, 64, [10, 10, 10]))
    emit('32_control_bmp_24bpp.bmp', make_bmp(96, 64))
    emit('33_control_png_16bit_depth.png',
         make_png(32, 32, 8, bit_depth=16))

    # 34-37 boundary and format confusion
    # 34-35: the 32-bit ceiling story. 1.0.04 splits the ceiling by
    # pointer width, so 35 now doubles as the x64 acceptance boundary
    # (110 mp loads on x64, still refused on 32-bit) and 38 pins the
    # x64 refusal point above the 400 mp ceiling.
    emit('34_png_exactly_100mp_boundary.png', make_png(10000, 10000, 1))
    emit('35_png_over_budget_110mp.png', full_png(10500, 10500))
    emit('36_gif_data_with_bmp_name.bmp',
         b'GIF89a' + make_gif(8, 8, [10])[6:])
    emit('37_bmp_dib_size_lies.bmp',
         make_bmp(96, 64)[:2] +
         struct.pack('<IHHI', 999999, 0, 0, 54) +
         make_bmp(96, 64)[14:])
    emit('38_png_over_x64_budget_625mp.png', make_png(25000, 25000, 1))

    return files


# ------------------------------------------------------------- self check

def png_ihdr(data):
    if len(data) < 24 or data[:8] != PNG_SIG:
        return None
    if data[12:16] != b'IHDR':
        return None
    return struct.unpack('>II', data[16:24])


def self_check(files, out_dir):
    failures = []
    by_name = {n: d for n, d in files}

    def expect(cond, msg):
        if not cond:
            failures.append(msg)

    expect(len(files) == 38, 'expected 38 samples, produced %d' % len(files))

    # png header claims
    for name, wh in [
        ('06_png_claims_20000x20000.png', (20000, 20000)),
        ('07_png_claims_0x0.png', (0, 0)),
        ('08_png_claims_maxdim_x1.png', (0xffffffff, 1)),
        ('09_png_claims_65535x65535.png', (65535, 65535)),
        ('10_png_claims_10001x10001.png', (10001, 10001)),
        ('34_png_exactly_100mp_boundary.png', (10000, 10000)),
        ('35_png_over_budget_110mp.png', (10500, 10500)),
        ('38_png_over_x64_budget_625mp.png', (25000, 25000)),
        ('28_control_png_100x100.png', (100, 100)),
        ('29_control_png_4000x3000.png', (4000, 3000)),
    ]:
        got = png_ihdr(by_name.get(name, b''))
        expect(got == wh, '%s ihdr %r != expected %r' % (name, got, wh))

    # the over budget canvas really is over 100 mp and fully encoded
    w, h = png_ihdr(by_name['35_png_over_budget_110mp.png'])
    expect(w * h == 10500 * 10500 and w * h > 100000000,
           'over budget sample is not over the budget')
    expect(len(by_name['35_png_over_budget_110mp.png']) < 2000000,
           'over budget sample grew beyond its deflate shape (the 258 byte'
           ' match limit keeps a solid 10500 wide row near 140 bytes)')
    expect(w * h < 400000000,
           '110 mp sample now doubles as the x64 acceptance boundary, it'
           ' must stay under the 400 mp x64 ceiling')
    w38, h38 = png_ihdr(by_name['38_png_over_x64_budget_625mp.png'])
    expect(w38 * h38 == 625000000 and w38 * h38 > 400000000,
           'x64 over budget sample is not over the 400 mp ceiling')

    # gif structure
    for name in ['12_gif_anim_all_zero_delay.gif',
                 '31_control_gif_anim_normal.gif']:
        expect(by_name[name][:6] == b'GIF89a', name + ' missing gif header')
    # every frame sits at (0,0) so image separators are the only 0x2c
    expect(by_name['14_gif_anim_300_frames.gif'].count(b'\x2c') == 300,
           '300 frame sample does not carry 300 frames')
    expect(by_name['16_gif_anim_1200_frames.gif'].count(b'\x2c') == 1200,
           '1200 frame sample does not carry 1200 frames')

    # shells and truncations
    expect(by_name['24_empty_zero_bytes.png'] == b'', 'empty sample not empty')
    expect(len(by_name['05_png_signature_only.png']) == 8,
           'signature only sample has extra bytes')
    expect(len(by_name['03_png_truncated_header.png']) == 20,
           'header truncation has wrong length')
    expect(by_name['26_png_bad_signature_byte.png'][0:1] == b'\x88',
           'bad signature sample kept the good signature')

    # controls decode: full png stream must be consistent
    for name in ['29_control_png_4000x3000.png', '35_png_over_budget_110mp.png']:
        data = by_name[name]
        expect(data[-12:] == png_chunk(b'IEND', b''), name + ' missing iend')
        expect(b'IDAT' in data, name + ' missing idat')

    # bmp controls
    expect(by_name['32_control_bmp_24bpp.bmp'][:2] == b'BM',
           'bmp control missing magic')
    expect(b'\x36\x00\x00\x00' not in by_name['36_gif_data_with_bmp_name.bmp'][:6],
           'gif-in-bmp sample should not look like a bmp')

    return failures


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'samples')
    files = build(out_dir)
    failures = self_check(files, out_dir)
    print('generated %d samples in %s' % (len(files), out_dir))
    for name, data in files:
        print('  %-40s %8d bytes' % (name, len(data)))
    if failures:
        for f in failures:
            print('SELF CHECK FAIL: ' + f)
        sys.exit(1)
    print('SELF CHECK PASS: 38 samples, all headers verified')
    sys.exit(0)


if __name__ == '__main__':
    main()
