#include <Adafruit_NeoPixel.h>
#include <Wire.h>

const uint8_t NUM_LEDS = 100;

const uint8_t MAX_LIGHTNESS = 10;
const uint8_t MINIMUM_LIGHTNESS_FACTOR_RATIO = 10;

const uint8_t NUM_FOOD = 10;
const uint8_t MAX_SNAKE_LEN = NUM_FOOD;
const uint16_t PERIOD_LENGTH = 100;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, D5, NEO_RGB + NEO_KHZ800);

enum class direction { left, right };

uint8_t target_idx = 0;

typedef struct _color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct _food {
    int16_t idx;
    color_t color;
} food_t;

int16_t *left_tail_idx;
int16_t *right_tail_idx;

food_t food[NUM_FOOD] = { 0 };

typedef struct _snake {
    uint8_t len;
    direction dir;
    food_t food[MAX_SNAKE_LEN];
} snake_t;

snake_t snake;

void setup() {
    // put your setup code here, to run once:
    strip.begin();
    uint8_t r = MAX_LIGHTNESS, g = MAX_LIGHTNESS, b = MAX_LIGHTNESS;
    for (int x = 0; x <= NUM_LEDS; x++) {
        strip.setPixelColor(x, r, g, b);
    }
    strip.show();
    delay(1000);
    clearSky();
    strip.show();
}

void loop() {
    if (random(0, 2)) {
        snake.food[0] = { .idx = 0,  .color={.r = MAX_LIGHTNESS, .g = MAX_LIGHTNESS, .b = MAX_LIGHTNESS} };
        snake.dir = direction::right;
    } else {
        snake.food[0] = { .idx = NUM_LEDS - 1,  .color={.r = MAX_LIGHTNESS, .g = MAX_LIGHTNESS, .b = MAX_LIGHTNESS} };
        snake.dir = direction::left;
    }
    left_tail_idx = &snake.food[0].idx;
    right_tail_idx = &snake.food[0].idx;
    snake.len = 1;
    generateFood();
    placeFood();
    findTarget();
    while (snakeMove() && placeFood()) delay(PERIOD_LENGTH);
}

void generateFood() {
    uint8_t *random_idcs = randomNumsWithoutDuplictates(NUM_FOOD, NUM_LEDS /* -1 +1 */);
    for (int i = 0; i < NUM_FOOD; i++) {
        food[i] = { .idx = random_idcs[i], .color = { .r = random(MAX_LIGHTNESS/MINIMUM_LIGHTNESS_FACTOR_RATIO, MAX_LIGHTNESS + 1), .g = random(MAX_LIGHTNESS/MINIMUM_LIGHTNESS_FACTOR_RATIO, MAX_LIGHTNESS + 1), .b = random(MAX_LIGHTNESS/MINIMUM_LIGHTNESS_FACTOR_RATIO, MAX_LIGHTNESS + 1)} };
    }
}

uint8 *randomNumsWithoutDuplictates(uint8_t num, uint8_t max) {
    uint8_t *randoms = new uint8_t[num];
    for (int i = 0; i < num; i++) {
        randoms[i] = random(0, max);
        for (int j = 0; j < i; j++) {
            if (randoms[i] == randoms[j]) {
                i--;
                break;
            }
        }
    }
    return randoms;
}

int placeFood() {
    for (int i = 0; i < NUM_FOOD; i++) {
        if (food[i].color.r != 0) {
            strip.setPixelColor(food[i].idx, food[i].color.r, food[i].color.g, food[i].color.b);
        }
    }
    strip.show();
    return true;
}

boolean snakeMove() {
    if (snake.len == MAX_SNAKE_LEN + 1) {
        return false;
    }

    // choose next target if fruit was eaten
    if (*left_tail_idx <= food[target_idx].idx && food[target_idx].idx <= *right_tail_idx) {
        snake.len++;
        snake.food[snake.len - 1].color.r = food[target_idx].color.r;
        snake.food[snake.len - 1].color.g = food[target_idx].color.g;
        snake.food[snake.len - 1].color.b = food[target_idx].color.b;

        switch (snake.dir) {
        case direction::left:
            snake.food[snake.len-1].idx = *left_tail_idx - 1;
            left_tail_idx = &snake.food[snake.len-1].idx;
            break;
        case direction::right:
            snake.food[snake.len-1].idx = *right_tail_idx + 1;
            right_tail_idx = &snake.food[snake.len-1].idx;
            break;
        }

        food[target_idx].color.r = 0;
        food[target_idx].color.g = 0;
        food[target_idx].color.b = 0;

        findTarget();
    }

    // move snake and it's tail made out of food
    for (int i = 0; i < snake.len; i++) {
        strip.setPixelColor(snake.food[i].idx, 0);
    }
    for (int i = 0; i < snake.len; i++) {
        switch (snake.dir) {
        case direction::right:
            snake.food[i].idx++;
            if (snake.food[i].idx >= 0 && snake.food[i].idx < NUM_LEDS) {
                strip.setPixelColor(snake.food[i].idx, snake.food[i].color.r, snake.food[i].color.g, snake.food[i].color.b);
            }
            break;
        case direction::left:
            snake.food[i].idx--;
            if (snake.food[i].idx >= 0 && snake.food[i].idx < NUM_LEDS) {
                strip.setPixelColor(snake.food[i].idx, snake.food[i].color.r, snake.food[i].color.g, snake.food[i].color.b);
            }
            break;
        }
    }
    strip.show();
    return true;
}

void findTarget() {
    while (true) {
        target_idx = (target_idx + 1) % NUM_FOOD;
        if (food[target_idx].color.r != 0) {
            // -1 and +1 are necessary because of the edge case where food is within the snake's tail but as the snake is moving forwards it will pass the food and not be able to eat it immediately, thus not changing the direction
            if (snake.food[0].idx - 1 < food[target_idx].idx) {
                snake.dir = direction::right;
            } else if (food[target_idx].idx < snake.food[snake.len - 1].idx + 1) {
                snake.dir = direction::left;
            }
            break;
        }
    }
}

void clearSky(void) {
    for (int x = 0; x <= NUM_LEDS; x++) {
        strip.setPixelColor(x, 0);
    }
    strip.show();
}
