//
// code1
//
// extra 'code' file for overriding certain user library functions
// overwrites Message_DrawTextDefault, the unused Japanese text handlers
//

#include "global.h"

extern void (*sSceneCmdHandlers[SCENE_CMD_MAX])(PlayState*, SceneCmd*);
asm("sSceneCmdHandlers = 0x801c26a8;");

extern void func_80169ECC(PlayState* this);
extern void Play_InitScene(PlayState* this, s32 spawn);

#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

/**
 * Executes all of the commands in a scene or room header.
 */
void Custom_Scene_ExecuteCommands(PlayState* play, SceneCmd* sceneCmd)
{
    u32 cmdId;
#if 0
    u8 handled[SCENE_CMD_MAX];
    
    for (int i = 0; i < SCENE_CMD_MAX; ++i)
        handled[i] = false;
#endif
    
    while (true)
    {
        cmdId = sceneCmd->base.code;
        
        if (cmdId == SCENE_CMD_ID_END)
            break;
        
        // hotfix some actors
        if (cmdId == SCENE_CMD_ID_ACTOR_LIST)
        {
            struct {
                u16 id;
                Vec3s pos;
                Vec3s rot;
                u16 params;
            } *actor = (void*)(Lib_SegmentedToVirtual(sceneCmd->actorList.segment));
            int num = sceneCmd->actorList.num;
            
            for (int i = 0; i < num; ++i, ++actor)
            {
                switch (actor->id)
                {
                    case ACTOR_OBJ_ICE_POLY:
                    case ACTOR_OBJ_SNOWBALL:
                        actor->rot.y = 0x007f;
                        break;
                    
                    // redirect existing signposts to custom variation
                    case ACTOR_EN_A_OBJ:
                        actor->id = 251;
                        break;
                }
            }
        }
        #if 1
        // new entrance system courtesy of z64me
        // how it works: in sharpocarina, use exits 0000, 0001, 0002, etc.
        //               those will correspond to exit data defined within
        //               the (optional) exit-data.txt file in the same folder
        //               as the specified zscene
        else if (cmdId == SCENE_CMD_ID_EXIT_LIST && sceneCmd->base.data1)
        {
            extern SceneEntranceTableEntry sSceneEntranceTable[0x6f];
            asm("sSceneEntranceTable = 0x801C5720;");
            int num = sceneCmd->base.data1;
            
            static EntranceTableEntry data[32] = {0};
				static EntranceTableEntry* table[ARRAY_COUNT(data)] = {0};
            
            if (!table[0])
            	for (int i = 0; i < ARRAY_COUNT(data); ++i)
            		table[i] = &data[i];
            
            // XXX old method
            /*
            Lib_MemCpy(
                &sSceneEntranceTable[0]
                , Lib_SegmentedToVirtual((void*)(sceneCmd->base.data2 + num * (2 + 4 + 4))) // skip index data
                , sizeof(*sSceneEntranceTable)
            );
            
            sSceneEntranceTable[0].table = Lib_SegmentedToVirtual(sSceneEntranceTable[0].table);
            
            for (int i = 0; i < num; ++i)
                sSceneEntranceTable[0].table[i] = Lib_SegmentedToVirtual(sSceneEntranceTable[0].table[i]);
            */
            
            // migrate everything to static storage so it persists across scenes
            Lib_MemCpy(data, Lib_SegmentedToVirtual((void*)(sceneCmd->base.data2 + num * 2)), num * 4);
            
            sSceneEntranceTable[0].table = table;
        }
        // guarantee a minimum number of spawn points for Link
        // (fixes a crash when voiding out in certain custom scenes,
        // likely due to hardcoded shenanigans employed by Nintendo)
        // (also likely because my new routing system doesn't play
        // completely nicely with Nintendo's hardcoded shenanigans...)
        #define atLeast 17 // 17 is the max number of spawns in any MM scene
        #define stride  16 // 16 bytes per record
        else if (cmdId == SCENE_CMD_ID_SPAWN_LIST && sceneCmd->spawnList.data1 < atLeast)
        {
            static u8 wow[atLeast * stride] __attribute__ ((aligned (2))) = {0};
            SCmdSpawnList* this = &sceneCmd->spawnList;
            u8 count = this->data1;
            
            // copy the records that we do have
            Lib_MemCpy(
                wow
                , Lib_SegmentedToVirtual(this->segment)
                , count * stride
            );
            
            // update variable for each so he doesn't run too fast when entering scenes
            u8* params = wow + stride - 2;
            for (int i = 0; i < count; ++i, params += stride)
                if (*params == 0x0f)
                    *params = 0x0e;
            
            // all the additional records will be duplicates of the last record
            u8* tail = wow + count * stride;
            while (count < atLeast)
            {
                Lib_MemCpy(tail, tail - stride, stride);
                
                tail += stride;
                count += 1;
            }
            
            // update the header to reflect these changes
            this->data1 = atLeast;
            this->segment = wow;
        }
        #undef atLeast
        #undef stride
        #endif
        
        sSceneCmdHandlers[cmdId](play, sceneCmd);
//      handled[cmdId] = true;
        
        sceneCmd++;
    }
    
#if 0
    // is scene
    // adapted for this project using code provided by Zel640
    if (handled[SCENE_CMD_ID_ROOM_LIST])
    {
        if (!handled[SCENE_CMD_ID_ACTOR_CUTSCENE_LIST])
        {
            // TODO: Make this more concise
            static ActorCutscene defaultActorCutscene[] = {
                { 700, -1, -3, -1, 1, 0, 255, 0, 0, 27 },  { 600, -1, -2, -1, 2, 0, 255, 0, 0, 27 },
                { 700, -1, -4, -1, 3, 0, 255, 0, 0, 27 },  { 700, -1, -5, -1, 4, 0, 255, 0, 0, 27 },
                { 500, -1, -7, -1, 5, 0, 255, 0, 0, 32 },  { 400, -1, -11, -1, 6, 0, 255, 0, 1, 32 },
                { 100, -1, -8, -1, 7, 0, 255, 0, 0, 32 },  { 200, -1, -9, -1, 8, 0, 255, 0, 0, 32 },
                { 800, -1, -6, -1, -1, 0, 255, 0, 0, 32 }, { 850, -1, 0, -1, 10, 0, 255, 0, 1, 32 },
                { 860, -1, 1, -1, -1, 0, 255, 0, 1, 32 },  { -1, -1, -1, 0, -1, 0, 255, 0, 0, 0 },
            };
            CutsceneManager_Init(play, defaultActorCutscene, ARRAY_COUNT(defaultActorCutscene));
        }
        if (!handled[SCENE_CMD_ID_ACTOR_CUTSCENE_CAM_LIST])
        {
            // TODO: Make this more concise
            static Vec3s defaultActorCsCamPoints[] = {
                { 1363, 446, 794 },  { 910, 17112, 0 },  { -1, -1, -1 },
                { 1171, 367, -348 }, { 728, -16384, 0 }, { -1, -1, -1 },
            };
            static ActorCsCamInfo defaultActorCsCamInfo[] = {
                { 31, 3, &defaultActorCsCamPoints[0] },
                { 31, 3, &defaultActorCsCamPoints[3] },
            };

            play->actorCsCamList = defaultActorCsCamInfo;
        }
        if (false && !handled[SCENE_CMD_ID_MAP_DATA])
        {
            MapDisp_Init(play);
            MapDisp_InitMapData(play, NULL);
        }
        if (!handled[SCENE_CMD_ID_ANIMATED_MATERIAL_LIST])
        {
            play->sceneMaterialAnims = NULL;
        }
    }
#endif
}


static bool sDeathRespawn = false;
static bool sVoidRespawn = false;

// Gameplay_LoadToLastEntrance ?
// Used by game_over and Test7
void custom_func_80169F78(GameState* thisx) {
    PlayState* this = (PlayState*)thisx;

    this->nextEntrance = gSaveContext.respawn[RESPAWN_MODE_TOP].entrance;
    sDeathRespawn = true;
    gSaveContext.respawnFlag = -1;
    func_80169ECC(this);
    this->transitionTrigger = TRANS_TRIGGER_START;
    this->transitionType = TRANS_TYPE_FADE_BLACK;
}

void Custom_Play_SpawnScene(PlayState* this, s32 sceneId, s32 spawn) {
    static s32 sceneIdPrev;
    static s32 spawnPrev;
    
    if (sDeathRespawn)
    {
        sceneId = sceneIdPrev;
        spawn = spawnPrev;
        sDeathRespawn = false;
        gSaveContext.respawnFlag = -2; // guarantee title card and entrance anim on scene reset
    }
    else if (sVoidRespawn)
    {
        sceneId = sceneIdPrev;
        sVoidRespawn = false;
    }
    else
    {
        sceneIdPrev = sceneId;
        spawnPrev = spawn;
    }
    
    SceneTableEntry* scene = &gSceneTable[sceneId];

    scene->unk_D = 0;
    this->loadedScene = scene;
    this->sceneId = sceneId;
    this->sceneConfig = scene->drawConfig;
    this->sceneSegment = Play_LoadFile(this, &scene->segment);
    scene->unk_D = 0;
    gSegments[2] = OS_K0_TO_PHYSICAL(this->sceneSegment);
    Play_InitScene(this, spawn);
    Room_AllocateAndLoad(this, &this->roomCtx);
}

// Gameplay_TriggerVoidOut ?
// Used by Player, Ikana_Rotaryroom, Bji01, Kakasi, LiftNuts, Test4, Warptag, WarpUzu, Roomtimer
void Custom_func_80169EFC(GameState* thisx) {
    PlayState* this = (PlayState*)thisx;

    sVoidRespawn = true;
    gSaveContext.respawn[RESPAWN_MODE_DOWN].tempSwitchFlags = this->actorCtx.sceneFlags.switches[2];
    gSaveContext.respawn[RESPAWN_MODE_DOWN].unk_18 = this->actorCtx.sceneFlags.collectible[1];
    gSaveContext.respawn[RESPAWN_MODE_DOWN].tempCollectFlags = this->actorCtx.sceneFlags.collectible[2];
    this->nextEntrance = gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance;
    gSaveContext.respawnFlag = 1;
    func_80169ECC(this);
    this->transitionTrigger = TRANS_TRIGGER_START;
    this->transitionType = TRANS_TYPE_FADE_BLACK;
}

// patched to allow dpad lens of truth
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
extern s16 sMagicMeterOutlinePrimRed;
extern s16 sMagicMeterOutlinePrimGreen;
extern s16 sMagicMeterOutlinePrimBlue;
extern s16 sMagicBorderRatio;
extern s16 sMagicBorderStep;
extern void Magic_FlashMeterBorder(void);
void Custom_Magic_Update(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 magicCapacityTarget;

    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
        Magic_FlashMeterBorder();
    }

    switch (gSaveContext.magicState) {
        case MAGIC_STATE_STEP_CAPACITY:
            // Step magicCapacity to the capacity determined by magicLevel
            // This changes the width of the magic meter drawn
            magicCapacityTarget = gSaveContext.save.saveInfo.playerData.magicLevel * MAGIC_NORMAL_METER;
            if (gSaveContext.magicCapacity != magicCapacityTarget) {
                if (gSaveContext.magicCapacity < magicCapacityTarget) {
                    gSaveContext.magicCapacity += 0x10;
                    if (gSaveContext.magicCapacity > magicCapacityTarget) {
                        gSaveContext.magicCapacity = magicCapacityTarget;
                    }
                } else {
                    gSaveContext.magicCapacity -= 0x10;
                    if (gSaveContext.magicCapacity <= magicCapacityTarget) {
                        gSaveContext.magicCapacity = magicCapacityTarget;
                    }
                }
            } else {
                // Once the capacity has reached its target,
                // follow up by filling magic to magicFillTarget
                gSaveContext.magicState = MAGIC_STATE_FILL;
            }
            break;

        case MAGIC_STATE_FILL:
            // Add magic until magicFillTarget is reached
            gSaveContext.save.saveInfo.playerData.magic += 0x10;

            if ((gSaveContext.gameMode == GAMEMODE_NORMAL) && (gSaveContext.sceneLayer < 4)) {
                Audio_PlaySfx(NA_SE_SY_GAUGE_UP - SFX_FLAG);
            }

            if (((void)0, gSaveContext.save.saveInfo.playerData.magic) >= ((void)0, gSaveContext.magicFillTarget)) {
                gSaveContext.save.saveInfo.playerData.magic = gSaveContext.magicFillTarget;
                gSaveContext.magicState = MAGIC_STATE_IDLE;
            }
            break;

        case MAGIC_STATE_CONSUME_SETUP:
            // Sets the speed at which magic border flashes
            sMagicBorderRatio = 2;
            gSaveContext.magicState = MAGIC_STATE_CONSUME;
            break;

        case MAGIC_STATE_CONSUME:
            // Consume magic until target is reached or no more magic is available
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                gSaveContext.save.saveInfo.playerData.magic =
                    ((void)0, gSaveContext.save.saveInfo.playerData.magic) - ((void)0, gSaveContext.magicToConsume);
                if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                    gSaveContext.save.saveInfo.playerData.magic = 0;
                }
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
                sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
            }
            fallthrough;
            // fallthrough (flash border while magic is being consumed)
        case MAGIC_STATE_METER_FLASH_1:
        case MAGIC_STATE_METER_FLASH_2:
        case MAGIC_STATE_METER_FLASH_3:
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_RESET:
            sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;

        case MAGIC_STATE_CONSUME_LENS:
            // Slowly consume magic while Lens of Truth is active
            if ((play->pauseCtx.state == PAUSE_STATE_OFF) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == MSGMODE_NONE) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF) &&
                !Play_InCsMode(play)) {

                if ((gSaveContext.save.saveInfo.playerData.magic == 0) ||
                    ((Player_GetEnvironmentalHazard(play) >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR) &&
                     (Player_GetEnvironmentalHazard(play) <= PLAYER_ENV_HAZARD_UNDERWATER_FREE)) ||
                    (false && (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) != ITEM_LENS_OF_TRUTH) && // allow dpad lens of truth
                     (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) != ITEM_LENS_OF_TRUTH) &&
                     (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) != ITEM_LENS_OF_TRUTH)) ||
                    !play->actorCtx.lensActive) {
                    // Deactivate Lens of Truth and set magic state to idle
                    play->actorCtx.lensActive = false;
                    Audio_PlaySfx(NA_SE_SY_GLASSMODE_OFF);
                    gSaveContext.magicState = MAGIC_STATE_IDLE;
                    sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
                    break;
                }

                interfaceCtx->magicConsumptionTimer--;
                if (interfaceCtx->magicConsumptionTimer == 0) {
                    if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                        gSaveContext.save.saveInfo.playerData.magic--;
                    }
                    interfaceCtx->magicConsumptionTimer = 80;
                }
            }
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_CONSUME_GORON_ZORA_SETUP:
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                gSaveContext.save.saveInfo.playerData.magic -= 2;
            }
            if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                gSaveContext.save.saveInfo.playerData.magic = 0;
            }
            gSaveContext.magicState = MAGIC_STATE_CONSUME_GORON_ZORA;
            // fallthrough
        case MAGIC_STATE_CONSUME_GORON_ZORA:
            if ((play->pauseCtx.state == PAUSE_STATE_OFF) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == 0) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
                if (!Play_InCsMode(play)) {
                    interfaceCtx->magicConsumptionTimer--;
                    if (interfaceCtx->magicConsumptionTimer == 0) {
                        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                            gSaveContext.save.saveInfo.playerData.magic--;
                        }
                        if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                            gSaveContext.save.saveInfo.playerData.magic = 0;
                        }
                        interfaceCtx->magicConsumptionTimer = 10;
                    }
                }
            }
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_CONSUME_GIANTS_MASK:
            if ((play->pauseCtx.state == PAUSE_STATE_OFF) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == MSGMODE_NONE) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
                if (!Play_InCsMode(play)) {
                    interfaceCtx->magicConsumptionTimer--;
                    if (interfaceCtx->magicConsumptionTimer == 0) {
                        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                            gSaveContext.save.saveInfo.playerData.magic--;
                        }
                        if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                            gSaveContext.save.saveInfo.playerData.magic = 0;
                        }
                        interfaceCtx->magicConsumptionTimer = R_MAGIC_CONSUME_TIMER_GIANTS_MASK;
                    }
                }
            }
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
                Magic_FlashMeterBorder();
            }
            break;

        default:
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;
    }
}

void Custom_Actor_Init(Actor* actor, PlayState* play) {
    Actor_SetWorldToHome(actor);
    Actor_SetShapeRotToWorld(actor);
    Actor_SetFocus(actor, 0.0f);
    Math_Vec3f_Copy(&actor->prevPos, &actor->world.pos);
    Actor_SetScale(actor, 0.01f);
    actor->targetMode = TARGET_MODE_3;
    actor->terminalVelocity = -20.0f;

    actor->xyzDistToPlayerSq = FLT_MAX;
    actor->uncullZoneForward = 1000.0f;
    actor->uncullZoneScale = 350.0f;
    actor->uncullZoneDownward = 700.0f;

    // XXX giving every targetable actor tagged as an enemy a tatl hint
    actor->hintId = (actor->category == ACTORCAT_ENEMY && (actor->flags & ACTOR_FLAG_TARGETABLE)) ? 1 : TATL_HINT_ID_NONE;

    CollisionCheck_InitInfo(&actor->colChkInfo);
    actor->floorBgId = BGCHECK_SCENE;

    ActorShape_Init(&actor->shape, 0.0f, NULL, 0.0f);
    if (Object_IsLoaded(&play->objectCtx, actor->objectSlot)) {
        Actor_SetObjectDependency(play, actor);
        actor->init(actor, play);
        actor->init = NULL;
    }
}

// turn off all Tatl hints related to quest progression
u16 Custom_QuestHint_GetTatlTextId(PlayState* play)
{
    return 0;
    (void)play; // -Wunused-parameter
}


