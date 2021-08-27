#include "infinimap.hpp"

using namespace blit;

Vec2 offset(0, 0);
uint32_t pixel_size = 1;

// https://www.youtube.com/watch?v=LWFzPP8ZbdU
uint32_t hash(Point position, uint32_t seed) {
    constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
    constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
    constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;

    uint32_t mangled = position.x + position.y * 198491317;
    mangled *= BIT_NOISE1;
    mangled += seed;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);

    return mangled;
    //return Pen(uint8_t(mangled & 0xff), uint8_t(mangled & 0xff), uint8_t(mangled & 0xff));
    //return Pen(uint8_t(mangled & 0xff), uint8_t((mangled >> 8) & 0xff), uint8_t((mangled >> 16) & 0xff));
}

Pen hash_2d(Point point) {
    constexpr int PRIME = 198491317;
    std::hash<int64_t> hash_xy;
    int64_t p = point.x + (int64_t)point.y * PRIME;
    size_t xy = hash_xy(p * PRIME);
    //size_t xy = hash_xy(((int64_t)point.y * YPRIME) * ((int64_t)point.x * XPRIME));
    return Pen(uint8_t(xy & 0xff), uint8_t((xy >> 8) & 0xff), uint8_t((xy >> 16) & 0xff));
}

void init() {
    set_screen_mode(ScreenMode::hires);
    screen.sprites = Surface::load(dingbads);
}

void render(uint32_t time) {
    uint32_t seed = 123456;
    uint32_t t_start = now_us();
    uint32_t ms_start = now();
    screen.pen = Pen(255, 255, 255);
    screen.clear();

    Point p(0, 0);
    Vec2 camera_offset(screen.bounds.w, screen.bounds.h);
    camera_offset /= 2;
    camera_offset /= pixel_size;
    camera_offset = offset - camera_offset;
    std::vector<Point> points;
    for(p.y = 0; p.y <= screen.bounds.h / pixel_size; ++p.y) {
        for(p.x = 0; p.x <= screen.bounds.w / pixel_size; ++p.x) {
            uint32_t r = hash(p + camera_offset, seed);
            uint8_t c = (r & 0xff);
            if(p + camera_offset == Vec2(0, 0)) {
                screen.pen = Pen(255, 0, 0);
            }
            else {
                screen.pen = Pen(c >> 4, c >> 4, c >> 4);
            }
            if(c == 64) {
                points.push_back(p * pixel_size);
            }
            screen.rectangle(Rect(p * pixel_size, Size(pixel_size, pixel_size)));
        }
    }
    for(auto &point : points) {
        screen.stretch_blit(screen.sprites, Rect(0, 0, 8, 8), Rect(point, Size(pixel_size * 8, pixel_size * 8)));
    }
    points.clear();
    uint32_t t_end = now_us();
    uint32_t ms_end = now();
    screen.pen = Pen(255, 0, 0);
    screen.text(std::to_string(t_end - t_start), minimal_font, Point(0, 0));
    screen.pen = Pen(0, 255, 0);
    screen.text(std::to_string(int(offset.x)) + ":" + std::to_string(int(offset.y)), minimal_font, Point(50, 0));

    // draw FPS meter
    screen.mask = nullptr;
    screen.pen = Pen(255, 0, 0);
    for (unsigned int i = 0; i < (ms_end - ms_start); i++) {
        screen.pen = Pen(i * 5, 255 - (i * 5), 0);
        screen.rectangle(Rect(i * 6 + 2, screen.bounds.h - 6, 4, 4));
    }
}

void update(uint32_t time) {
    if(buttons & Button::DPAD_LEFT) {
        offset.x -= 1.0f / pixel_size;
    }
    if(buttons & Button::DPAD_RIGHT) {
        offset.x += 1.0f / pixel_size;
    }
    if(buttons & Button::DPAD_UP) {
        offset.y -= 1.0f / pixel_size;
    }
    if(buttons & Button::DPAD_DOWN) {
        offset.y += 1.0f / pixel_size;
    }
    if(buttons.pressed & Button::A) {
        pixel_size++;
    }
    if(buttons.pressed & Button::B) {
        if(pixel_size > 1) {
            pixel_size--;
        }
    }
}