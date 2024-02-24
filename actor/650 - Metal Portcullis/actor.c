/*
 * File: z_obj_kzsaku.c
 * Overlay: ovl_Obj_Kzsaku
 * Description: Underwater grate
 */

#include "z_obj_kzsaku.h"

#define gUnderwaterGrateDL ((void*)0x06000040)
#define gUnderwaterGrateCol ((void*)0x06001118)

#define FLAGS (ACTOR_FLAG_10 | ACTOR_FLAG_20)

#define THIS ((ObjKzsaku*)thisx)

void ObjKzsaku_Init(Actor* thisx, PlayState* play);
void ObjKzsaku_Destroy(Actor* thisx, PlayState* play);
void ObjKzsaku_Update(Actor* thisx, PlayState* play);
void ObjKzsaku_Draw(Actor* thisx, PlayState* play);

void ObjKzsaku_SetupIdle(ObjKzsaku* this);
void func_80C08BBC(ObjKzsaku* this);
void func_80C08C84(ObjKzsaku* this);
void ObjKzsaku_Idle(ObjKzsaku* this, PlayState* play);
void ObjKzsaku_Rise(ObjKzsaku* this, PlayState* play);
void func_80C08CB0(ObjKzsaku* this, PlayState* play);

const ActorInit ZzrtlInitVars = {
    /**/ ACTOR_OBJ_KZSAKU,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_KZSAKU,
    /**/ sizeof(ObjKzsaku),
    /**/ ObjKzsaku_Init,
    /**/ ObjKzsaku_Destroy,
    /**/ ObjKzsaku_Update,
    /**/ ObjKzsaku_Draw,
};

void ObjKzsaku_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    ObjKzsaku* this = THIS;
    CollisionHeader* col = NULL;
    
    // non-zero means it moves down
    this->upDown = KZSAKU_GET_UP_DOWN(thisx) ? -1.0f : 1.0f;
    
    // scale
    this->scale = KZSAKU_GET_SCALE(thisx) * 0.003921569f;
    if (!this->scale)
        this->scale = 1.0f;
    
    // make the metal portcullis big enough in the fire area
    if (play->sceneId == 33)
        this->scale = 1.8f;

    Actor_SetScale(&this->dyna.actor, this->scale);
    DynaPolyActor_Init(&this->dyna, DYNA_TRANSFORM_POS);
    CollisionHeader_GetVirtual(gUnderwaterGrateCol, &col);

    this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, thisx, col);

    this->switchFlag = KZSAKU_GET_SWITCH_FLAG(thisx);
    this->raisedAmount = 0.0f;

    if (Flags_GetSwitch(play, this->switchFlag)) {
        func_80C08C84(this);
    } else {
        ObjKzsaku_SetupIdle(this);
    }
}

void ObjKzsaku_Destroy(Actor* thisx, PlayState* play) {
    ObjKzsaku* this = THIS;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

void ObjKzsaku_SetupIdle(ObjKzsaku* this) {
    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y;
    this->actionFunc = ObjKzsaku_Idle;
}

void ObjKzsaku_Idle(ObjKzsaku* this, PlayState* play) {
    if (Flags_GetSwitch(play, this->switchFlag)) {
        func_80C08BBC(this);
    }
}

void func_80C08BBC(ObjKzsaku* this) {
    this->actionFunc = ObjKzsaku_Rise;
}

void ObjKzsaku_Rise(ObjKzsaku* this, PlayState* play) {
    if (this->dyna.actor.csId != CS_ID_NONE) {
        if (CutsceneManager_IsNext(this->dyna.actor.csId)) {
            // No closeup of Link's face when Portcullis raises.
            //CutsceneManager_StartWithPlayerCs(this->dyna.actor.csId, &this->dyna.actor);
        } else {
            CutsceneManager_Queue(this->dyna.actor.csId);
        }
    }
    if (this->raisedAmount < 450.0f) {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
        this->raisedAmount += 15.0f;
    } else {
        func_80C08C84(this);
    }
    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y + this->raisedAmount * this->scale * this->upDown;
}

void func_80C08C84(ObjKzsaku* this) {
    this->timer = 0;
    this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y + 450.0f;
    this->actionFunc = func_80C08CB0;
}

void func_80C08CB0(ObjKzsaku* this, PlayState* play) {
    if (this->timer <= 20) {
        if (this->timer == 20) {
            if (CutsceneManager_GetCurrentCsId() == this->dyna.actor.csId) {
                CutsceneManager_Stop(this->dyna.actor.csId);
            }
            this->timer = 21;
        } else {
            this->timer++;
        }
    }
}

void ObjKzsaku_Update(Actor* thisx, PlayState* play) {
    ObjKzsaku* this = THIS;

    // fall back down if switch times out or is deactivated some other way
    if (this->raisedAmount > 0
        && !Flags_GetSwitch(play, this->switchFlag)
    )
    {
        Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_METALDOOR_SLIDE - SFX_FLAG);
        this->raisedAmount -= 15.0f;
        
        if (this->raisedAmount <= 0.0)
            this->raisedAmount = 0;
        
        this->dyna.actor.world.pos.y = this->dyna.actor.home.pos.y + this->raisedAmount * this->scale * this->upDown;
        this->actionFunc = ObjKzsaku_Idle;
        return;
    }
    
    this->actionFunc(this, play);
}

void ObjKzsaku_Draw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, gUnderwaterGrateDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
