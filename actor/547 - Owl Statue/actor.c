/*
 * File: z_obj_warpstone.c
 * Overlay: ovl_Obj_Warpstone
 * Description: Owl Statue
 */

#include "z_obj_warpstone.h"
#include "objects/object_sek/object_sek.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

#define THIS ((ObjWarpstone*)thisx)

asm(" \
    gOwlStatueClosedDL = 0x060001D0; \
    gOwlStatueOpenedDL = 0x06003770; \
");

static void TextOverride(PlayState* play, const char* string)
{
    Font* font = &play->msgCtx.font;
    char *tmp = font->msgBuf.schar + 11;
    
    for (;;)
    {
        *tmp = *string;
        
        if (*tmp == 0xbf)
            break;
        
        ++tmp;
        ++string;
    }
}

void ObjWarpstone_Init(Actor* thisx, PlayState* play);
void ObjWarpstone_Destroy(Actor* thisx, PlayState* play);
void ObjWarpstone_Update(Actor* thisx, PlayState* play);
void ObjWarpstone_Draw(Actor* thisx, PlayState* play2);
void ObjWarpstone_SetupAction(ObjWarpstone* this, ObjWarpstoneActionFunc actionFunc);
s32 ObjWarpstone_ClosedIdle(ObjWarpstone* this, PlayState* play);
s32 ObjWarpstone_BeginOpeningCutscene(ObjWarpstone* this, PlayState* play);
s32 ObjWarpstone_PlayOpeningCutscene(ObjWarpstone* this, PlayState* play);
s32 ObjWarpstone_OpenedIdle(ObjWarpstone* this, PlayState* play);

ActorInit ZzrtlInitVars = {
    /**/ ACTOR_OBJ_WARPSTONE,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_SEK,
    /**/ sizeof(ObjWarpstone),
    /**/ ObjWarpstone_Init,
    /**/ ObjWarpstone_Destroy,
    /**/ ObjWarpstone_Update,
    /**/ ObjWarpstone_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_METAL,
        AT_NONE,
        AC_ON | AC_HARD | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK2,
        { 0x00100000, 0x00, 0x00 },
        { 0x01000202, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_ON | BUMP_HOOKABLE,
        OCELEM_ON,
    },
    { 20, 60, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(targetMode, TARGET_MODE_1, ICHAIN_STOP),
};

static Gfx* sOwlStatueDLs[] = { gOwlStatueClosedDL, gOwlStatueOpenedDL };

void ObjWarpstone_SetupAction(ObjWarpstone* this, ObjWarpstoneActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

void ObjWarpstone_Init(Actor* thisx, PlayState* play) {
    ObjWarpstone* this = THIS;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    Collider_InitAndSetCylinder(play, &this->collider, &this->dyna.actor, &sCylinderInit);
    Actor_SetFocus(&this->dyna.actor, 40.0f);

    // default
    ObjWarpstone_SetupAction(this, ObjWarpstone_OpenedIdle);
    return;
    
    if (!OBJ_WARPSTONE_IS_ACTIVATED(OBJ_WARPSTONE_GET_ID(&this->dyna.actor))) {
        ObjWarpstone_SetupAction(this, ObjWarpstone_ClosedIdle);
    } else {
        ObjWarpstone_SetupAction(this, ObjWarpstone_OpenedIdle);
        this->modelIndex = SEK_MODEL_OPENED;
    }
}

void ObjWarpstone_Destroy(Actor* thisx, PlayState* play) {
    ObjWarpstone* this = THIS;

    Collider_DestroyCylinder(play, &this->collider);
}

s32 ObjWarpstone_ClosedIdle(ObjWarpstone* this, PlayState* play) {
    if (this->collider.base.acFlags & AC_HIT) {
        ObjWarpstone_SetupAction(this, ObjWarpstone_BeginOpeningCutscene);
        return true;
    } else {
        /*Ye who hold the sacred sword, leave proof of our encounter.*/
        this->dyna.actor.textId = 0xC00;
        return false;
    }
}

s32 ObjWarpstone_BeginOpeningCutscene(ObjWarpstone* this, PlayState* play) {
    if ((this->dyna.actor.csId <= CS_ID_NONE) || CutsceneManager_IsNext(this->dyna.actor.csId)) {
        CutsceneManager_StartWithPlayerCs(this->dyna.actor.csId, &this->dyna.actor);
        ObjWarpstone_SetupAction(this, ObjWarpstone_PlayOpeningCutscene);
        Actor_PlaySfx(&this->dyna.actor, NA_SE_EV_OWL_WARP_SWITCH_ON);
    } else {
        CutsceneManager_Queue(this->dyna.actor.csId);
    }
    return true;
}

s32 ObjWarpstone_PlayOpeningCutscene(ObjWarpstone* this, PlayState* play) {
    if (this->openingCSTimer++ >= OBJ_WARPSTONE_TIMER_ACTIVATE_THRESHOLD) {
        CutsceneManager_Stop(this->dyna.actor.csId);
        Sram_ActivateOwl(OBJ_WARPSTONE_GET_ID(&this->dyna.actor));
        ObjWarpstone_SetupAction(this, ObjWarpstone_OpenedIdle);
    } else if (this->openingCSTimer < OBJ_WARPSTONE_TIMER_OPEN_THRESHOLD) {
        Math_StepToF(&this->dyna.actor.velocity.x, 0.01f, 0.001f);
        Math_StepToS(&this->dyna.actor.home.rot.x, 0xFF, 0x12);
    } else {
        Math_StepToF(&this->dyna.actor.velocity.x, 20.0f, 0.01f);
        if (this->dyna.actor.velocity.x > 0.2f) {
            this->modelIndex = SEK_MODEL_OPENED;
            Math_StepToS(&this->dyna.actor.home.rot.x, 0, 0x14);
        }
    }
    return true;
}

s32 ObjWarpstone_OpenedIdle(ObjWarpstone* this, PlayState* play) {
    /*You can save your progress and quit here.*/
    this->dyna.actor.textId = 0xC01;
    return false;
}

void ObjWarpstone_Update(Actor* thisx, PlayState* play) {
    ObjWarpstone* this = THIS;
    s32 pad;
    
    // don't exit to title screen; instead, display message announcing progress was saved successfully
    if ((play->msgCtx.msgMode == MSGMODE_OWL_SAVE_1
            || play->msgCtx.msgMode == MSGMODE_NEW_CYCLE_0
            || play->msgCtx.msgMode == MSGMODE_NEW_CYCLE_1
        )
    )
    {
        if (play->msgCtx.msgMode == MSGMODE_OWL_SAVE_1)
        {
            play->msgCtx.msgMode = MSGMODE_NEW_CYCLE_0;
            
            Message_StartTextbox(play, 0x4444, NULL);
            if (gSaveContext.save.saveInfo.playerData.playerName[0])
                TextOverride(play, "Saved progress successfully.\x00\xbf");
            else
                TextOverride(play, "Please wait...\x1a");
        }
        else if (play->msgCtx.msgMode = MSGMODE_NEW_CYCLE_1)
        {
            play->msgCtx.msgMode = MSGMODE_NONE;
        }
    }
    
    if (this->waitReset)
    {
        if (!(this->waitReset -= 1))
        {
            gSaveContext.gameMode = GAMEMODE_OWL_SAVE;
            play->transitionTrigger = TRANS_TRIGGER_START;
            play->transitionType = TRANS_TYPE_FADE_BLACK;
            play->nextEntrance = ENTRANCE(CUTSCENE, 0);
            gSaveContext.save.cutsceneIndex = 0;
            gSaveContext.sceneLayer = 0;
        }
        
        return;
    }

    if (this->isTalking) {
        if (Actor_TextboxIsClosing(&this->dyna.actor, play)) {
            this->isTalking = false;
        } else if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
            if (play->msgCtx.choiceIndex != 0) {
                Audio_PlaySfx_MessageDecide();
                if (play->msgCtx.choiceIndex == 1) // save progress
                {
                    // erase all progress
                    if (play->msgCtx.currentTextId == 0x4444)
                    {
                        // invalidate this save file
                        bzero(&gSaveContext.cycleSceneFlags, sizeof(gSaveContext.cycleSceneFlags));
                        gSaveContext.save.saveInfo.playerData.playerName[0] = '\0';
                        play->sceneId = 0;
                        
                        // wait some time
                        this->waitReset = 20 * 3;
                    }
                    
                    // hack for persistence across reboots
                    gSaveContext.save.saveInfo.unk_F40 = gSaveContext.options.zTargetSetting;
                    
                    // handle writing save data
                    play->msgCtx.msgMode = MSGMODE_OWL_SAVE_0;
                }
                else if (play->msgCtx.choiceIndex == 2) // erase progress
                {
                    // are you sure?
                    Message_StartTextbox(play, 0x4444, NULL);
                    TextOverride(play, "Are you sure you want\x11""to \x01""erase all progress\x00""?\x11\xc2\x02""No\x11""Yes\x00\xbf");
                }
                play->msgCtx.unk120D6 = 0;
                play->msgCtx.unk120D4 = 0;
                gSaveContext.save.owlSaveLocation = OBJ_WARPSTONE_GET_ID(&this->dyna.actor);
            } else {
                Message_CloseTextbox(play);
            }
        }
    } else if (Actor_ProcessTalkRequest(&this->dyna.actor, &play->state)) {
        this->isTalking = true;
        TextOverride(play, "What brings you here?\x11\xc3\x02""Never mind\x11""Save current progress\x11""Erase all progress\x00\xbf");
    } else if (!this->actionFunc(this, play)) {
        Actor_OfferTalkNearColChkInfoCylinder(&this->dyna.actor, play);
    }

    Collider_ResetCylinderAC(play, &this->collider.base);
    Collider_UpdateCylinder(&this->dyna.actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
}

void ObjWarpstone_Draw(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    ObjWarpstone* this = THIS;
    
    // custom model
    Gfx_DrawDListOpa(play, (Gfx*)0x06001610); // base
    Gfx_DrawDListOpa(play, (Gfx*)0x06001848); // feather
    return;

    Gfx_DrawDListOpa(play, sOwlStatueDLs[this->modelIndex]);
    
    if (this->dyna.actor.home.rot.x != 0) {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        Matrix_Translate(this->dyna.actor.world.pos.x, this->dyna.actor.world.pos.y + 34.0f,
                         this->dyna.actor.world.pos.z, MTXMODE_NEW);
        Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
        Matrix_Translate(0.0f, 0.0f, 30.0f, MTXMODE_APPLY);
        Matrix_Scale(this->dyna.actor.velocity.x, this->dyna.actor.velocity.x, this->dyna.actor.velocity.x,
                     MTXMODE_APPLY);
        Matrix_Push();
        gDPPipeSync(POLY_XLU_DISP++);
        gDPSetPrimColor(POLY_XLU_DISP++, 128, 128, 255, 255, 200, this->dyna.actor.home.rot.x);
        gDPSetEnvColor(POLY_XLU_DISP++, 100, 200, 0, 255);
        Matrix_RotateZF(BINANG_TO_RAD_ALT2((play->gameplayFrames * 1500) & 0xFFFF), MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);
        Matrix_Pop();
        Matrix_RotateZF(BINANG_TO_RAD_ALT2(~((play->gameplayFrames * 1200) & 0xFFFF)), MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
