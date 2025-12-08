#include <gb/gb.h>
#include <gb/metasprites.h>

#include "src/common.h"
#include "src/hud.h"
#include "src/ghosts.h"
#include "src/pacman.h"
#include "src/dots.h"
#include "src/util.h"

#include "graphics/GhostsEaten.h"
#include "graphics/GhostsScared.h"
#include "graphics/Dots.h"
#include "graphics/Ghosts.h"
#include "graphics/Pacman.h"
#include "graphics/PacmanEatenLeft.h"
#include "graphics/PacmanEatenRight.h"
#include "graphics/PacmanEatenUp.h"
#include "graphics/PacmanEatenDown.h"
#include "graphics/Numbers.h"
#include "graphics/Map.h"
#include "graphics/HighText.h"
#include "graphics/ReadyText.h"
#include "graphics/BlackSquare.h"
#include "graphics/OneUpText.h"
#include "graphics/TargetTiles.h"

uint8_t counter;

void UpdateCameraPosition(uint8_t dividor) {
    uint8_t playerX = pacman.column * 8 + Directions[pacman.direction].x * (pacman.move >> 4);
    uint8_t playerY = pacman.row * 8 + Directions[pacman.direction].y * (pacman.move >> 4);

    uint8_t sidebarWidth = 56;
    uint8_t heightRemainder = Map_HEIGHT - DEVICE_SCREEN_HEIGHT * 8;

    uint16_t targetCameraX = SCX_REG;
    uint16_t targetCameraY = SCY_REG;

    if (playerX < Map_WIDTH / 3) targetCameraX = 0;
    else if (playerX < (Map_WIDTH / 3) * 2) targetCameraX = ((playerX - Map_WIDTH / 3) * sidebarWidth) / (Map_WIDTH / 3);
    else targetCameraX = sidebarWidth;

    if (playerY < Map_HEIGHT / 3) targetCameraY = 0;
    else if (playerY < (Map_HEIGHT / 3) * 2) targetCameraY = ((playerY - Map_HEIGHT / 3) * heightRemainder) / (Map_HEIGHT / 3);
    else targetCameraY = heightRemainder;

    int16_t cameraXDiff = targetCameraX - SCX_REG;
    int16_t cameraYDiff = targetCameraY - SCY_REG;

    SCX_REG += cameraXDiff / dividor;
    SCY_REG += cameraYDiff / dividor;
}

uint8_t CheckBackgroundTileIsWalkable(int8_t nextColumn, int8_t nextRow) {
    if(nextColumn == 10 && nextRow == 11) return TRUE;

    if (nextColumn >= Map_WIDTH / 8 || nextColumn < 0) return TRUE;

    return get_bkg_tile_xy(nextColumn, nextRow) == blank ||
        get_bkg_tile_xy(nextColumn, nextRow) == DOTS_TILES_START ||
        get_bkg_tile_xy(nextColumn, nextRow) == DOTS_TILES_START + 1;
}

void SetupVRAM() {
    set_sprite_palette(0, 5, Pacman_palettes);
    set_bkg_palette(0, 5, Map_palettes);

    set_bkg_data(0, Map_TILE_COUNT, Map_tiles);
    set_bkg_data(DOTS_TILES_START, Dots_TILE_COUNT, Dots_tiles);
    set_bkg_data(NUMBERS_TILES_START, Numbers_TILE_COUNT, Numbers_tiles);
    set_bkg_data(READYTEXT_TILES_START, ReadyText_TILE_COUNT, ReadyText_tiles);
    set_bkg_data(ONEUPTEXT_TILES_START, OneUpText_TILE_COUNT, OneUpText_tiles);
    set_bkg_data(HIGHTEXT_TILES_START, HighText_TILE_COUNT, HighText_tiles);
    set_bkg_data(BLACKSQUARE_TILES_START, BlackSquare_TILE_COUNT, BlackSquare_tiles);

    set_sprite_data(GHOSTS_SPRITES_START, Pacman_TILE_COUNT, Ghosts_tiles);
    set_sprite_data(GHOSTS_SCARED_SPRITES_START, GhostsScared_TILE_COUNT, GhostsScared_tiles);
    set_sprite_data(GHOSTS_EATEN_SPRITES_START, GhostsEaten_TILE_COUNT, GhostsEaten_tiles);
    set_sprite_data(TARGET_TILES_SPRITES_START, TargetTiles_TILE_COUNT, TargetTiles_tiles);
    
    VBK_REG = 1; set_bkg_tiles(0, 0, 21, 27, Map_map_attributes);
    VBK_REG = 0; set_bkg_tiles(0, 0, 21, 27, Map_map);

    blank = get_bkg_tile_xy(10, 13);
}

void StartGameplay() {
    VBK_REG = 1; set_bkg_tiles(8, 15, 6, 1, ReadyText_map_attributes);
    VBK_REG = 0; set_bkg_based_tiles(8, 15, 6, 1, ReadyText_map, READYTEXT_TILES_START);

    delay(1000);

    VBK_REG = 1; set_bkg_tile_xy(10, 11, 0);
    VBK_REG = 0; set_bkg_tile_xy(10, 11, blank);

    set_bkg_tile_xy(9, 11, get_bkg_tile_xy(8, 9));
    set_bkg_tile_xy(11, 11, get_bkg_tile_xy(12, 9));

    VBK_REG = 1; fill_bkg_rect(8, 15, 6, 1, 1);
    VBK_REG = 0; fill_bkg_rect(8, 15, 6, 1, blank);

    twoFrameAnimator = 0;
    threeFrameAnimator = 0;
}

void SetupGameplay() {
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;

    DISPLAY_ON;
    SHOW_SPRITES;
    SPRITES_8x16;
    SHOW_BKG;
    SHOW_WIN;

    score = 0;

    SetupVRAM();
    SetupHUD();
    SetupDots();
    SetupPacman();
    SetupGhosts();

    UpdateScore();
    UpdateCameraPosition(1);
    DrawGhost(0);
    DrawGhost(1);
    DrawGhost(2);
    DrawGhost(3);
    DrawPacman();

    for (uint8_t i = 0; i < 21; i++) {
        for (uint8_t j = 0; j < 27; j++) {
            TileSideWalkability[i][j][DOWN] = CheckBackgroundTileIsWalkable(i, j + 1);
            TileSideWalkability[i][j][UP] = CheckBackgroundTileIsWalkable(i, j - 1);
            TileSideWalkability[i][j][RIGHT] = CheckBackgroundTileIsWalkable(i + 1, j);
            TileSideWalkability[i][j][LEFT] = CheckBackgroundTileIsWalkable(i - 1, j);
        }
    }
}

void UpdateInputs() {
    joypadCurrent = joypadPrevious;
    joypadCurrent = joypad();

    if ((joypadCurrent & J_SELECT) && !(joypadPrevious & J_SELECT)) {
        enableDebug = !enableDebug;
    }
}

void UpdateGlobalFrameCounters() {
    counter++;
    if (counter >= 5) {
        counter = 0;
        twoFrameAnimator++;
        threeFrameAnimator++;

        if (threeFrameAnimator > 2) {
            threeFrameAnimator = 0;
        }
        if (twoFrameAnimator > 1) {
            twoFrameAnimator = 0;
        }
    }
}

void BlinkLevelBlueAndWhite_Halting() {
    for (uint8_t i = 0; i < 10; i++) {
        if (i % 2 == 0) set_bkg_palette(0, 1, Map_palettes + 8);
        else set_bkg_palette(0, 1, Map_palettes);

        delay(250);
    }

    set_bkg_palette(0, 1, Map_palettes);
}

void HandleDeath() {
    if (!ghostsResetting) PacmanDeathAnimation_Halting();
    ghostsResetting = 1;

    if (ghostsReady) {
        SetupGameplay();
        StartGameplay();
    }
}

void HandleWin() {
    BlinkLevelBlueAndWhite_Halting();
    SetupGameplay();
    StartGameplay();
}

void main(void) {
    SetupGameplay();
    StartGameplay();

    while(TRUE) {
        UpdateInputs();
        
        UpdateGlobalFrameCounters();
        UpdateHUD();
        UpdateAllGhosts();
        UpdatePacman();
        HandleDotConsumption();
        UpdateCameraPosition(5);

        if (pacman.state == 0) {
            HandleDeath();
        }
        else if (dotsRemaining == 0) {
            HandleWin();
        }

        vsync();
    }
}
