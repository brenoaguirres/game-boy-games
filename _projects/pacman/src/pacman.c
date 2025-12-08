#include <gb/gb.h>
#include <gb/metasprites.h>
#include "util.h"
#include "character.h"
#include "pacman.h"
#include "common.h"
#include "../graphics/Pacman.h"
#include "../graphics/Map.h"
#include "../graphics/Dots.h"
#include "../graphics/PacmanEatenLeft.h"
#include "../graphics/PacmanEatenRight.h"
#include "../graphics/PacmanEatenUp.h"
#include "../graphics/PacmanEatenDown.h"



void SetupPacman(){
    set_sprite_data(0, Pacman_TILE_COUNT, Pacman_tiles);

    pacman.column = 10;
    pacman.move = 0;
    pacman.row = 20;
    pacman.state = 1;
    pacman.direction = RIGHT;
}


void UpdatePacman(){
    if (joypadCurrent & (J_UP | J_DOWN | J_LEFT | J_RIGHT)) {
        uint8_t dir = UP;
        if (joypadCurrent & J_DOWN) dir = DOWN;
        else if (joypadCurrent & J_LEFT) dir = LEFT;
        else if (joypadCurrent & J_RIGHT) dir = RIGHT;

        TryChangeDirection(&pacman, dir);
    }

    if (pacman.state != 0) {
        MoveForward(&pacman, 10);
        DrawPacman();
    }
}

void DrawPacman(){
    DrawCharacter(&pacman, Pacman_metasprites[pacman.direction * 3 + threeFrameAnimator], 0, 0);
}


void PacmanDeathAnimation_Halting(){
    NR10_REG = 0x7C;
    NR11_REG = 0xCB;
    NR12_REG = 0x67;
    NR13_REG = 0xA6;
    NR14_REG = 0x86;

    switch(pacman.direction) {
        case DOWN: set_sprite_data(0, PacmanEatenDown_TILE_COUNT, PacmanEatenDown_tiles); break;
        case UP: set_sprite_data(0, PacmanEatenUp_TILE_COUNT, PacmanEatenUp_tiles); break;
        case LEFT: set_sprite_data(0, PacmanEatenLeft_TILE_COUNT, PacmanEatenLeft_tiles); break;
        case RIGHT: set_sprite_data(0, PacmanEatenRight_TILE_COUNT, PacmanEatenRight_tiles); break;
    }

    move_sprite(0, 0, 0);
    move_sprite(1, 0, 0);

    for (uint8_t i = 0; i < 8; i++) {
        switch (pacman.direction) {
            case DOWN: DrawCharacter(&pacman, PacmanEatenDown_metasprites[i], 0, 0); break;
            case UP: DrawCharacter(&pacman, PacmanEatenUp_metasprites[i], 0, 0); break;
            case LEFT: DrawCharacter(&pacman, PacmanEatenLeft_metasprites[i], 0, 0); break;
            case RIGHT: DrawCharacter(&pacman, PacmanEatenRight_metasprites[i], 0, 0); break;
        }

        vsync();
        vsync();
        vsync();
    }

    move_sprite(0, 0, 0);
    move_sprite(1, 0, 0);

    pacman.column = 10;
    pacman.row = 20;
    pacman.move = 0;
    pacman.direction = RIGHT;
}
