#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MARIO_WIDTH 32
#define MARIO_HEIGHT 32
#define GRAVITY 0.5
#define JUMP_STRENGTH -10
#define MAX_SPEED 5
#define ACCELERATION 0.2
#define FRICTION 0.1

typedef struct {
    float x, y;
    float velocityX, velocityY;
    bool jumping;
    bool facingLeft;
    int frame;
    int frameCounter;
    SDL_Texture* texture;
} Mario;

typedef struct {
    float x, y, width, height;
} Platform;

#define PLATFORM_COUNT 3
Platform platforms[PLATFORM_COUNT] = {
    {100, 400, 200, 20},
    {400, 300, 150, 20},
    {600, 500, 180, 20}
};

Mario mario = {100, 300, 0, 0, false, false, 0, 0, NULL};
SDL_Renderer* renderer;

SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Failed to load image %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool check_platform_collision(Mario* m, Platform* p) {
    return (m->x + MARIO_WIDTH > p->x &&
            m->x < p->x + p->width &&
            m->y + MARIO_HEIGHT > p->y &&
            m->y + MARIO_HEIGHT <= p->y + 10);
}

void update_mario() {
    mario.velocityY += GRAVITY;

    if (mario.velocityX > 0) {
        mario.velocityX -= FRICTION;
        if (mario.velocityX < 0) mario.velocityX = 0;
    } else if (mario.velocityX < 0) {
        mario.velocityX += FRICTION;
        if (mario.velocityX > 0) mario.velocityX = 0;
    }

    mario.x += mario.velocityX;
    mario.y += mario.velocityY;

    bool onPlatform = false;
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        if (check_platform_collision(&mario, &platforms[i])) {
            mario.y = platforms[i].y - MARIO_HEIGHT;
            mario.velocityY = 0;
            mario.jumping = false;
            onPlatform = true;
            break;
        }
    }

    if (mario.y >= SCREEN_HEIGHT - MARIO_HEIGHT) {
        mario.y = SCREEN_HEIGHT - MARIO_HEIGHT;
        mario.velocityY = 0;
        mario.jumping = false;
    } else if (!onPlatform) {
        mario.jumping = true;
    }
}

void handle_input(SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                mario.velocityX -= ACCELERATION;
                if (mario.velocityX < -MAX_SPEED) mario.velocityX = -MAX_SPEED;
                mario.facingLeft = true;
                break;
            case SDLK_RIGHT:
                mario.velocityX += ACCELERATION;
                if (mario.velocityX > MAX_SPEED) mario.velocityX = MAX_SPEED;
                mario.facingLeft = false;
                break;
            case SDLK_SPACE:
                if (!mario.jumping) {
                    mario.jumping = true;
                    mario.velocityY = JUMP_STRENGTH;
                }
                break;
        }
    }
}

void render_mario() {
    SDL_Rect srcRect = { mario.frame * MARIO_WIDTH, 0, MARIO_WIDTH, MARIO_HEIGHT };
    SDL_Rect destRect = { (int)mario.x, (int)mario.y, MARIO_WIDTH, MARIO_HEIGHT };
    
    SDL_RendererFlip flip = mario.facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;  
    SDL_RenderCopyEx(renderer, mario.texture, &srcRect, &destRect, 0, NULL, flip);
}

void render_platforms() {
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        SDL_Rect platformRect = { (int)platforms[i].x, (int)platforms[i].y, (int)platforms[i].width, (int)platforms[i].height };
        SDL_RenderFillRect(renderer, &platformRect);
    }
}

void game_loop() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Mario C", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    mario.texture = load_texture("mario_sprites.png", renderer);
    if (!mario.texture) {
        printf("Failed to load sprite sheet.\n");
        return;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            handle_input(event);
        }

        update_mario();

        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_RenderClear(renderer);
        
        render_platforms();
        render_mario();
        
        SDL_RenderPresent(renderer);
        
        SDL_Delay(16);
    }

    SDL_DestroyTexture(mario.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main() {
    game_loop();
    return 0;
}
