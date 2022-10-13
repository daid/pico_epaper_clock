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
    for zone_id in range(zone_total):
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
                result.append(value)
        f.write("const uint8_t ZONE_%d[] = {\n" % (zone_id))
        for n in result:
            f.write(f"0x{n:02x},")
        f.write("};\n");
    f.write("const uint8_t* ZONES[] = {")
    for zone_id in range(zone_total):
        f.write("ZONE_%d, " % (zone_id))
    f.write("};\n");
