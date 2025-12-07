#include <gb/gb.h>
#include <gb/metasprites.h>
#include "common.h"
#include "character.h"
#include "util.h"
#include "graphics/Map.h"
#include "graphics/Dots.h"

uint8_t directionsCharacterCanMoveIn[4] = {0,0,0,0},numberOfDirectionsCharacterCanMoveIn=0;

void GetDirectionsCharacterCanMoveIn(Character *character,uint8_t allowReverse){
    numberOfDirectionsCharacterCanMoveIn = 0;

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t notReversingOrIsAllowed = (character->direction != reverseDirections[i] || allowReverse);

        if (CanCharacterMoveInThisDirection(character, i) && notReversingOrIsAllowed) 
            directionsCharacterCanMoveIn[numberOfDirectionsCharacterCanMoveIn++] = i;
    }
}

uint8_t CanCharacterMoveInThisDirection(Character *character, uint8_t direction){
    int8_t nextColumn = character->column + Directions[direction].x;
    int8_t nextRow = character->row + Directions[direction].y;

        if (nextColumn >= Map_WIDTH / 8 || nextColumn < 0) {
            if (direction == UP || direction == DOWN) return FALSE;

            return TRUE;
        }

    return TileSideWalkability[character->column][character->row][direction];
}

void TryChangeDirection(Character *character, uint8_t nextDirection){
    if (character->direction == nextDirection) return;

    uint8_t aligned = character->move == 0 || character->move >= 128;

    uint8_t changingAxis = 
        (character->direction == LEFT && (nextDirection == UP || nextDirection == DOWN)) ||
        (character->direction == RIGHT && (nextDirection == UP || nextDirection == DOWN)) ||
        (character->direction == UP && (nextDirection == LEFT || nextDirection == RIGHT)) ||
        (character->direction == DOWN && (nextDirection == LEFT || nextDirection == RIGHT));

    uint8_t canMoveThisWay = CanCharacterMoveInThisDirection(character, nextDirection);

    if (canMoveThisWay) {
        if (changingAxis && aligned) {
            character->direction = nextDirection;
            character->move = 0;
        }
        else if (!changingAxis) {
            character->column += Directions[character->direction].x;
            character->row += Directions[character->direction].y;

            character->direction = nextDirection;

            character->move = (128 - character->move);
        }
    }
}

uint8_t MoveForward(Character *character, uint8_t speed){
    if (CanCharacterMoveInThisDirection(character, character->direction)) {
        character->move += speed;

        if (character->move >= 128) {
            character->move = 0;

            if (character->column == 0 && character->direction == LEFT) {
                character->column = Map_WIDTH / 8 + 1;    
            }
            else if (character->column == Map_WIDTH / 8 && character->direction == RIGHT) {
                character->column = -1;
            }
            else {
                character->column += Directions[character->direction].x;
                character->row += Directions[character->direction].y;
            }

            return 1;
        }
    }
    return 0;
}

void DrawCharacter(Character *character, metasprite_t const *metasprites, uint8_t baseSprite, uint8_t baseTile){
    uint16_t screenX = (character->column * 8 + Directions[character->direction].x * (character->move >> 4)) - SCX_REG;
    uint16_t screenY = (character->row * 8 + Directions[character->direction].y * (character->move >> 4)) - SCY_REG;

    move_metasprite(metasprites, baseTile, baseSprite, screenX + 12, screenY + 20);
}