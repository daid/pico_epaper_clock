import PIL.Image

img = PIL.Image.open("Design1.png")
img = bytearray(img.convert("L").getdata())
def fill(x, y, zone_id):
    if 0 <= x < 250 and 0 <= y < 122 and img[x + y * 250] == 0:
        img[x + y * 250] = zone_id
        fill(x+1, y+1, zone_id)
        fill(x+0, y+1, zone_id)
        fill(x-1, y+1, zone_id)
        fill(x+1, y+0, zone_id)
        fill(x-1, y+0, zone_id)
        fill(x+1, y-1, zone_id)
        fill(x+0, y-1, zone_id)
        fill(x-1, y-1, zone_id)
zone_total = 0
for x in range(250):
    for y in range(122):
        if img[x + y * 250] == 0:
            fill(x, y, zone_total + 1)
            zone_total += 1
print("Zones: ", zone_total)

with open("img.h", "wt") as f:
    f.write("""
struct ZoneDef {
    int offset;
    int width;
    int height;
    const uint8_t* data;
};
""")
    zone_dims = []
    for zone_id in range(zone_total):
        xmin = 250
        xmax = 0
        ymin = 122
        ymax = 0
        result = []
        for x in range(250):
            for row in range(16):
                value = 0
                for y in range(8):
                    try:
                        pix = img[x + (row * 8 + y) * 250]
                    except IndexError:
                        pix = 0
                    if pix != zone_id + 1:
                        value |= 0x80 >> y
                    else:
                        xmin = min(x, xmin)
                        xmax = max(x, xmax)
                        ymin = min(row, ymin)
                        ymax = max(row, ymax)
                result.append(value)
        w = xmax - xmin + 1
        h = ymax - ymin + 1
        zone_dims.append((xmin, ymin, w, h))
        f.write(f"const uint8_t ZONE_DATA_{zone_id}[] = {{\n")
        for x in range(xmin, xmax + 1):
            for y in range(ymin, ymax + 1):
                f.write(f"0x{result[x*16+y]:02x},")
            f.write("\n")
        f.write("};\n");
    
    f.write("const ZoneDef ZONES[] = {\n")
    for zone_id, (x, y, w, h) in enumerate(zone_dims):
        print(zone_id, x, y, w, h)
        offset = x * 16 + y
        f.write(f"  {{{offset}, {w}, {h}, ZONE_DATA_{zone_id}}},\n")
    f.write("};\n");
