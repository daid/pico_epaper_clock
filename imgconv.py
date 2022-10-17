import PIL.Image

img = PIL.Image.open("Design1.png")
img = bytearray(img.convert("L").getdata())
start_point = {}
def getpixel(x, y):
    if 0 <= x < 250 and 0 <= y < 122:
        return img[x + y * 250]
    return 255
def fill(x, y, zone_id):
    if 0 <= x < 250 and 0 <= y < 122 and img[x + y * 250] == 0:
        cnt = 0
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                if getpixel(x + dx, y + dy) in {0, zone_id}:
                    cnt += 1
        if cnt < 3:
            start_point[zone_id-1] = (x, y)
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
        if img[x + y * 250] == 38:
            img[x + y * 250] = 0
            fill(x, y, zone_total + 1)
            zone_total += 1
print("Zones: ", zone_total)

with open("img.h", "wt") as f:
    f.write("""
struct PointDef {
    uint8_t x, y;
};
struct ZoneDef {
    int offset;
    int width;
    int height;
    const uint8_t* data;
    const PointDef* line;
    int line_length;
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
        
        x, y = start_point[zone_id]
        px, py = x, y
        zone_line = [(x, y)]
        while True:
            options = []
            for dx in range(-1, 2):
                for dy in range(-1, 2):
                    if (dx != 0 or dy != 0) and getpixel(x + dx, y + dy) == zone_id + 1:
                        if (x + dx, y + dy) not in zone_line:
                            options.append((x + dx, y + dy))
            if len(options) != 1:
                break
            px, py = x, y
            x, y = options[0]
            zone_line.append((x, y))
        f.write(f"const PointDef ZONE_LINE_{zone_id}[] = {{\n")
        for x, y in zone_line:
            f.write(f"{{{x}, {y}}},")
        f.write("};\n");
    
    f.write("const ZoneDef ZONES[] = {\n")
    for zone_id, (x, y, w, h) in enumerate(zone_dims):
        print(zone_id, x, y, w, h)
        offset = x * 16 + y
        f.write(f"  {{{offset}, {w}, {h}, ZONE_DATA_{zone_id}, ZONE_LINE_{zone_id}, sizeof(ZONE_LINE_{zone_id}) / sizeof(PointDef)}},\n")
    f.write("};\n");
