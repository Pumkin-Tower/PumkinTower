/*
 * File: z_select.c
 * Overlay: ovl_select
 * Description: Debug Map Select Menu
 */

#include "z_select.h"
#include "z64shrink_window.h"
#include "z64view.h"
#include "libc/alloca.h"
#include "overlays/gamestates/ovl_title/z_title.h"

#define MAX_SCENES 128
#define MAX_CHARS  40   // chars per scene name (39 readable, 1 for '\0')
#define ITEMS_PER_PAGE 20

static u8 sSceneNameBuffer[MAX_CHARS * MAX_SCENES] =
    "SceneNameBuffer" // this is so zzrtl can find and populate with text
    //"\0\0\0\0\0\0\0\0\0\0\0\0" "Grottos\0" "Curiosity Shop\0" "\0\0" "Ranch House" // test with two items
;
static char* sSceneName[MAX_SCENES] = { (void*)-1 };
static u8 sSceneIndex[MAX_SCENES] = { -1 };

void Custom_MapSelect_LoadConsoleLogo(MapSelectState* this) {
    STOP_GAMESTATE(&this->state);
    SET_NEXT_GAMESTATE(&this->state, ConsoleLogo_Init, sizeof(ConsoleLogoState));
}

void Custom_MapSelect_LoadGame(MapSelectState* this, u32 entrance, s32 spawn) {
    if (gSaveContext.fileNum == 0xFF) {
        Sram_InitDebugSave();
    }
    
    // revamped entrance system
    extern SceneEntranceTableEntry sSceneEntranceTable[0x6f];
    asm("sSceneEntranceTable = 0x801C5720;");
    sSceneEntranceTable[0].table[0]->sceneId = entrance;
    sSceneEntranceTable[0].table[0]->spawnNum = spawn;
    spawn = 0;
    entrance = 0;

    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
    gSaveContext.hudVisibilityForceButtonAlphasByStatus = false;
    gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibilityTimer = 0;

    SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0);
    gSaveContext.save.entrance = entrance;

    if (spawn != 0) {
        gSaveContext.save.entrance =
            Entrance_Create(gSaveContext.save.entrance >> 9, spawn, gSaveContext.save.entrance & 0xF);
    }
    /*
    if (gSaveContext.save.entrance == ENTRANCE(CLOCK_TOWER_INTERIOR, 0)) {
        gSaveContext.save.day = 0;
        gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
    }
    */

    gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = 0xFFFF;
    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
    gSaveContext.showTitleCard = true;
    gSaveContext.respawnFlag = 0;
    gSaveContext.respawn[RESPAWN_MODE_GORON].entrance = 0xFF;
    gSaveContext.respawn[RESPAWN_MODE_ZORA].entrance = 0xFF;
    gSaveContext.respawn[RESPAWN_MODE_DEKU].entrance = 0xFF;
    gSaveContext.respawn[RESPAWN_MODE_HUMAN].entrance = 0xFF;
    gWeatherMode = WEATHER_MODE_CLEAR;

    STOP_GAMESTATE(&this->state);
    SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));
}

#if 0
// "Translation" (Actual name)
static SceneSelectEntry sScenes[] = {
    // "0: OP Woods for Cutscene Use" (Forest Opening Scene)
    { "  0:OP" "Forest Opening Scene", Custom_MapSelect_LoadGame, ENTRANCE(CUTSCENE, 0) },

    // "0-0: Lost Woods" (Lost Woods)
    { "  0-0:" "Lost Woods", Custom_MapSelect_LoadGame, ENTRANCE(LOST_WOODS, 0) },

    // "1: Town Outskirts" (Termina Field)
    { "  1:" "Termina Field", Custom_MapSelect_LoadGame, ENTRANCE(TERMINA_FIELD, 0) },

    // "1-0: Astral Observatory" (Astral Observatory)
    { "  1-0:" "Astral Observatory", Custom_MapSelect_LoadGame, ENTRANCE(ASTRAL_OBSERVATORY, 0) },

    // "1-1: Astral Observatory Telescope" (Astral Observatory - Telescope)
    { "  1-1:" "Telescope", Custom_MapSelect_LoadGame, ENTRANCE(TERMINA_FIELD, 10) },

    // "1-2: Ikana Graveyard" (Ikana Canyon Graveyard)
    { "  1-2:" "Ikana Canyon Graveyard", Custom_MapSelect_LoadGame, ENTRANCE(IKANA_GRAVEYARD, 0) },

    // "2: Romani Ranch" (Romani Ranch)
    { "  2:" "Romani Ranch", Custom_MapSelect_LoadGame, ENTRANCE(ROMANI_RANCH, 0) },

    // "3: Milk Road" (Milk Road)
    { "  3:" "Milk Road", Custom_MapSelect_LoadGame, ENTRANCE(MILK_ROAD, 0) },

    // "4: Main Building" ("Ranch House)
    { "  4:" "Ranch House", Custom_MapSelect_LoadGame, ENTRANCE(RANCH_HOUSE, 1) },

    // "5: Cow Shed" (Cow Shed)
    { "  5:" "Cow Shed", Custom_MapSelect_LoadGame, ENTRANCE(RANCH_HOUSE, 0) },

    // "6: Cucco Shed" (Cucco Shed)
    { "  6:" "Cucco Shed", Custom_MapSelect_LoadGame, ENTRANCE(CUCCO_SHACK, 0) },

    // "7: Dog Racing Area" (Doggy Racetrack)
    { "  7:" "Doggy Racetrack", Custom_MapSelect_LoadGame, ENTRANCE(DOGGY_RACETRACK, 0) },

    // "8: Gorman Track" (Gorman Track)
    { "  8:" "Gorman Track", Custom_MapSelect_LoadGame, ENTRANCE(GORMAN_TRACK, 0) },

    // "10: Mountain Village -Winter-" (Mountain Village - Winter)
    { " 10:" "Mountain Village - Winter", Custom_MapSelect_LoadGame, ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 0) },

    // "10-0: Mountain Village -Spring-" (Mountain Village - Spring)
    { " 10-0:" "After Goht", Custom_MapSelect_LoadGame, ENTRANCE(MOUNTAIN_VILLAGE_SPRING, 0) },

    // "10-1: Mountain Village Blacksmith" (Mountain Smithy)
    { " 10-1:" "Mountain Smithy", Custom_MapSelect_LoadGame, ENTRANCE(MOUNTAIN_SMITHY, 0) },

    // "11: Goron Village -Winter-" (Goron Village - Winter)
    { " 11:" "Goron Village - Winter", Custom_MapSelect_LoadGame, ENTRANCE(GORON_VILLAGE_WINTER, 0) },

    // "11-0: Goron Village -Spring-" (Goron Village - Spring)
    { " 11-0:" "After Goht", Custom_MapSelect_LoadGame, ENTRANCE(GORON_VILLAGE_SPRING, 0) },

    // "11-1: Goron Shrine" (Goron Shrine)
    { " 11-1:" "Goron Shrine", Custom_MapSelect_LoadGame, ENTRANCE(GORON_SHRINE, 0) },

    // "11-2: Lone Peak Shrine" (Lone Peak Shrine)
    { " 11-2:" "Lone Peak Shrine", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 16) },

    // "11-3: Goron Shop" (Goron Shop)
    { " 11-3:" "Goron Shop", Custom_MapSelect_LoadGame, ENTRANCE(GORON_SHOP, 0) },

    // "12: Snowhead" (Snowhead)
    { " 12:" "Snowhead", Custom_MapSelect_LoadGame, ENTRANCE(SNOWHEAD, 0) },

    // "13: Blizzard Path" (Path to Goron Village - Part 1)
    { " 13:" "Path to Mountain Village", Custom_MapSelect_LoadGame, ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 0) },

    // "14: Snowball Path" (Path to Goron Village - Part 2)
    { " 14:" "Path to Snowhead", Custom_MapSelect_LoadGame, ENTRANCE(PATH_TO_SNOWHEAD, 0) },

    // "15: Goron Racetrack" (Goron Racetrack)
    { " 15:" "Goron Racetrack", Custom_MapSelect_LoadGame, ENTRANCE(GORON_RACETRACK, 0) },

    // "16: Goron Grave" (Darmani's Grave)
    { " 16:" "Darmani's Grave", Custom_MapSelect_LoadGame, ENTRANCE(GORON_GRAVERYARD, 0) },

    // "17: Snow Field Battle -Winter-" (Path to Goron Village - Winter)
    { " 17:" "Goron Village Path", Custom_MapSelect_LoadGame, ENTRANCE(PATH_TO_GORON_VILLAGE_WINTER, 0) },

    // "17-0: Snow Field Battle -Spring-" (Path to Goron Village - Spring)
    { " 17-0:" "After Goht", Custom_MapSelect_LoadGame, ENTRANCE(PATH_TO_GORON_VILLAGE_SPRING, 0) },

    // "20: Swampland" (Southern Swamp)
    { " 20:" "Southern Swamp", Custom_MapSelect_LoadGame, ENTRANCE(SOUTHERN_SWAMP_POISONED, 0) },

    // "20-0: Swampland - Afterwards" (Southern Swamp - After Odolwa)
    { " 20-0:" "After Odolwa", Custom_MapSelect_LoadGame, ENTRANCE(SOUTHERN_SWAMP_CLEARED, 0) },

    // "20-1: Swamp Tourist Information" (Tourist Information)
    { " 20-1:" "Tourist Information", Custom_MapSelect_LoadGame, ENTRANCE(TOURIST_INFORMATION, 0) },

    // "20-2: Magic Hags' Potion Shop" (Magic Hags' Potion Shop)
    { " 20-2:" "Magic Hags' Potion Shop", Custom_MapSelect_LoadGame, ENTRANCE(MAGIC_HAGS_POTION_SHOP, 0) },

    // "21: Wood Mountain" (Woodfall)
    { " 21:" "Woodfall", Custom_MapSelect_LoadGame, ENTRANCE(WOODFALL, 0) },

    // "21-0: Deku Princess's Prison" (Deku Princess's Prison Cutscene: Tatl Apologizes)
    { " 21-0:" "Deku Princess", Custom_MapSelect_LoadGame, ENTRANCE(WOODFALL_TEMPLE, 1) },

    // "22: Deku Castle" (Deku Palace)
    { " 22:" "Deku Palace", Custom_MapSelect_LoadGame, ENTRANCE(DEKU_PALACE, 0) },

    // "22-0: Boe Hole 0" (Deku Palace Grotto 0 - Deku Baba & Butterflies, Entrance 1)
    { " 22-0:" "Grotto w/ Baba - Near", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 6) },

    // "22-1: Boe Hole 1" (Deku Palace Grotto 1 - Deku Baba & Butterflies, Entrance 2)
    { " 22-1:" "Grotto w/ Baba - Far", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 14) },

    // "22-2: Boe Hole 2" (Deku Palace Grotto 2 - Skullwalltula Wall, Lower Entrance)
    { " 22-2:" "Grotto w/ Spider - Lower", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 8) },

    // "22-3: Boe Hole 3" (Deku Palace Grotto 3 - Skullwalltula Wall, Upper Entrance)
    { " 22-3:" "Grotto w/ Spider - Upper", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 15) },

    // "22-4: Boe Hole 4" (Deku Palace Grotto 4 - Bean Seller)
    { " 22-4:" "Grotto w/ Bean Seller", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 12) },

    // "24: Beast Path" (Road to Swamp)
    { " 24:" "Road to Swamp", Custom_MapSelect_LoadGame, ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 0) },

    // "24-0: Forest Shooting Gallery" (Swamp Shooting Gallery)
    { " 24-0:" "Swamp Shooting Gallery", Custom_MapSelect_LoadGame, ENTRANCE(SWAMP_SHOOTING_GALLERY, 0) },

    // "25: Deku King's Chamber" (Deku Palace Throne Room)
    { " 25:" "Deku Palace Throne Room", Custom_MapSelect_LoadGame, ENTRANCE(DEKU_KINGS_CHAMBER, 0) },

    // "26: Woods of Mystery" (Woods of Mystery)
    { " 26:" "Woods of Mystery", Custom_MapSelect_LoadGame, ENTRANCE(WOODS_OF_MYSTERY, 0) },

    // "30: Great Bay Coast" (Great Bay Coast - Entrance Area)
    { " 30:" "Great Bay Coast - Entrance Area", Custom_MapSelect_LoadGame, ENTRANCE(GREAT_BAY_COAST, 0) },

    // "30-0: Ocean Laboratory" (Marine Research Lab)
    { " 30-0:" "Marine Research Lab", Custom_MapSelect_LoadGame, ENTRANCE(MARINE_RESEARCH_LAB, 0) },

    // "30-1: Fisherman's House" (Fisherman's Hut)
    { " 30-1:" "Fisherman's Hut", Custom_MapSelect_LoadGame, ENTRANCE(FISHERMANS_HUT, 0) },

    // "30-2: Pointed Rock" (Twin Pillars)
    { " 30-2:" "Twin Pillars", Custom_MapSelect_LoadGame, ENTRANCE(PINNACLE_ROCK, 0) },

    // "31: Cape" (Great Bay Coast - River Area)
    { " 31:" "Great Bay Coast - River", Custom_MapSelect_LoadGame, ENTRANCE(ZORA_CAPE, 0) },

    // "32: Outside of Pirates' Fortress" (Pirates' Fortress - Exterior)
    { " 32:" "Pirates' Fortress", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_EXTERIOR, 0) },

    // "32-0: Pirates' Fortress" (Pirates' Fortress - Courtyard)
    { " 32-0:" "Courtyard", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS, 0) },

    // "32-1: Pirates' Fortress - Telescope" (Pirates' Fortress - Secret Entrance, Looking Through Telescope)
    { " 32-1:" "Using Telescope", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS, 10) },

    // "32-2: Pirates' Fortress - Interior 0" (Pirates' Fortress - Throne Room)
    { " 32-2:" "Throne Room", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 0) },

    // "32-3: Pirates' Fortress - Interior 1" (Pirates' Fortress - View of Throne Room, Wasp Nest)
    { " 32-3:" "Wasp Nest", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 1) },

    // "32-4: Pirates' Fortress - Interior 2" (Pirates' Fortress - Tempting Treasure Chest, Leading to 32-5)
    { " 32-4:" "Treasure Chest", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 2) },

    // "32-5: Pirates' Fortress - Interior 3" (Pirates' Fortress - Zora Egg Room, One Shell Blade)
    { " 32-5:" "Zora Egg w/ Shellblade", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 3) },

    // "32-6: Pirates' Fortress - Interior 4" (Pirates' Fortress - Oil Drum Room, Leading to 32-7)
    { " 32-6:" "Oil Drum Room", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 4) },

    // "32-7: Pirates' Fortress - Interior 5" (Pirates' Fortress - Zora Egg Room, One Shell Blade)
    { " 32-7:" "Zora Egg w/ Shellblade", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 5) },

    // "32-8: Pirates' Fortress - Interior 6" (Pirates' Fortress - Fenced Indoor Walkway, Leading to 32-9)
    { " 32-8:" "Fenced Indoor Walkway", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 6) },

    // "32-9: Pirates' Fortress - Interior 7" (Pirates' Fortress - Zora Egg Room, One Desbreko & Treasure Chest)
    { " 32-9:" "Zora Egg w/ Desbreko", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 7) },

    // "32-10: Pirates' Fortress - Interior 8" (Pirates' Fortress - End of Secret Entrance: Telescope Room)
    { " 32-10:" "Beside Telescope", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 8) },

    // "32-11: Pirates' Fortress - Interior 9" (Pirates' Fortress - Start of Secret Entrance)
    { " 32-11:" "Secret Entrance", Custom_MapSelect_LoadGame, ENTRANCE(PIRATES_FORTRESS_INTERIOR, 9) },

    // "33: Zora Shrine" (Zora Hall)
    { " 33:" "Zora Hall", Custom_MapSelect_LoadGame, ENTRANCE(ZORA_HALL, 0) },

    // "33-0: Zora Shop" (Zora Shop)
    { " 33-0:" "Zora Shop", Custom_MapSelect_LoadGame, ENTRANCE(ZORA_HALL_ROOMS, 5) },

    // "33-1: Zora Waiting Room" (Mikau & Tijo's Room)
    { " 33-1:" "Mikau & Tijo's Room", Custom_MapSelect_LoadGame, ENTRANCE(ZORA_HALL_ROOMS, 0) },

    // "34: Great Bay" (Great Bay Cutscene: Pirates Approach Temple)
    { " 34:" "Great Bay Credits - Pirates", Custom_MapSelect_LoadGame, ENTRANCE(GREAT_BAY_CUTSCENE, 0) },

    // "35: Mountain Stream Above Falls" (Waterfall Rapids)
    { " 35:" "Waterfall Rapids", Custom_MapSelect_LoadGame, ENTRANCE(WATERFALL_RAPIDS, 0) },

    // "40: Rock Building Shaft" (Stone Tower)
    { " 40:" "Stone Tower", Custom_MapSelect_LoadGame, ENTRANCE(STONE_TOWER, 0) },

    // "40-0: Heaven & Earth Are Overturned" (Stone Tower Cutscene: Tower is Flipped)
    { " 40-0:" "Cutscene: Flip Tower", Custom_MapSelect_LoadGame, ENTRANCE(STONE_TOWER_INVERTED, 0) },

    // "41: Road to Ikana" (Road to Ikana)
    { " 41:" "Road to Ikana", Custom_MapSelect_LoadGame, ENTRANCE(ROAD_TO_IKANA, 0) },

    // "42: Ancient Castle of Ikana" (Ancient Castle of Ikana)
    { " 42:" "Ancient Castle of Ikana", Custom_MapSelect_LoadGame, ENTRANCE(IKANA_CASTLE, 0) },

    // "42-0: Ancient Castle of Ikana - Interior" (Ancient Castle of Ikana - Interior)
    { " 42-0:" "Interior", Custom_MapSelect_LoadGame, ENTRANCE(IKANA_CASTLE, 3) },

    // "42-B: Ancient Castle of Ikana - Boss Room" (Ikana King's Throne)
    { " 42-B:" "Igos du Ikana", Custom_MapSelect_LoadGame, ENTRANCE(IGOS_DU_IKANAS_LAIR, 0) },

    // "43: Ikana Canyon" (Ikana Canyon)
    { " 43:" "Ikana Canyon", Custom_MapSelect_LoadGame, ENTRANCE(IKANA_CANYON, 0) },

    // "43-0: Ikana Canyon Cave" (Sharp's Cave)
    { " 43-0:" "Sharp's Cave", Custom_MapSelect_LoadGame, ENTRANCE(IKANA_CANYON, 14) },

    // "43-1: Secom's House" (Sakon's Hideout)
    { " 43-1:" "Sakon's Hideout", Custom_MapSelect_LoadGame, ENTRANCE(SAKONS_HIDEOUT, 0) },

    // "43-2: Music Box House" (Music Box House)
    { " 43-2:" "Music Box House", Custom_MapSelect_LoadGame, ENTRANCE(MUSIC_BOX_HOUSE, 0) },

    // "50: Clock Town -East-" (East Clock Town)
    { " 50:" "East Clock Town", Custom_MapSelect_LoadGame, ENTRANCE(EAST_CLOCK_TOWN, 0) },

    // "50-0: Town Target Range" (Shooting Gallery)
    { " 50-0:" "Shooting Gallery", Custom_MapSelect_LoadGame, ENTRANCE(TOWN_SHOOTING_GALLERY, 0) },

    // "50-1: Honey and Darling's Shop" (Honey & Darling's Shop)
    { " 50-1:" "Honey & Darling's Shop", Custom_MapSelect_LoadGame, ENTRANCE(HONEY_AND_DARLINGS_SHOP, 0) },

    // "50-2: Treasure Chest Shop" (Treasure Chest Shop)
    { " 50-2:" "Treasure Chest Shop", Custom_MapSelect_LoadGame, ENTRANCE(TREASURE_CHEST_SHOP, 0) },

    // "50-3: Pots 'n' Pans Inn" (Stockpot Inn)
    { " 50-3:" "Stockpot Inn", Custom_MapSelect_LoadGame, ENTRANCE(STOCK_POT_INN, 0) },

    // "50-4: Mayor's House" (The Mayor's Residence)
    { " 50-4:" "Mayor's Office", Custom_MapSelect_LoadGame, ENTRANCE(MAYORS_RESIDENCE, 0) },

    // "50-5: Milk Bar" (Milk Bar)
    { " 50-5:" "Milk Bar", Custom_MapSelect_LoadGame, ENTRANCE(MILK_BAR, 0) },

    // "51: Clock Town -West-" (West Clock Town)
    { " 51:" "West Clock Town", Custom_MapSelect_LoadGame, ENTRANCE(WEST_CLOCK_TOWN, 0) },

    // "51-0: Bomb Shop" (Bomb Shop)
    { " 51-0:" "Bomb Shop", Custom_MapSelect_LoadGame, ENTRANCE(BOMB_SHOP, 0) },

    // "51-1: Maniac Mart" (Curiosity Shop)
    { " 51-1:" "Curiosity Shop", Custom_MapSelect_LoadGame, ENTRANCE(CURIOSITY_SHOP, 0) },

    // "51-2: General Store" (Trading Post)
    { " 51-2:" "Trading Post", Custom_MapSelect_LoadGame, ENTRANCE(TRADING_POST, 0) },

    // "51-3: Sword Dojo" (Swordsman's School)
    { " 51-3:" "Swordsman's School", Custom_MapSelect_LoadGame, ENTRANCE(SWORDMANS_SCHOOL, 0) },

    // "51-4: Post House" (Post Office)
    { " 51-4:" "Post Office", Custom_MapSelect_LoadGame, ENTRANCE(POST_OFFICE, 0) },

    // "51-5: Lottery Shop" (Lottery Shop)
    { " 51-5:" "Lottery Shop", Custom_MapSelect_LoadGame, ENTRANCE(LOTTERY_SHOP, 0) },

    // "52: Clock Town -North-" (North Clock Town)
    { " 52:" "North Clock Town", Custom_MapSelect_LoadGame, ENTRANCE(NORTH_CLOCK_TOWN, 0) },

    // "53: Clocktown -South-" (South Clock Town)
    { " 53:" "South Clock Town", Custom_MapSelect_LoadGame, ENTRANCE(SOUTH_CLOCK_TOWN, 0) },

    // "53-0: Clock Tower Interior" (Clock Tower Interior)
    { " 53-0:" "Clock Tower Interior", Custom_MapSelect_LoadGame, ENTRANCE(CLOCK_TOWER_INTERIOR, 0) },

    // "54: Clock Tower Rooftop" (Clock Tower Rooftop)
    { " 54:" "Clock Tower Rooftop", Custom_MapSelect_LoadGame, ENTRANCE(CLOCK_TOWER_ROOFTOP, 0) },

    // "55: Laundry Area" (Laundry Pool)
    { " 55:" "Laundry Pool", Custom_MapSelect_LoadGame, ENTRANCE(LAUNDRY_POOL, 0) },

    // "55-0: Maniac Mart - Rear Entrance" (Curiosity Shop - Back Room)
    { " 55-0:" "Curiosity Shop Back Room", Custom_MapSelect_LoadGame, ENTRANCE(CURIOSITY_SHOP, 1) },

    // "55-1: Maniac Mart - Peephole" (Curiosity Shop Back Room - Peephole)
    { " 55-1:" "Curiosity Shop Peephole", Custom_MapSelect_LoadGame, ENTRANCE(CURIOSITY_SHOP, 2) },

    // "100: Wood Mountain Temple" (Woodfall Temple)
    { "100:" "Woodfall Temple", Custom_MapSelect_LoadGame, ENTRANCE(WOODFALL_TEMPLE, 0) },

    // "100-B: Wood Mountain Temple - Boss" (Odolwa's Lair)
    { "100-B:" "Odolwa's Lair", Custom_MapSelect_LoadGame, ENTRANCE(ODOLWAS_LAIR, 0) },

    // "101: Snowhead Temple" (Snowhead Temple)
    { "101:" "Snowhead Temple", Custom_MapSelect_LoadGame, ENTRANCE(SNOWHEAD_TEMPLE, 0) },

    // "101-B: Snowhead Temple - Boss" (Goht's Lair)
    { "101-B:" "Goht's Lair", Custom_MapSelect_LoadGame, ENTRANCE(GOHTS_LAIR, 0) },

    // "102: Great Bay Temple" (Great Bay Temple)
    { "102:" "Great Bay Temple", Custom_MapSelect_LoadGame, ENTRANCE(GREAT_BAY_TEMPLE, 0) },

    // "102-B: Great Bay Temple - Boss" (Gyorg's Lair)
    { "102-B:" "Gyorg's Lair", Custom_MapSelect_LoadGame, ENTRANCE(GYORGS_LAIR, 0) },

    // "103: Rock Building Temple -Top Side-" (Stone Tower Temple)
    { "103:" "Stone Tower Temple", Custom_MapSelect_LoadGame, ENTRANCE(STONE_TOWER_TEMPLE, 0) },

    // "103-0: Rock Building Temple -Underside-" (Stone Tower Temple - Flipped)
    { "103-0:" "Upside Down", Custom_MapSelect_LoadGame, ENTRANCE(STONE_TOWER_TEMPLE_INVERTED, 0) },

    // "103-B: Rock Building Temple - Boss" (Twinmold's Lair)
    { "103-B:" "Twinmold's Lair", Custom_MapSelect_LoadGame, ENTRANCE(TWINMOLDS_LAIR, 0) },

    // "104: Steppe" (On the Moon)
    { "104:" "On the Moon", Custom_MapSelect_LoadGame, ENTRANCE(THE_MOON, 0) },

    // "104-0: Last Deku Dungeon" (Deku Trial)
    { "104-0:" "Deku Trial", Custom_MapSelect_LoadGame, ENTRANCE(MOON_DEKU_TRIAL, 0) },

    // "104-1: Last Goron Dungeon" (Goron Trial)
    { "104-1:" "Goron Trial", Custom_MapSelect_LoadGame, ENTRANCE(MOON_GORON_TRIAL, 0) },

    // "104-2: Last Zora Dungeon" (Zora Trial)
    { "104-2:" "Zora Trial", Custom_MapSelect_LoadGame, ENTRANCE(MOON_ZORA_TRIAL, 0) },

    // "104-3: Last Link Dungeon" (Link Trial)
    { "104-3:" "Link Trial", Custom_MapSelect_LoadGame, ENTRANCE(MOON_LINK_TRIAL, 0) },

    // "104-B: Last Dungeon -Boss-" (Majora's Lair)
    { "104-B:" "Majora's Lair", Custom_MapSelect_LoadGame, ENTRANCE(MAJORAS_LAIR, 0) },

    // "119: Deku Shrine" (Deku Shrine)
    { "119:" "Deku Shrine", Custom_MapSelect_LoadGame, ENTRANCE(DEKU_SHRINE, 0) },

    // "121: Secret Shrine" (Secret Shrine)
    { "121:" "Secret Shrine", Custom_MapSelect_LoadGame, ENTRANCE(SECRET_SHRINE, 0) },

    // "122: Opening Dungeon" (Pond Area Cutscene: Falling of the Cliff)
    { "122:" "Intro: Falling into abyss", Custom_MapSelect_LoadGame, ENTRANCE(OPENING_DUNGEON, 0) },

    // "123: Giants' Chamber" (Giants' Chamber)
    { "123:" "Giants' Chamber", Custom_MapSelect_LoadGame, ENTRANCE(GIANTS_CHAMBER, 0) },

    // "126: Deku Minigame" (Deku Rupee Minigame)
    { "126:" "Deku Rupee Minigame", Custom_MapSelect_LoadGame, ENTRANCE(DEKU_SCRUB_PLAYGROUND, 0) },

    // "127-0: Fairy Fountain 0" (Fairy Fountain - Clock Town)
    { "127-0:" "Fairy - Clock Town", Custom_MapSelect_LoadGame, ENTRANCE(FAIRY_FOUNTAIN, 0) },

    // "127-1: Fairy Fountain 1" (Fairy Fountain - Woodfall)
    { "127-1:" "Fairy - Woodfall", Custom_MapSelect_LoadGame, ENTRANCE(FAIRY_FOUNTAIN, 1) },

    // "127-2: Fairy Fountain 2" (Fairy Fountain - Snowhead)
    { "127-2:" "Fairy - Snowhead", Custom_MapSelect_LoadGame, ENTRANCE(FAIRY_FOUNTAIN, 2) },

    // "127-3: Fairy Fountain 3" (Fairy Fountain - Great Bay Coast)
    { "127-3:" "Fairy - Great Bay", Custom_MapSelect_LoadGame, ENTRANCE(FAIRY_FOUNTAIN, 3) },

    // "127-4: Fairy Fountain 4" (Fairy Fountain - Ikana Canyon)
    { "127-4:" "Fairy - Ikana", Custom_MapSelect_LoadGame, ENTRANCE(FAIRY_FOUNTAIN, 4) },

    // "128: Swamp Spider Manor" (Swamp Spider House)
    { "128:" "Swamp Spider House", Custom_MapSelect_LoadGame, ENTRANCE(SWAMP_SPIDER_HOUSE, 0) },

    // "129: Ocean Spider Manor" (Oceanside Spider House)
    { "129:" "Oceanside Spider House", Custom_MapSelect_LoadGame, ENTRANCE(OCEANSIDE_SPIDER_HOUSE, 0) },

    // "130: Beneath the Graves - Dampe" (Beneath the Graveyard)
    { "130:" "Dampe's Hideout", Custom_MapSelect_LoadGame, ENTRANCE(DAMPES_HOUSE, 0) },

    // "131: Beneath the Well" (Beneath the Well)
    { "131:" "Beneath the Well", Custom_MapSelect_LoadGame, ENTRANCE(BENEATH_THE_WELL, 0) },

    // "132: Ghost Hut" (Ghost Hut)
    { "132:" "Ghost Hut", Custom_MapSelect_LoadGame, ENTRANCE(GHOST_HUT, 0) },

    // "133-0: Beneath the Graves 0" (Beneath the Graveyard - Part 1)
    { "133-0:" "Beneath Graveyard Near", Custom_MapSelect_LoadGame, ENTRANCE(BENEATH_THE_GRAVERYARD, 0) },

    // "133-1: Beneath the Graves 1" (Beneath the Graveyard - Part 2)
    { "133-1:" "Beneath Graveyard Far", Custom_MapSelect_LoadGame, ENTRANCE(BENEATH_THE_GRAVERYARD, 1) },

    // "134-0: Secret Grotto 0" (Secret Grotto - Four Gossip Stones)
    { "134:" "Grottos: Gossip Stones", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 0) },

    // "134-1: Secret Grotto 1" (Secret Grotto - Four Gossip Stones, Skulltula)
    { "134-1:" "Gossips, Spider", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 1) },

    // "134-2: Secret Grotto 2" (Secret Grotto - Four Gossip Stones, Water Puddles)
    { "134-2:" "Gossips, Puddles", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 2) },

    // "134-3: Secret Grotto 3" (Secret Grotto - Four Gossip Stones, Water Puddle With Bugs)
    { "134-3:" "Gossips, Puddles, Bugs", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 3) },

    // "134-4: Secret Grotto 4" (Secret Grotto - Chest with Blue Rupee, Deku Babas)
    { "134-4:" "Blue Rupee Chest, Babas", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 4) },

    // "134-5: Secret Grotto 5" (Secret Grotto - Hot Spring, Deku Babas, Large Stones)
    { "134-5:" "Hot Spring, Babas, Rocks", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 5) },

    // "134-7: Secret Grotto 7" (Secret Grotto - Two Dondogos)
    { "134-7:" "Dodongos", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 7) },

    // "134-9: Secret Grotto 9" (Secret Grotto - Tall Grass With Box, Pot, Bugs)
    { "134-9:" "Tall Grass w/ Box/Pot/Bugs", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 9) },

    // "134-10: Secret Grotto 10" (Secret Grotto - Two Cows Surrounded by Grass)
    { "134-10:" "Cows & Grass", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 10) },

    // "134-11: Secret Grotto 11" (Secret Grotto - Watery Hole Filled with Underwater Babas, Fish)
    { "134-11:" "Aquifer w/ Biobabas", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 11) },

    // "134-13: Secret Grotto 13" (Secret Grotto - Peahat in Center)
    { "134-13:" "Peahat", Custom_MapSelect_LoadGame, ENTRANCE(GROTTOS, 13) },

    // "X 1: SPOT00" (Opening Cutscene)
    { "X 1:SPOT00", Custom_MapSelect_LoadGame, ENTRANCE(CUTSCENE, 0) },

    // "Title" (Title Screen)
    { "title", (void*)Custom_MapSelect_LoadConsoleLogo, 0 },
};
#endif

void Custom_MapSelect_UpdateMenu(MapSelectState* this) {
    s32 playerForm;
    Input* controller1 = CONTROLLER1(&this->state);
    s32 stickY;
    s32 pad[2];
    s16 i;
    s32 updateRate;

    if (this->verticalInputAccumulator == 0) {
        if (CHECK_BTN_ALL(controller1->press.button, BTN_A) || CHECK_BTN_ALL(controller1->press.button, BTN_START)) {
            for (i = 0; i < TIMER_ID_MAX; i++) {
                gSaveContext.timerStates[i] = TIMER_STATE_OFF;
                gSaveContext.timerCurTimes[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerTimeLimits[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerStartOsTimes[i] = 0;
                gSaveContext.timerStopTimes[i] = SECONDS_TO_TIMER(0);
                gSaveContext.timerPausedOsTimes[i] = 0;
            }
            gSaveContext.minigameStatus = MINIGAME_STATUS_INACTIVE;

            //if (this->scenes[this->currentScene].loadFunc != NULL) {
            //    this->scenes[this->currentScene].loadFunc(this, this->scenes[this->currentScene].entrance, this->opt);
            //}
            Custom_MapSelect_LoadGame(this, sSceneIndex[this->currentScene], this->opt);
        }

        if (CHECK_BTN_ALL(controller1->press.button, BTN_B)) {
            playerForm = GET_PLAYER_FORM - 1;
            if (playerForm < PLAYER_FORM_FIERCE_DEITY) {
                playerForm = PLAYER_FORM_HUMAN;
            }
            gSaveContext.save.playerForm = playerForm;
        }

        if (CHECK_BTN_ALL(controller1->press.button, BTN_Z)) {
            if (gSaveContext.save.cutsceneIndex == 0x8000) {
                gSaveContext.save.cutsceneIndex = 0;
            } else if (gSaveContext.save.cutsceneIndex == 0) {
                gSaveContext.save.cutsceneIndex = 0x8800;
            } else if (gSaveContext.save.cutsceneIndex == 0x8800) {
                gSaveContext.save.cutsceneIndex = 0xFFF0;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF0) {
                gSaveContext.save.cutsceneIndex = 0xFFF1;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF1) {
                gSaveContext.save.cutsceneIndex = 0xFFF2;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF2) {
                gSaveContext.save.cutsceneIndex = 0xFFF3;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF3) {
                gSaveContext.save.cutsceneIndex = 0xFFF4;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF4) {
                gSaveContext.save.cutsceneIndex = 0xFFF5;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF5) {
                gSaveContext.save.cutsceneIndex = 0xFFF6;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF6) {
                gSaveContext.save.cutsceneIndex = 0xFFF7;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF7) {
                gSaveContext.save.cutsceneIndex = 0xFFF8;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF8) {
                gSaveContext.save.cutsceneIndex = 0xFFF9;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF9) {
                gSaveContext.save.cutsceneIndex = 0xFFFA;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFFA) {
                gSaveContext.save.cutsceneIndex = 0x8000;
            }
        } else if (CHECK_BTN_ALL(controller1->press.button, BTN_R)) {
            if (gSaveContext.save.cutsceneIndex == 0x8000) {
                gSaveContext.save.cutsceneIndex = 0xFFFA;
            } else if (gSaveContext.save.cutsceneIndex == 0) {
                gSaveContext.save.cutsceneIndex = 0x8000;
            } else if (gSaveContext.save.cutsceneIndex == 0x8800) {
                gSaveContext.save.cutsceneIndex = 0;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF0) {
                gSaveContext.save.cutsceneIndex = 0x8800;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF1) {
                gSaveContext.save.cutsceneIndex = 0xFFF0;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF2) {
                gSaveContext.save.cutsceneIndex = 0xFFF1;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF3) {
                gSaveContext.save.cutsceneIndex = 0xFFF2;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF4) {
                gSaveContext.save.cutsceneIndex = 0xFFF3;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF5) {
                gSaveContext.save.cutsceneIndex = 0xFFF4;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF6) {
                gSaveContext.save.cutsceneIndex = 0xFFF5;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF7) {
                gSaveContext.save.cutsceneIndex = 0xFFF6;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF8) {
                gSaveContext.save.cutsceneIndex = 0xFFF7;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFF9) {
                gSaveContext.save.cutsceneIndex = 0xFFF8;
            } else if (gSaveContext.save.cutsceneIndex == 0xFFFA) {
                gSaveContext.save.cutsceneIndex = 0xFFF9;
            }
        }

        gSaveContext.save.isNight = false;
        if (gSaveContext.save.cutsceneIndex == 0x8800) {
            gSaveContext.save.isNight = true;
        }

        if (CHECK_BTN_ALL(controller1->press.button, BTN_CUP)) {
            if (this->opt > 0)
                this->opt--;
        }
        if (CHECK_BTN_ALL(controller1->press.button, BTN_CDOWN)) {
            this->opt++;
        }

        if (CHECK_BTN_ALL(controller1->press.button, BTN_CLEFT)) {
            if (gSaveContext.save.day > 1) {
                gSaveContext.save.day--;
            }
        }

        if (CHECK_BTN_ALL(controller1->press.button, BTN_CRIGHT)) {
            if (gSaveContext.save.day < 4) {
                gSaveContext.save.day++;
            }
        }

        updateRate = R_UPDATE_RATE;
        stickY = controller1->rel.stick_y;
        if (stickY != 0) {
            this->verticalInput = (updateRate * stickY) / 7;
        } else {
            if (CHECK_BTN_ALL(controller1->press.button, BTN_DUP)) {
                if (this->lockUp == true) {
                    this->timerUp = 0;
                }
                if (this->timerUp == 0) {
                    this->timerUp = 20;
                    this->lockUp = true;

                    Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                    this->verticalInput = updateRate;
                }
            }

            if (CHECK_BTN_ALL(controller1->cur.button, BTN_DUP) && (this->timerUp == 0)) {
                Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                this->verticalInput = updateRate * 3;
            }

            if (CHECK_BTN_ALL(controller1->press.button, BTN_DDOWN)) {
                if (this->lockDown == true) {
                    this->timerDown = 0;
                }
                if (this->timerDown == 0) {
                    this->timerDown = 20;
                    this->lockDown = true;
                    Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                    this->verticalInput = -updateRate;
                }
            }
            if (CHECK_BTN_ALL(controller1->cur.button, BTN_DDOWN) && (this->timerDown == 0)) {
                Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                this->verticalInput = -updateRate * 3;
            }

            if (CHECK_BTN_ALL(controller1->press.button, BTN_DLEFT) ||
                CHECK_BTN_ALL(controller1->cur.button, BTN_DLEFT)) {
                Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                this->verticalInput = updateRate;
            }

            if (CHECK_BTN_ALL(controller1->press.button, BTN_DRIGHT) ||
                CHECK_BTN_ALL(controller1->cur.button, BTN_DRIGHT)) {
                Audio_PlaySfx(NA_SE_IT_SWORD_IMPACT);
                this->verticalInput = -updateRate;
            }
        }
    }

    if (CHECK_BTN_ALL(controller1->press.button, BTN_L)) {
        this->pageDownIndex++;
        this->pageDownIndex =
            (this->pageDownIndex + ARRAY_COUNT(this->pageDownStops)) % ARRAY_COUNT(this->pageDownStops);
        this->currentScene = this->topDisplayedScene = this->pageDownStops[this->pageDownIndex];
    }

    this->verticalInputAccumulator += this->verticalInput;

    if (this->verticalInputAccumulator < -7) {
        this->verticalInput = 0;
        this->verticalInputAccumulator = 0;

        this->currentScene++;
        this->currentScene = (this->currentScene + this->count) % this->count;

        if (this->currentScene == ((this->topDisplayedScene + this->count + 19) % this->count)) {
            this->topDisplayedScene++;
            this->topDisplayedScene = (this->topDisplayedScene + this->count) % this->count;
        }
    }

    if (this->verticalInputAccumulator > 7) {
        this->verticalInput = 0;
        this->verticalInputAccumulator = 0;

        if (this->currentScene == this->topDisplayedScene) {
            this->topDisplayedScene -= 2;
            this->topDisplayedScene = (this->topDisplayedScene + this->count) % this->count;
        }

        this->currentScene--;
        this->currentScene = (this->currentScene + this->count) % this->count;

        if (this->currentScene == ((this->topDisplayedScene + this->count) % this->count)) {
            this->topDisplayedScene--;
            this->topDisplayedScene = (this->topDisplayedScene + this->count) % this->count;
        }
    }

    this->currentScene = (this->currentScene + this->count) % this->count;
    this->topDisplayedScene = (this->topDisplayedScene + this->count) % this->count;

    dREG(80) = this->currentScene;
    dREG(81) = this->topDisplayedScene;
    dREG(82) = this->pageDownIndex;

    if (this->timerUp != 0) {
        this->timerUp--;
    }

    if (this->timerUp == 0) {
        this->lockUp = false;
    }

    if (this->timerDown != 0) {
        this->timerDown--;
    }

    if (this->timerDown == 0) {
        this->lockDown = false;
    }
    
    // prevent scrolling when there are too few items to scroll through
    if (this->count <= ITEMS_PER_PAGE)
        this->topDisplayedScene = 0;
}

void Custom_MapSelect_PrintMenu(MapSelectState* this, GfxPrint* printer) {
    s32 i;

    GfxPrint_SetColor(printer, 255, 155, 150, 255);
    GfxPrint_SetPos(printer, 12, 2);
    GfxPrint_Printf(printer, "ZELDA MAP SELECT");
    GfxPrint_SetColor(printer, 255, 255, 255, 255);

    for (i = 0; i < ITEMS_PER_PAGE; i++) {
        s32 sceneIndex;
        char* sceneName;
        
        // prevent scrolling when there are too few items to scroll through
        if (this->count <= ITEMS_PER_PAGE && i >= this->count)
            break;

        GfxPrint_SetPos(printer, 1/*9*/, i + 4);
        sceneIndex = (this->topDisplayedScene + i + this->count) % this->count;
        if (sceneIndex == this->currentScene) {
            GfxPrint_SetColor(printer, 255, 20, 20, 255);
        } else {
            GfxPrint_SetColor(printer, 200, 200, 55, 255);
        }

        //sceneName = this->scenes[sceneIndex].name;
        sceneName = sSceneName[sceneIndex];
        if (sceneName == NULL) {
            sceneName = "**Null**";
        }
        //GfxPrint_Printf(printer, "%s", sceneName);
        GfxPrint_Printf(printer, "%3d %s", sSceneIndex[sceneIndex], sceneName);
    }

    GfxPrint_SetColor(printer, 155, 55, 150, 255);
    GfxPrint_SetPos(printer, 23/*20*/, 26);
    GfxPrint_Printf(printer, "SpawnPoint:%d", this->opt);
    
    static char* hints[] = {
        "Use C-Down/Up to adjust SpawnPoint"
        , "Use D-Down/Up to select a scene"
        , "Use control stick to move fast"
    };
    static int hintsCounter = 0;
    GfxPrint_SetPos(printer, 3, 28);
    GfxPrint_Printf(printer, "%s", hints[((hintsCounter++) / (60 * 2)) % ARRAY_COUNT(hints)]);
}

static const char* sLoadingMessages[] = {
    "Please wait a minute",
    "Hold on a sec",
    "Wait a moment",
    "Loading",
    "Now working",
    "Now creating",
    "It's not broken",
    "Coffee Break",
    "Please set B side",
    "Be patient, now",
    "Please wait just a minute",
    "Don't worry, don't worry. Take a break, take a break.",
};

void Custom_MapSelect_PrintLoadingMessage(MapSelectState* this, GfxPrint* printer) {
    s32 randomMsg;

    GfxPrint_SetPos(printer, 10, 15);
    GfxPrint_SetColor(printer, 255, 255, 255, 255);

    randomMsg = Rand_ZeroOne() * ARRAY_COUNT(sLoadingMessages);
    GfxPrint_Printf(printer, "%s", sLoadingMessages[randomMsg]);
}

static const char* sFormLabel[] = {
    "17 (Adult)",
    "30 (Goron)",
    "78 (Zora)",
    "12 (Deku)",
    "5 (Child)",
};

void Custom_MapSelect_PrintAgeSetting(MapSelectState* this, GfxPrint* printer, s32 playerForm) {
    s32 pad;
    const char* age;
    s32 phi_v0;

    if ((playerForm >= PLAYER_FORM_FIERCE_DEITY) && (playerForm < PLAYER_FORM_MAX)) {
        age = sFormLabel[playerForm];
    } else {
        age = NULL;
    }

    GfxPrint_SetPos(printer, 4, 26);
    GfxPrint_SetColor(printer, 255, 255, 55, 255);
    if (age != NULL) {
        GfxPrint_Printf(printer, "Form:%s", age);
    } else {
        // clang-format off
        GfxPrint_Printf(printer, "Form:???" "(%d)", playerForm);
        // clang-format on
    }
}

void Custom_MapSelect_PrintCutsceneSetting(MapSelectState* this, GfxPrint* printer, u16 csIndex) {
    const char* stage;
    const char* day;

    GfxPrint_SetPos(printer, 4, 25);
    GfxPrint_SetColor(printer, 255, 255, 55, 255);

    // "-jara" used in these strings is a Kokiri speech quirk word
    switch (csIndex) {
        case 0:
            // clang-format off
            // "Afternoon-jara"
            gSaveContext.save.time = CLOCK_TIME(12, 0); stage = "Noon";
            // clang-format on
            break;

        case 0x8000:
            // clang-format off
            // "Morning-jara"
            gSaveContext.save.time = CLOCK_TIME(6, 0) + 1; stage = "Morning";
            // clang-format on
            break;

        case 0x8800:
            gSaveContext.save.time = CLOCK_TIME(18, 1);
            // "Night-jara"
            stage = "Night";
            break;

        case 0xFFF0:
            // clang-format off
            // "Cutscene 00"
            gSaveContext.save.time = CLOCK_TIME(12, 0); stage = "Cutscene 00";
            // clang-format on
            break;

        case 0xFFF1:
            // "Cutscene 01"
            stage = "Cutscene 01";
            break;

        case 0xFFF2:
            // "Cutscene 02"
            stage = "Cutscene 02";
            break;

        case 0xFFF3:
            // "Cutscene 03"
            stage = "Cutscene 03";
            break;

        case 0xFFF4:
            // "Cutscene 04"
            stage = "Cutscene 04";
            break;

        case 0xFFF5:
            // "Cutscene 05"
            stage = "Cutscene 05";
            break;

        case 0xFFF6:
            // "Cutscene 06"
            stage = "Cutscene 06";
            break;

        case 0xFFF7:
            // "Cutscene 07"
            stage = "Cutscene 07";
            break;

        case 0xFFF8:
            // "Cutscene 08"
            stage = "Cutscene 08";
            break;

        case 0xFFF9:
            // "Cutscene 09"
            stage = "Cutscene 09";
            break;

        case 0xFFFA:
            // "Cutscene 0A"
            stage = "Cutscene 0A";
            break;

        default:
            stage = "???";
            break;
    }
    gSaveContext.skyboxTime = gSaveContext.save.time;
    GfxPrint_Printf(printer, "Stage:" "%s", stage);

    GfxPrint_SetPos(printer, 23, 25);
    GfxPrint_SetColor(printer, 255, 255, 55, 255);

    switch (gSaveContext.save.day) {
        case 1:
            // "The First Day"
            day = "1";
            break;

        case 2:
            // "The Next Day"
            day = "2";
            break;

        case 3:
            // "The Final Day"
            day = "3";
            break;

        case 4:
            // "Clear Day"
            day = "4";
            break;

        default:
            gSaveContext.save.day = 1;
            // "The First Day"
            day = "1";
            break;
    }

    GfxPrint_Printf(printer, "Day:" "%s", day);
}

void Custom_MapSelect_DrawMenu(MapSelectState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    GfxPrint* printer;

    OPEN_DISPS(gfxCtx);

    Gfx_SetupDL28_Opa(gfxCtx);

    printer = alloca(sizeof(GfxPrint));
    GfxPrint_Init(printer);
    GfxPrint_Open(printer, POLY_OPA_DISP);

    Custom_MapSelect_PrintMenu(this, printer);
    Custom_MapSelect_PrintAgeSetting(this, printer, GET_PLAYER_FORM);
    Custom_MapSelect_PrintCutsceneSetting(this, printer, ((void)0, gSaveContext.save.cutsceneIndex));

    POLY_OPA_DISP = GfxPrint_Close(printer);
    GfxPrint_Destroy(printer);

    CLOSE_DISPS(gfxCtx);
}

void Custom_MapSelect_DrawLoadingScreen(MapSelectState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;
    GfxPrint printer;

    OPEN_DISPS(gfxCtx);

    Gfx_SetupDL28_Opa(gfxCtx);

    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, POLY_OPA_DISP);

    Custom_MapSelect_PrintLoadingMessage(this, &printer);

    POLY_OPA_DISP = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);

    CLOSE_DISPS(gfxCtx);
}

void Custom_MapSelect_Draw(MapSelectState* this) {
    GraphicsContext* gfxCtx = this->state.gfxCtx;

    func_8012CF0C(gfxCtx, true, true, 0, 0, 0);

    SET_FULLSCREEN_VIEWPORT(&this->view);
    View_Apply(&this->view, VIEW_ALL);
    if (!this->state.running) {
        Custom_MapSelect_DrawLoadingScreen(this);
    } else {
        Custom_MapSelect_DrawMenu(this);
    }
}

void Custom_MapSelect_Main(GameState* thisx) {
    MapSelectState* this = (MapSelectState*)thisx;

    Custom_MapSelect_UpdateMenu(this);
    Custom_MapSelect_Draw(this);
}

void Custom_MapSelect_Destroy(GameState* thisx) {
    ShrinkWindow_Destroy();
}

void Custom_MapSelect_Init(GameState* thisx) {
    MapSelectState* this = (MapSelectState*)thisx;

    this->state.main = Custom_MapSelect_Main;
    this->state.destroy = Custom_MapSelect_Destroy;

    //this->scenes = sScenes;
    this->topDisplayedScene = 0;
    this->currentScene = 0;
    this->pageDownStops[0] = 0;
    this->pageDownStops[1] = 19;
    this->pageDownStops[2] = 37;
    this->pageDownStops[3] = 51;
    this->pageDownStops[4] = 59;
    this->pageDownStops[5] = 73;
    this->pageDownStops[6] = 91;
    this->pageDownIndex = 0;
    this->opt = 0;
    //this->count = ARRAY_COUNT(sScenes);
    
    // revamped entrance system
    if (sSceneIndex[0] == 255)
    {
        int sceneIndex = 0;
        
        this->count = 0;
        for (int i = 0
            ; i < sizeof(sSceneNameBuffer) && this->count < MAX_SCENES
            ; ++i, ++sceneIndex
        )
        {
            if (sSceneNameBuffer[i] == '\0')
                continue;
            
            // add to list
            sSceneName[this->count] = sSceneNameBuffer + i;
            sSceneIndex[this->count] = sceneIndex;
            if (dREG(80) < 0 && sceneIndex == -dREG(80))
                dREG(80) = this->count;
            this->count += 1;
            
            while (i < sizeof(sSceneNameBuffer) && sSceneNameBuffer[i] != '\0')
                ++i;
        }
    }
    dREG(81) = (dREG(80) + this->count) % this->count;

    ShrinkWindow_Init();
    View_Init(&this->view, this->state.gfxCtx);
    this->view.flags = (0x08 | 0x02);
    this->verticalInputAccumulator = 0;
    this->verticalInput = 0;
    this->timerUp = 0;
    this->timerDown = 0;
    this->lockUp = false;
    this->lockDown = false;
    this->unk_274 = 0;

    if ((dREG(80) >= 0) && (dREG(80) < this->count)) {
        this->currentScene = dREG(80);
        this->topDisplayedScene = dREG(81);
        this->pageDownIndex = dREG(82);
    }

    GameState_SetFramerateDivisor(&this->state, 1);
    gSaveContext.save.cutsceneIndex = 0;
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.linkAge = 0;
}

u32 SelectFunctions[] = {
    0x53656c65, 0x63744675, 0x6e637469, 0x6f6e733a // "SelectFunctions:"
    , (u32)Custom_MapSelect_Init, (u32)Custom_MapSelect_Destroy//asdfasfx
};

