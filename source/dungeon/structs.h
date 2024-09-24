/**
 * @file structs.h
 *
 * Various global structures.
 */

//////////////////////////////////////////////////
// miniwin
//////////////////////////////////////////////////

#ifndef BASETYPES
typedef int32_t INT;
typedef uint8_t BOOLEAN;

typedef uint32_t DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef uint16_t WORD;

typedef unsigned int UINT;
#endif
//////////////////////////////////////////////////
// control
//////////////////////////////////////////////////

typedef struct POS32 {
	int x;
	int y;
} POS32;

typedef struct AREA32 {
	int w;
	int h;
} AREA32;

typedef struct RECT32 {
	int x;
	int y;
	int w;
	int h;
} RECT32;

typedef struct RECT_AREA32 {
	int x1;
	int y1;
	int x2;
	int y2;
} RECT_AREA32;

//////////////////////////////////////////////////
// items
//////////////////////////////////////////////////

typedef struct RANGE {
	BYTE from;
	BYTE to;
} RANGE;

typedef struct AffixData {
	BYTE PLPower; // item_effect_type
	int PLParam1;
	int PLParam2;
	RANGE PLRanges[NUM_IARS];
	int PLIType; // affix_item_type
	BOOLEAN PLDouble;
	BOOLEAN PLOk;
	int PLMinVal;
	int PLMaxVal;
	int PLMultVal;
} AffixData;

typedef struct UniqItemData {
	const char* UIName;
	BYTE UIUniqType; // unique_item_type
	BYTE UIMinLvl;
	uint16_t UICurs;
	int UIValue;
	BYTE UIPower1; // item_effect_type
	int UIParam1a;
	int UIParam1b;
	BYTE UIPower2; // item_effect_type
	int UIParam2a;
	int UIParam2b;
	BYTE UIPower3; // item_effect_type
	int UIParam3a;
	int UIParam3b;
	BYTE UIPower4; // item_effect_type
	int UIParam4a;
	int UIParam4b;
	BYTE UIPower5; // item_effect_type
	int UIParam5a;
	int UIParam5b;
	BYTE UIPower6; // item_effect_type
	int UIParam6a;
	int UIParam6b;
} UniqItemData;

typedef struct ItemFileData {
	const char* ifName; // Map of item type .cel file names.
	int idSFX;          // sounds effect of dropping the item on ground (_sfx_id).
	int iiSFX;          // sounds effect of placing the item in the inventory (_sfx_id).
	int iAnimLen;       // item drop animation length
} ItemFileData;

typedef struct ItemData {
	const char* iName;
	BYTE iRnd;
	BYTE iMinMLvl;
	BYTE iUniqType; // unique_item_type
	int iCurs;      // item_cursor_graphic
	int itype;      // item_type
	int iMiscId;    // item_misc_id
	int iSpell;     // spell_id
	BYTE iClass;    // item_class
	BYTE iLoc;      // item_equip_type
	BYTE iDamType;  // item_damage_type
	BYTE iMinDam;
	BYTE iMaxDam;
	BYTE iBaseCrit;
	BYTE iMinStr;
	BYTE iMinMag;
	BYTE iMinDex;
	BOOLEAN iUsable;
	BYTE iMinAC;
	BYTE iMaxAC;
	BYTE iDurability;
	int iValue;
} ItemData;

typedef struct ItemStruct {
	int32_t _iSeed;
	uint16_t _iIdx;        // item_indexes
	uint16_t _iCreateInfo; // icreateinfo_flag
	union {
		int _ix;
		int _iPHolder; // parent index of a placeholder entry in InvList
	};
	int _iy;
	int _iCurs;   // item_cursor_graphic
	int _itype;   // item_type
	int _iMiscId; // item_misc_id
	int _iSpell;  // spell_id
	BYTE _iClass; // item_class enum
	BYTE _iLoc;   // item_equip_type
	BYTE _iDamType; // item_damage_type
	BYTE _iMinDam;
	BYTE _iMaxDam;
	BYTE _iBaseCrit;
	BYTE _iMinStr;
	BYTE _iMinMag;
	BYTE _iMinDex;
	BOOLEAN _iUsable; // can be placed in belt, can be consumed/used or stacked (if max durability is not 1)
	BYTE _iPrePower; // item_effect_type
	BYTE _iSufPower; // item_effect_type
	BYTE _iMagical;	// item_quality
	BYTE _iSelFlag;
	BOOLEAN _iFloorFlag;
	BOOLEAN _iAnimFlag;
	BYTE* _iAnimData;        // PSX name -> ItemFrame
	unsigned _iAnimFrameLen; // Tick length of each frame in the current animation
	unsigned _iAnimCnt;      // Increases by one each game tick, counting how close we are to _iAnimFrameLen
	unsigned _iAnimLen;      // Number of frames in current animation
	unsigned _iAnimFrame;    // Current frame of animation.
	//int _iAnimWidth;
	//int _iAnimXOffset;
	BOOL _iPostDraw; // should be drawn during the post-phase (magic rock on the stand) -- unused
	BOOL _iIdentified;
	char _iName[32];
	int _ivalue;
	int _iIvalue;
	int _iAC;
	int _iFlags;	// item_special_effect
	int _iCharges;
	int _iMaxCharges;
	int _iDurability;
	int _iMaxDur;
	int _iPLDam;
	int _iPLToHit;
	int _iPLAC;
	int _iPLStr;
	int _iPLMag;
	int _iPLDex;
	int _iPLVit;
	int _iPLFR;
	int _iPLLR;
	int _iPLMR;
	int _iPLAR;
	int _iPLMana;
	int _iPLHP;
	int _iPLDamMod;
	int _iPLGetHit;
	int8_t _iPLLight;
	int8_t _iPLSkillLevels;
	BYTE _iPLSkill;
	int8_t _iPLSkillLvl;
	BYTE _iPLManaSteal;
	BYTE _iPLLifeSteal;
	BYTE _iPLCrit;
	BOOLEAN _iStatFlag;
	int _iUid; // unique_item_indexes
	BYTE _iPLFMinDam;
	BYTE _iPLFMaxDam;
	BYTE _iPLLMinDam;
	BYTE _iPLLMaxDam;
	BYTE _iPLMMinDam;
	BYTE _iPLMMaxDam;
	BYTE _iPLAMinDam;
	BYTE _iPLAMaxDam;
	int _iVAdd;
	int _iVMult;
} ItemStruct;

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlrAnimType {
	char patTxt[4]; // suffix to select the player animation CL2
	int patGfxIdx;  // player_graphic_idx
} PlrAnimType;

typedef struct PlrAnimStruct {
	BYTE* paAnimData[NUM_DIRS];
	unsigned paFrames;
	int paAnimWidth;
} PlrAnimStruct;

typedef struct PlayerStruct {
	int _pmode; // PLR_MODE
	int8_t _pWalkpath[MAX_PATH_LENGTH + 1];
	int _pDestAction;
	int _pDestParam1;
	int _pDestParam2;
	int _pDestParam3; // the skill to be used in case of skill based actions
	int _pDestParam4; // the level of the skill to be used in case of skill based actions
	BOOLEAN _pActive;
	BYTE _pInvincible;
	BOOLEAN _pLvlChanging; // True when the player is transitioning between levels
	BYTE _pDunLevel; // dungeon_level
	BYTE _pClass; // plr_class
	BYTE _pLevel;
	BYTE _pRank;
	BYTE _pTeam;
	uint16_t _pStatPts;
	BYTE _pLightRad;
	BYTE _pManaShield;
	int16_t _pTimer[NUM_PLRTIMERS];
	unsigned _pExperience;
	unsigned _pNextExper;
	int _px;      // Tile X-position where the player should be drawn
	int _py;      // Tile Y-position where the player should be drawn
	int _pfutx;   // Future tile X-position where the player will be at the end of its action
	int _pfuty;   // Future tile Y-position where the player will be at the end of its action
	int _poldx;   // Most recent tile X-position where the player was at the start of its action
	int _poldy;   // Most recent tile Y-position where the player was at the start of its action
	int _pxoff;   // Pixel X-offset from tile position where the player should be drawn
	int _pyoff;   // Pixel Y-offset from tile position where the player should be drawn
	int _pdir;    // Direction faced by player (direction enum)
	BYTE* _pAnimData;
	int _pAnimFrameLen; // Tick length of each frame in the current animation
	int _pAnimCnt;        // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimXOffset;
	unsigned _plid; // light id of the player
	unsigned _pvid; // vision id of the player
	BYTE _pAtkSkill;         // the selected attack skill for the primary action
	BYTE _pAtkSkillType;     // the (RSPLTYPE_)type of the attack skill for the primary action
	BYTE _pMoveSkill;        // the selected movement skill for the primary action
	BYTE _pMoveSkillType;    // the (RSPLTYPE_)type of the movement skill for the primary action
	BYTE _pAltAtkSkill;      // the selected attack skill for the secondary action
	BYTE _pAltAtkSkillType;  // the (RSPLTYPE_)type of the attack skill for the secondary action
	BYTE _pAltMoveSkill;     // the selected movement skill for the secondary action
	BYTE _pAltMoveSkillType; // the (RSPLTYPE_)type of the movement skill for the secondary action
	BYTE _pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE _pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE _pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE _pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE _pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE _pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE _pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE _pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE _pAtkSkillSwapKey[4];         // the attack skill selected by the hotkey after skill-set swap
	BYTE _pAtkSkillTypeSwapKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey after skill-set swap
	BYTE _pMoveSkillSwapKey[4];        // the movement skill selected by the hotkey after skill-set swap
	BYTE _pMoveSkillTypeSwapKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey after skill-set swap
	BYTE _pAltAtkSkillSwapKey[4];      // the attack skill selected by the alt-hotkey after skill-set swap
	BYTE _pAltAtkSkillTypeSwapKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey after skill-set swap
	BYTE _pAltMoveSkillSwapKey[4];     // the movement skill selected by the alt-hotkey after skill-set swap
	BYTE _pAltMoveSkillTypeSwapKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey after skill-set swap
	BYTE _pSkillLvlBase[64]; // the skill levels of the player if they would not wear an item
	BYTE _pSkillActivity[64];
	unsigned _pSkillExp[64];
	uint64_t _pMemSkills;  // Bitmask of learned skills
	uint64_t _pAblSkills;  // Bitmask of abilities
	uint64_t _pInvSkills;  // Bitmask of skills available via items in inventory (scrolls or runes)
	char _pName[PLR_NAME_LEN];
	uint16_t _pBaseStr;
	uint16_t _pBaseMag;
	uint16_t _pBaseDex;
	uint16_t _pBaseVit;
	int _pHPBase;    // the hp of the player if they would not wear an item
	int _pMaxHPBase; // the maximum hp of the player without items
	int _pManaBase;    // the mana of the player if they would not wear an item
	int _pMaxManaBase; // the maximum mana of the player without items
	int _pVar1;
	int _pVar2;
	int _pVar3;
	int _pVar4;
	int _pVar5;
	int _pVar6;
	int _pVar7;
	int _pVar8;
	int _pGFXLoad; // flags of the loaded gfx('s)  (player_graphic_flag)
	PlrAnimStruct _pAnims[NUM_PGXS];
	unsigned _pAFNum; // action frame number of the attack animation
	unsigned _pSFNum; // action frame number of the spell animation
	ItemStruct _pHoldItem;
	ItemStruct _pInvBody[NUM_INVLOC];
	ItemStruct _pSpdList[MAXBELTITEMS];
	ItemStruct _pInvList[NUM_INV_GRID_ELEM];
	int _pGold;
	int _pStrength;
	int _pMagic;
	int _pDexterity;
	int _pVitality;
	int _pHitPoints; // the current hp of the player
	int _pMaxHP;     // the maximum hp of the player
	int _pMana;      // the current mana of the player
	int _pMaxMana;   // the maximum mana of the player
	BYTE _pSkillLvl[64]; // the skill levels of the player
	uint64_t _pISpells; // Bitmask of skills available via equipped items (staff)
	BYTE _pSkillFlags;    // Bitmask of allowed skill-types (SFLAG_*)
	BOOLEAN _pInfraFlag;
	BYTE _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	BOOLEAN _pHasUnidItem; // whether the player has an unidentified (magic) item equipped
	int _pISlMinDam; // min slash-damage (swords, axes)
	int _pISlMaxDam; // max slash-damage (swords, axes)
	int _pIBlMinDam; // min blunt-damage (maces, axes)
	int _pIBlMaxDam; // max blunt-damage (maces, axes)
	int _pIPcMinDam; // min puncture-damage (bows, daggers)
	int _pIPcMaxDam; // max puncture-damage (bows, daggers)
	int _pIChMinDam; // min charge-damage (shield charge)
	int _pIChMaxDam; // max charge-damage (shield charge)
	int _pIEvasion;
	int _pIAC;
	int8_t _pMagResist;
	int8_t _pFireResist;
	int8_t _pLghtResist;
	int8_t _pAcidResist;
	int _pIHitChance;
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	uint16_t _pIBlockChance;
	unsigned _pIFlags;
	BYTE _pIWalkSpeed;
	BYTE _pIRecoverySpeed;
	BYTE _pIBaseCastSpeed;
	BYTE _pAlign_B1;
	int _pIGetHit;
	BYTE _pIBaseAttackSpeed;
	int8_t _pIArrowVelBonus; // _pISplCost in vanilla code
	BYTE _pILifeSteal;
	BYTE _pIManaSteal;
	int _pIFMinDam; // min fire damage (item's added fire damage)
	int _pIFMaxDam; // max fire damage (item's added fire damage)
	int _pILMinDam; // min lightning damage (item's added lightning damage)
	int _pILMaxDam; // max lightning damage (item's added lightning damage)
	int _pIMMinDam; // min magic damage (item's added magic damage)
	int _pIMMaxDam; // max magic damage (item's added magic damage)
	int _pIAMinDam; // min acid damage (item's added acid damage)
	int _pIAMaxDam; // max acid damage (item's added acid damage)
	BYTE* _pAnimFileData[NUM_PGXS]; // file-pointers of the animations
} PlayerStruct;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

typedef struct MissileData {
	int (*mAddProc)(int, int, int, int, int, int, int, int, int);
	void (*mProc)(int);
	BYTE mdFlags; // missile_flags
	BYTE mResist; // missile_resistance
	BYTE mFileNum; // missile_gfx_id
	BOOLEAN mDrawFlag;
	int mlSFX; // sound effect when a missile is launched (_sfx_id)
	int miSFX; // sound effect on impact (_sfx_id)
	BYTE mlSFXCnt; // number of launch sound effects to choose from
	BYTE miSFXCnt; // number of impact sound effects to choose from
} MissileData;

typedef struct MisFileData {
	int mfAnimFAmt;
	const char* mfName;
	const char* mfAnimTrans;
	int mfFlags; // missile_anim_flags
	BYTE mfAnimFrameLen[16];
	BYTE mfAnimLen[16];
	int mfAnimWidth;
	int mfAnimXOffset; // could be calculated
} MisFileData;

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct MonAnimStruct {
	int maFrames;
	int maFrameLen;
} MonAnimStruct;

typedef struct MonsterAI {
	BYTE aiType; // _monster_ai
	BYTE aiInt;
	BYTE aiParam1;
	BYTE aiParam2;
} MonsterAI;

typedef struct MonsterData {
	uint16_t moFileNum; // _monster_gfx_id
	BYTE mLevel;
	BYTE mSelFlag;
	const char* mTransFile;
	const char* mName;
	MonsterAI mAI;
	uint16_t mMinHP;
	uint16_t mMaxHP;
	unsigned mFlags;  // _monster_flag
	uint16_t mHit;    // hit chance (melee+projectile)
	BYTE mMinDamage;
	BYTE mMaxDamage;
	uint16_t mHit2;   // hit chance of special melee attacks
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	BYTE mMagic;      // hit chance of magic-projectile
	BYTE mMagic2;     // unused
	BYTE mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE mEvasion;    // evasion: used against magic-projectile
	uint16_t mMagicRes;  // resistances in normal and nightmare difficulties (_monster_resistance)
	uint16_t mMagicRes2; // resistances in hell difficulty (_monster_resistance)
	uint16_t mExp;
} MonsterData;

typedef struct MonFileData {
	int moImage;
	const char* moGfxFile;
	const char* moSndFile;
	int moAnimFrames[NUM_MON_ANIM];
	int moAnimFrameLen[NUM_MON_ANIM];
	BYTE moWidth;
	BOOLEAN moSndSpecial;
	BYTE moAFNum;
	BYTE moAFNum2;
} MonFileData;

#pragma pack(push, 1)
typedef struct MapMonData {
	int cmType;
	BOOL cmPlaceScatter;
	MonAnimStruct cmAnims[NUM_MON_ANIM];
	const char* cmName;
	uint16_t cmFileNum;
	BYTE cmLevel;
	BYTE cmSelFlag;
	MonsterAI cmAI;
	unsigned cmFlags;    // _monster_flag
	int cmHit;           // hit chance (melee+projectile)
	int cmMinDamage;
	int cmMaxDamage;
	int cmHit2;          // hit chance of special melee attacks
	int cmMinDamage2;
	int cmMaxDamage2;
	int cmMagic;         // hit chance of magic-projectile
	int cmArmorClass;    // AC+evasion: used against physical-hit (melee+projectile)
	int cmEvasion;       // evasion: used against magic-projectile
	unsigned cmMagicRes; // resistances of the monster (_monster_resistance)
	unsigned cmExp;
	// int cmWidth;
	// int cmXOffset;
	// BYTE cmAFNum;
	// BYTE cmAFNum2;
	// uint16_t cmAlign_0; // unused
	int cmMinHP;
	int cmMaxHP;
} MapMonData;

#pragma pack(pop)
typedef struct MonsterStruct {
	int _mmode; // MON_MODE
	BYTE _mMTidx;
	int _mx;           // Tile X-position where the monster should be drawn
	int _my;           // Tile Y-position where the monster should be drawn
	int _mdir;         // Direction faced by monster (direction enum)
	int _mAnimFrameLen; // Tick length of each frame in the current animation
	int _mAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	int _mAnimLen;   // Number of frames in current animation
	int _mAnimFrame; // Current frame of animation.
	int _mmaxhp;
	int _mhitpoints;
	int32_t _mRndSeed;
	BYTE _muniqtype;
	BYTE _muniqtrans;
	BYTE _mNameColor;  // color of the tooltip. white: normal, blue: pack; gold: unique. (text_color)
	BYTE _mlid;        // light id of the monster
	BYTE _mleader;     // the leader of the monster
	BYTE _mleaderflag; // the status of the monster's leader
	BYTE _mpacksize;   // the number of 'pack'-monsters close to their leader
	BYTE _mvid;        // vision id of the monster (for minions only)
	const char* _mName;
	BYTE _mLevel;
	BYTE _mSelFlag;
	MonsterAI _mAI;
	unsigned _mFlags;    // _monster_flag
	int _mHit;           // hit chance (melee+projectile)
	int _mMinDamage;
	int _mMaxDamage;
	int _mHit2;          // hit chance of special melee attacks
	int _mMinDamage2;
	int _mMaxDamage2;
	int _mMagic;         // hit chance of magic-projectile
	int _mArmorClass;    // AC+evasion: used against physical-hit (melee+projectile)
	int _mEvasion;       // evasion: used against magic-projectile
	unsigned _mMagicRes; // resistances of the monster (_monster_resistance)
	unsigned _mExp;
} MonsterStruct;

typedef struct UniqMonData {
	int mtype; // _monster_id
	const char* mName;
	const char* mTrnName;
	BYTE muLevelIdx; // level-index to place the monster (dungeon_level)
	BYTE muLevel;    // difficulty level of the monster
	uint16_t mmaxhp;
	MonsterAI mAI;
	BYTE mMinDamage;
	BYTE mMaxDamage;
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	uint16_t mMagicRes;  // resistances in normal and nightmare difficulties (_monster_resistance)
	uint16_t mMagicRes2; // resistances in hell difficulty (_monster_resistance)
	BYTE mUnqFlags;// _uniq_monster_flag
	BYTE mUnqHit;  // to-hit (melee+projectile) bonus
	BYTE mUnqHit2; // to-hit (special melee attacks) bonus
	BYTE mUnqMag;  // to-hit (magic-projectile) bonus
	BYTE mUnqEva;  // evasion bonus
	BYTE mUnqAC;   // armor class bonus
	BYTE mQuestId; // quest_id
	int mtalkmsg;  // _speech_id
} UniqMonData;

//////////////////////////////////////////////////
// objects
//////////////////////////////////////////////////

typedef struct ObjectData {
	BYTE ofindex;     // object_graphic_id
	BYTE oLvlTypes;   // dungeon_type_mask
	BYTE otheme;      // theme_id
	BYTE oquest;      // quest_id
	//BYTE oAnimFlag;
	BYTE oAnimBaseFrame; // The starting/base frame of (initially) non-animated objects
	//int oAnimFrameLen; // Tick length of each frame in the current animation
	//int oAnimLen;   // Number of frames in current animation
	//int oAnimWidth;
	//int oSFX;
	//BYTE oSFXCnt;
	BYTE oLightRadius;
	int8_t oLightOffX;
	int8_t oLightOffY;
	BYTE oProc;       // object_proc_func
	BYTE oModeFlags;  // object_mode_flags
	//BOOL oSolidFlag;
	//BYTE oBreak;
	BOOLEAN oMissFlag;
	BYTE oDoorFlag;   // object_door_type
	BYTE oSelFlag;
	BYTE oPreFlag;
	BOOLEAN oTrapFlag;
} ObjectData;

typedef struct ObjFileData {
	const char* ofName;
	int oSFX; // _sfx_id
	BYTE oSFXCnt;
	BYTE oAnimFlag; // object_anim_mode
	int oAnimFrameLen; // Tick length of each frame in the current animation
	int oAnimLen;   // Number of frames in current animation
	int oAnimWidth;
	BOOLEAN oSolidFlag;
	BYTE oBreak; // object_break_mode
} ObjFileData;

typedef struct ObjectStruct {
	int _otype; // _object_id
	int _ox;    // Tile X-position of the object
	int _oy;    // Tile Y-position of the object
	BYTE _oSFXCnt;
	BYTE _oAnimFlag;  // object_anim_mode
	BYTE _oProc;      // object_proc_func
	BYTE _oModeFlags; // object_mode_flags
	int _oAnimFrameLen; // Tick length of each frame in the current animation
	int _oAnimCnt;   // Increases by one each game tick, counting how close we are to _oAnimFrameLen
	int _oAnimLen;   // Number of frames in current animation
	int _oAnimFrame; // Current frame of animation.
	BOOLEAN _oSolidFlag;
	BYTE _oBreak; // object_break_mode
	BYTE _oTrapChance;
	BYTE _oAlign;
	BOOLEAN _oMissFlag;
	BYTE _oDoorFlag; // object_door_type
	BYTE _oSelFlag;
	BOOLEAN _oPreFlag;
	int32_t _oRndSeed;
	int _oVar1;
	int _oVar2;
	int _oVar3;
	int _oVar4;
	int _oVar5;
	int _oVar6;
	int _oVar7;
	int _oVar8;
} ObjectStruct;

//////////////////////////////////////////////////
// endianness
//////////////////////////////////////////////////

typedef struct LE_UINT16 {
	uint16_t _value;

	void operator=(uint16_t val) {
		_value = SwapLE16(val);
	};
	//void operator=(const LE_UINT16& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_UINT16 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_UINT16& oval) const {
		return _value != oval._value;
	};
	operator uint16_t() const { return SwapLE16(_value); }
} LE_UINT16;

typedef struct LE_INT16 {
	int16_t _value;

	void operator=(int16_t val) {
		_value = SwapLE16(val);
	};
	//void operator=(const LE_INT32& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_INT16 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_INT16& oval) const {
		return _value != oval._value;
	};
	operator int16_t() const { return SwapLE16(_value); }
} LE_INT16;

typedef struct LE_UINT32 {
	uint32_t _value;

	void operator=(unsigned val) {
		_value = SwapLE32(val);
	};
	void operator=(unsigned long val) {
		_value = SwapLE32(val);
	};
#if INT_MAX != INT32_MAX
	void operator=(uint32_t val) {
		_value = SwapLE32(val);
	};
#endif
	//void operator=(const LE_UINT32& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_UINT32 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_UINT32& oval) const {
		return _value != oval._value;
	};
	operator unsigned() const { return (uint32_t)SwapLE32(_value); }
} LE_UINT32;

typedef struct LE_INT32 {
	int32_t _value;

	void operator=(int val) {
		_value = SwapLE32(val);
	};
	void operator=(long val) {
		_value = SwapLE32(val);
	};
#if INT_MAX != INT32_MAX
	void operator=(int32_t val) {
		_value = SwapLE32(val);
	};
#endif
	void operator=(const LE_INT32& val) {
		_value = val._value;
	};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_INT32 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_INT32& oval) const {
		return _value != oval._value;
	};
	operator int() const { return (int32_t)SwapLE32(_value); }
} LE_INT32;

typedef struct LE_UINT64 {
	uint64_t _value;

	void operator=(uint64_t val) {
		_value = SwapLE64(val);
	};
	//void operator=(const LE_UINT64& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	//bool operator==(const LE_UINT64 & oval) const {
	//	return _value == oval._value;
	//};
	//bool operator!=(const LE_UINT64& oval) const {
	//	return _value != oval._value;
	//};
	operator uint64_t() const { return (uint64_t)SwapLE64(_value); }
} LE_UINT64;

//////////////////////////////////////////////////
// pack
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct PkItemStruct {
	LE_INT32 dwSeed;
	LE_UINT16 wIndx;
	LE_UINT16 wCI;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	LE_UINT16 wValue;
	LE_UINT32 dwBuff;
} PkItemStruct;

typedef struct PkPlayerStruct {
	//BYTE px;
	//BYTE py;
	char pName[PLR_NAME_LEN];
	BOOLEAN pLvlChanging;
	BYTE pDunLevel;
	BYTE pClass;
	BYTE pLevel;
	BYTE pRank;
	BYTE pTeam;
	LE_UINT16 pStatPts;
	//BYTE pLightRad;
	//BYTE pManaShield;
	//LE_INT16 pTimer[NUM_PLRTIMERS];
	LE_UINT32 pExperience;
	LE_UINT16 pBaseStr;
	LE_UINT16 pBaseMag;
	LE_UINT16 pBaseDex;
	LE_UINT16 pBaseVit;
	LE_INT32 pHPBase;
	LE_INT32 pMaxHPBase;
	LE_INT32 pManaBase;
	LE_INT32 pMaxManaBase;
	BYTE pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE pAltMoveSkillTypeHotKey[4]; // the movement skill selected by the alt-hotkey
	BYTE pAtkSkillSwapKey[4];         // the attack skill selected by the hotkey after skill-set swap
	BYTE pAtkSkillTypeSwapKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey after skill-set swap
	BYTE pMoveSkillSwapKey[4];        // the movement skill selected by the hotkey after skill-set swap
	BYTE pMoveSkillTypeSwapKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey after skill-set swap
	BYTE pAltAtkSkillSwapKey[4];      // the attack skill selected by the alt-hotkey after skill-set swap
	BYTE pAltAtkSkillTypeSwapKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey after skill-set swap
	BYTE pAltMoveSkillSwapKey[4];     // the movement skill selected by the alt-hotkey after skill-set swap
	BYTE pAltMoveSkillTypeSwapKey[4]; // the movement skill selected by the alt-hotkey after skill-set swap
	BYTE pSkillLvlBase[64];
	BYTE pSkillActivity[64];
	LE_UINT32 pSkillExp[64];
	LE_UINT64 pMemSkills;
	PkItemStruct pHoldItem;
	PkItemStruct pInvBody[NUM_INVLOC];
	PkItemStruct pSpdList[MAXBELTITEMS];
	PkItemStruct pInvList[NUM_INV_GRID_ELEM];
	int8_t pInvGrid[NUM_INV_GRID_ELEM];
	LE_INT32 pNumInv; // unused
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// levels
//////////////////////////////////////////////////

typedef struct LevelStruct {
	int _dLevelIdx;   // dungeon_level / NUM_LEVELS
	int _dLevelNum;   // index in AllLevels (dungeon_level / NUM_FIXLVLS)
	bool _dSetLvl;    // cached flag if the level is a set-level
	bool _dDynLvl;    // cached flag if the level is a dynamic-level
	int _dLevel;      // cached difficulty value of the level
	int _dType;       // cached type of the level (dungeon_type)
	int _dDunType;    // cached type of the dungeon (dungeon_gen_type)
	int _dLevelPlyrs; // cached number of players when the level was 'initialized'
	int _dLevelBonus; // cached level bonus
} LevelStruct;

typedef struct LevelFileData {
	const char* dSubtileSettings;
	const char* dTileFlags;
	const char* dMicroCels;
	const char* dMegaTiles;
	const char* dMiniTiles;
	const char* dSpecCels;
	const char* dLightTrns;
} LevelFileData;

typedef struct LevelData {
	BYTE dLevel;
	BOOLEAN dSetLvl;
	BYTE dType;    // dungeon_type
	BYTE dDunType; // dungeon_gen_type
	BYTE dMusic;   // _music_id
	BYTE dfindex;  // level_graphic_id
	BYTE dMicroTileLen;
	BYTE dBlocks;
	const char* dLevelName;
	const char* dPalName;
	const char* dLoadCels;
	const char* dLoadPal;
	BOOLEAN dLoadBarOnTop;
	BYTE dLoadBarColor;
	BYTE dMonDensity;
	BYTE dObjDensity;
	BYTE dSetLvlDunX;
	BYTE dSetLvlDunY;
	BYTE dSetLvlWarp; // dungeon_warp_type
	BYTE dSetLvlPiece; // _setpiece_type
	BYTE dMonTypes[32];
} LevelData;

typedef struct WarpStruct {
	int _wx;
	int _wy;
	int _wtype; // dungeon_warp_type
	int _wlvl;  // dungeon_level / _setlevels
} WarpStruct;

typedef struct SetPieceStruct {
	BYTE* _spData;
	int _spx;
	int _spy;
	int _sptype; // _setpiece_type
} SetPieceStruct;

typedef struct SetPieceData {
	const char* _spdDunFile;
	const char* _spdPreDunFile;
} SetPieceData;

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	BYTE _qactive; // quest_state
	BYTE _qvar1; // quest parameter which is synchronized with the other players
	BYTE _qvar2; // quest parameter which is NOT synchronized with the other players
	BOOLEAN _qlog;
	unsigned _qmsg;
} QuestStruct;

typedef struct QuestData {
	BYTE _qdlvl; // dungeon_level
	BYTE _qslvl; // _setlevels
	int _qdmsg;  // _speech_id
	const char* _qlstr; // quest title
} QuestData;

//////////////////////////////////////////////////
// spells
//////////////////////////////////////////////////

typedef struct SpellData {
	BYTE sManaCost;
	BYTE sType;
	BYTE sIcon;
	const char* sNameText;
	BYTE sBookLvl;   // minimum level for books
	BYTE sStaffLvl;  // minimum level for staves
	BYTE sScrollLvl; // minimum level for scrolls/runes
	BYTE sSkillFlags; // flags (SDFLAG*) of the skill
	BYTE scCurs; // cursor for scrolls/runes
	BYTE spCurs; // cursor for spells
	BYTE sUseFlags; // the required flags(SFLAG*) to use the skill
	BYTE sMinInt;
	BYTE sSFX;
	BYTE sMissile;
	BYTE sManaAdj;
	BYTE sMinMana;
	uint16_t sStaffMin;
	uint16_t sStaffMax;
	int sBookCost;
	int sStaffCost; // == sScrollCost == sRuneCost
} SpellData;

//////////////////////////////////////////////////
// gendung
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// drlg
//////////////////////////////////////////////////

typedef struct ShadowPattern {
	union {
		struct {
			BYTE sh11;
			BYTE sh01;
			BYTE sh10;
			BYTE sh00;
		};
		uint32_t asUInt32;
	};
} ShadowPattern;

typedef struct ShadowStruct {
	ShadowPattern shPattern;
	ShadowPattern shMask;
	BYTE nv1;
	BYTE nv2;
	BYTE nv3;
} ShadowStruct;

typedef struct ROOMHALLNODE {
	int nRoomParent;
	int nRoomx1;
	int nRoomy1;
	int nRoomx2;
	int nRoomy2;
	int nHallx1;
	int nHally1;
	int nHallx2;
	int nHally2;
	int nHalldir;
} ROOMHALLNODE;

typedef struct L1ROOM {
	int lrx;
	int lry;
	int lrw;
	int lrh;
} L1ROOM;

typedef struct ThemePosDir {
	int tpdx;
	int tpdy;
	int tpdvar1;
	int tpdvar2; // unused
} ThemePosDir;

/** The number of generated rooms in cathedral. */
#define L1_MAXROOMS ((DSIZEX * DSIZEY) / sizeof(L1ROOM))
/** The number of generated rooms in catacombs. */
#define L2_MAXROOMS 32
/** Possible matching locations in a theme room. */
#define THEME_LOCS ((DSIZEX * DSIZEY) / sizeof(ThemePosDir))

typedef struct DrlgMem {
	union {
		L1ROOM L1RoomList[L1_MAXROOMS];     // drlg_l1
		ROOMHALLNODE RoomList[L2_MAXROOMS]; // drlg_l2
		BYTE transvalMap[DMAXX][DMAXY];     // drlg_l1, drlg_l2, drlg_l3, drlg_l4
		BYTE transDirMap[DSIZEX][DSIZEY];   // drlg_l1, drlg_l2, drlg_l3, drlg_l4 (gendung)
		BYTE lockoutMap[DMAXX][DMAXY];      // drlg_l3
		BYTE dungBlock[L4BLOCKX][L4BLOCKY]; // drlg_l4
		ThemePosDir thLocs[THEME_LOCS];     // themes
	};
} DrlgMem;

//////////////////////////////////////////////////
// themes
//////////////////////////////////////////////////

typedef struct ThemeStruct {
	int _tsx1; // top-left corner of the theme-room
	int _tsy1;
	int _tsx2; // bottom-right corner of the theme-room
	int _tsy2;
	BYTE _tsType;
	BYTE _tsTransVal;
	BYTE _tsObjVar1;
	BYTE _tsObjVar2; // unused
	int _tsObjX;
	int _tsObjY;
} ThemeStruct;

//////////////////////////////////////////////////
// inv
//////////////////////////////////////////////////

typedef struct InvXY {
	int X;
	int Y;
} InvXY;

//////////////////////////////////////////////////
// lighting
//////////////////////////////////////////////////

typedef struct LightListStruct {
	int _lx;
	int _ly;
	int _lunx;
	int _luny;
	BYTE _lradius;
	BYTE _lunr;
	int8_t _lunxoff;
	int8_t _lunyoff;
	BOOLEAN _ldel;
	BOOLEAN _lunflag;
	BOOLEAN _lmine;
	BYTE _lAlign2;
	int _lxoff;
	int _lyoff;
} LightListStruct;

//////////////////////////////////////////////////
// trigs
//////////////////////////////////////////////////

typedef struct TriggerStruct {
	int _tx;
	int _ty;
	int _ttype; // dungeon_warp_type
	int _tlvl;  // dungeon_level
	int _tmsg;  // window_messages
} TriggerStruct;

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

typedef struct _uidefaultstats {
	uint16_t dsStrength;
	uint16_t dsMagic;
	uint16_t dsDexterity;
	uint16_t dsVitality;
} _uidefaultstats;

typedef struct _uiheroinfo {
	BYTE hiIdx;
	BYTE hiLevel;
	BYTE hiClass;
	BYTE hiRank;
	char hiName[16];
	int16_t hiStrength;
	int16_t hiMagic;
	int16_t hiDexterity;
	int16_t hiVitality;
} _uiheroinfo;
