/*
 * File: z_platforms.c
 * Overlay: ovl_Platforms
 * Description: Various floating platforms, from the Hylian Modding actor pack.
 */

#include "platforms.h"

#define PLATFORM_TYPE(this) ((this->dyna.actor.params >> 0xC) & 0xF) // 0xF000
#define SWITCH_FLAG(this) (this->dyna.actor.params & 0x3F)           // 0x00FF
#define TIMER_DURATION(this) (this->dyna.actor.home.rot.x)           // <= 10 minutes (600)
#define THIS ((Platforms*)thisx)

/**
 * Params:
 *  0xF000 = Type
 *     0x0 = Square Wooden Platform
 *     0x1 = Square Wooden Platform (Checkered Pattern)
 *     0x2 = Square Stone Platform
 *     0x3 = Square Stone Platform (Checkered Pattern)
 *     0x4 = Square Grass Platform
 *     0x5 = Square Ice Platform
 *     0x6 = Hexagonal Grass Platform
 *     0x7 = Hexagonal Ice Platform
 *     0x8 = Cone Floating Grass Platform
 *
 *  0x00FF = Switch Flag
 *
 *  Rot X > 0 = Duration timer for spawned platforms (in seconds)
 *
 *  All platform types (except for the Cone Floating Type) will spawn on switch flag activation. After the specified
 * timer amount, they unset the switch and disappear again until the flag is once again set.
 *
 *  Default (Rot X = 0) timer is 10 seconds. You can go up to 10 minutes (600).
 *
 *  Square platforms = 200 units
 *  Hex platforms    = 328 units
 *  Cone platform    = 206 units
 */

#define FLAGS (ACTOR_FLAG_10 | ACTOR_FLAG_20)

#define SECONDS_TO_FRAMES(sec) (sec * (60 / R_UPDATE_RATE))
static u16 sTimer = 0;

void Platforms_Init(Actor* thisx, PlayState* play);
void Platforms_Destroy(Actor* thisx, PlayState* play);
void Platforms_Update(Actor* thisx, PlayState* play);
void Platforms_Draw(Actor* thisx, PlayState* play);

const ActorInit ZzrtlInitVars = {
    55,
    ACTORCAT_BG,
    FLAGS,
    45,
    sizeof(Platforms),
    (ActorFunc)Platforms_Init,
    (ActorFunc)Platforms_Destroy,
    (ActorFunc)Platforms_Update,
    (ActorFunc)Platforms_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneForward, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 500, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 1000, ICHAIN_STOP),
};

void Platforms_Init(Actor* thisx, PlayState* play) {
    Platforms* this = THIS;
    CollisionHeader* colHeader = NULL;
    u16 seconds;

    Actor_ProcessInitChain(thisx, sInitChain);
    DynaPolyActor_Init(&this->dyna, 3);

    CollisionHeader_GetVirtual((void*)(0x06000600)/* (void*)(0x06001860) old cog coloffset*/, &colHeader);

    DynaPolyActor_LoadMesh(play, &this->dyna, colHeader);//this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);

    Actor_SetScale(&this->dyna.actor, 0.035f);
}

void Platforms_Destroy(Actor* thisx, PlayState* play) {
    Platforms* this = THIS;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
}

void Platforms_Update(Actor* thisx, PlayState* play) {
    Platforms* this = THIS;

    float speed = 100.0f;
    this->dyna.actor.shape.rot.y += speed;
    this->dyna.actor.home.rot.y += speed;
    Actor_PlaySfx_Flagged(&this->dyna.actor, NA_SE_EV_WINDMILL_LEVEL - SFX_FLAG);
}

void Platforms_Draw(Actor* thisx, PlayState* play) {
    Gfx_DrawDListOpa(play, (void*)(0x06001CD0)/* (void*)(0x06003400) old DL offset*/);
}