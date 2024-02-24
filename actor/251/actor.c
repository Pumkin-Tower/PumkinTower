#include "global.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

#define THIS ((EnAObj*)thisx)

void Init(Actor* thisx, PlayState* play);
void Destroy(Actor* thisx, PlayState* play);
void Update(Actor* thisx, PlayState* play);
void Draw(Actor* thisx, PlayState* play);

void Idle(EnAObj* this, PlayState* play);
void Talk(EnAObj* this, PlayState* play);

ActorInit ZzrtlInitVars = {
    /**/ ACTOR_EN_A_OBJ,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnAObj),
    /**/ Init,
    /**/ Destroy,
    /**/ Update,
    /**/ Draw,
};

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

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_NONE,
        OCELEM_ON,
    },
    { 25, 60, 0, { 0, 0, 0 } },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(targetMode, TARGET_MODE_0, ICHAIN_STOP),
};

void Init(Actor* thisx, PlayState* play) {
    EnAObj* this = THIS;

    this->actor.textId = AOBJ_GET_TEXTID(&this->actor);
    this->actor.params = AOBJ_GET_TYPE(&this->actor);
    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 0, ActorShadow_DrawCircle, 12);
    Collider_InitAndSetCylinder(play, &this->collision, &this->actor, &sCylinderInit);
    Collider_UpdateCylinder(&this->actor, &this->collision);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->actionFunc = Idle;
}

void Destroy(Actor* thisx, PlayState* play) {
    EnAObj* this = THIS;

    Collider_DestroyCylinder(play, &this->collision);
}

void Idle(EnAObj* this, PlayState* play) {
    s32 yawDiff;

    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        this->actionFunc = Talk;
    } else {
        yawDiff = ABS_ALT((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y));

        if ((yawDiff < 0x2800) || ((this->actor.params == AOBJ_SIGNPOST_ARROW) && (yawDiff > 0x5800))) {
            Actor_OfferTalkNearColChkInfoCylinder(&this->actor, play);
        }
    }
}

void Talk(EnAObj* this, PlayState* play)
{
    if (Actor_TextboxIsClosing(&this->actor, play))
    {
        this->actionFunc = Idle;
    }
    // special case for z-targeting selection
    else if (play->msgCtx.currentTextId == 0x030c
        && Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE
        && Message_ShouldAdvance(play)
    )
    {
        char *statusMessage;
        
        Audio_PlaySfx_MessageDecide();
        
        // hold
        if (play->msgCtx.choiceIndex == 0)
        {
            statusMessage = "Z-Targeting style is now \x02""Hold\x00"".\xbf";
            gSaveContext.options.zTargetSetting = 1; // Z-Targeting 'Hold' option
        }
        // switch
        else
        {
            statusMessage = "Z-Targeting style is now \x02""Switch\x00"".\xbf";
            gSaveContext.options.zTargetSetting = 0; // Z-Targeting 'Switch' option
        }
        
        Message_StartTextbox(play, 0x4444, &this->actor);
        TextOverride(play, statusMessage);
    }
}

void Update(Actor* thisx, PlayState* play) {
    EnAObj* this = THIS;
    Player* player = GET_PLAYER(play);
    float yofs = 0.0f;
    
    // change sign y offset based on player height
    if (player->transformation == PLAYER_FORM_HUMAN
        || player->transformation == PLAYER_FORM_DEKU
    )
        yofs = -15.0f;
    if (thisx->params) // arrow signs are fine at their defaults though
        yofs = 0.0f;
    this->actor.world.pos.y = this->actor.home.pos.y + yofs;
    thisx->world.pos.y = thisx->home.pos.y + yofs;

    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, 45.0f);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collision.base);
}

static Gfx* sDLists[] = {
    gSignRectangularDL, // AOBJ_SIGNPOST_OBLONG
    gSignDirectionalDL, // AOBJ_SIGNPOST_ARROW
};

void Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, sDLists[thisx->params]);
}
