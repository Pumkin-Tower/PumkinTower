
#include "global.h"
#include "overlays/gamestates/ovl_daytelop/z_daytelop.h"
#include "overlays/gamestates/ovl_opening/z_opening.h"

#define FLAGS 0

typedef struct EnCredits
{
    Actor actor;
    char **row;
    int rows;
    int scroll;
} EnCredits;

#define THIS ((EnCredits*)thisx)

#define LINE_HEIGHT 12

void Init(Actor* thisx, PlayState* play);
void Destroy(Actor* thisx, PlayState* play);
void Update(Actor* thisx, PlayState* play);
void Draw(Actor* thisx, PlayState* play);

const ActorInit ZzrtlInitVars = {
    /**/ 286,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ 286,
    /**/ sizeof(EnCredits),
    /**/ Init,
    /**/ Destroy,
    /**/ Update,
    /**/ Draw,
};

// credits are now sourced from object/286/credits.txt.zobj
/*
static char credits[] = {
    #include "credits.txt"
};
*/

// helper: draw debug text
#if 1
void DrawDebugText(PlayState* play, int x, int y, u32 color, const char *fmt, ...)
{
    if (!(color & 0xff))
    {
        color <<= 8;
        color |= 0xff;
    }
    
    // setup
    Gfx* prevDisplayList;
    Gfx* displayList;
    OPEN_DISPS(play->state.gfxCtx);
    prevDisplayList = POLY_OPA_DISP;
    displayList = Graph_GfxPlusOne(POLY_OPA_DISP);
    gSPDisplayList(OVERLAY_DISP++, displayList);
    
    // draw text
    va_list args;
    va_start(args, fmt);
    GfxPrint printer;
    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, displayList);
    GfxPrint_SetPosPx(&printer, x, y);
    GfxPrint_SetColor(&printer, color >> 24, color >> 16, color >> 8, color);
    GfxPrint_VPrintf(&printer, fmt, args);
    displayList = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);
    va_end(args);
    
    // finalize
    gSPEndDisplayList(displayList++);
    Graph_BranchDlist(prevDisplayList, displayList);
    POLY_OPA_DISP = displayList;
    CLOSE_DISPS(play->state.gfxCtx);
}
#endif

void Init(Actor* thisx, PlayState* play)
{
    EnCredits *this = THIS;
    char *credits = (void*)Lib_SegmentedToVirtual((void*)0x06000000);
    char *line = credits;
    
    // handle credits sequence
    if (!gSaveContext.dogParams)
    {
        // default save data when starting a game
        Sram_InitNewSave();
        gSaveContext.save.entrance = ENTRANCE(TWINMOLDS_LAIR, 0); // credits sequence
        gSaveContext.nextCutsceneIndex = gSaveContext.save.cutsceneIndex = 0;
        gSaveContext.sceneLayer = 0;
        gSaveContext.save.time = CLOCK_TIME(8, 0);
        gSaveContext.save.day = 1;
        gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
        gSaveContext.save.saveInfo.playerData.health = 0x80;
        gSaveContext.save.saveInfo.playerData.healthCapacity = 0x80;
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
        gSaveContext.gameMode = GAMEMODE_NORMAL;
        
        // jump to Pumkin Tower screen w/ tagline
        STOP_GAMESTATE(&play->state);
        SET_NEXT_GAMESTATE(&play->state, DayTelop_Init, sizeof(DayTelopState));
        SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, NA_BGM_END_CREDITS | SEQ_FLAG_ASYNC);
        gSaveContext.dogParams = 1;
        
        // cleanup
        Actor_Kill(thisx);
        return;
    }
    
    this->rows = 0;
    for (int i = 0; credits[i]; ++i)
    {
        while (credits[i++] == '\n')
            this->rows += 1;
    }
    
    this->row = ZeldaArena_Malloc(sizeof(*(this->row)) * this->rows);
    this->rows = 0;
    for (int i = 0; credits[i]; ++i)
    {
        while (credits[i] == '\n')
        {
            this->row[this->rows] = line;
            credits[i] = '\0';
            this->rows += 1;
            ++i;
            line = credits + i;
        }
    }
    
    // clear the first and last lines
    *this->row[0] = '\0';
    *this->row[this->rows - 1] = '\0';
}

void Destroy(Actor* thisx, PlayState* play)
{
}

void Update(Actor* thisx, PlayState* play)
{
    EnCredits *this = THIS;
    
    // prevent the player from pausing the game during the credits sequence
    play->gameOverCtx.state = GAMEOVER_DEATH_START;
    
    Interface_SetHudVisibility(HUD_VISIBILITY_NONE_INSTANT);
    
    if (this->scroll > -(this->rows * LINE_HEIGHT + SCREEN_HEIGHT / 4))
        this->scroll -= 1;
}

void Draw(Actor* thisx, PlayState* play)
{
    EnCredits *this = THIS;
    
    Environment_FillScreen(play->state.gfxCtx, 0, 0, 0, 0xff, FILL_SCREEN_OPA | FILL_SCREEN_XLU);
    const int x = (((SCREEN_WIDTH / 8) - 24) / 2) * 8;
    int y = SCREEN_HEIGHT;
    
    y += this->scroll;
    
    for (int i = 0; i < this->rows; ++i, y += LINE_HEIGHT)
        if (y > 0 && y < SCREEN_HEIGHT)
            DrawDebugText(play, x, y, -1, this->row[i]);
}

