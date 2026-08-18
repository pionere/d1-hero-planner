/**
 * @file items.cpp
 *
 * Implementation of item functionality.
 */
#include "all.h"
//#include "engine/render/text_render.h"

#include <QApplication>
#include <QMessageBox>

DEVILUTION_BEGIN_NAMESPACE

#define ITEM_ANIM_DELAY 1

int itemactive[MAXITEMS];
/** Contains the items on ground in the current game. */
ItemStruct items[MAXITEMS + 1];
//static CelAnimBuf* itemanims[NUM_IFILE];
int numitems;

int ac_rnd;
int affix_rnd[6];

static inline unsigned items_get_currlevel()
{
	return currLvl._dLevel;
}

/*
 * Calculate the walk speed from walk-speed modifiers.
 *  ISPL_FASTWALK:    +1
 *  ISPL_FASTERWALK:  +2
 *  ISPL_FASTESTWALK: +3
 */
inline static BYTE WalkSpeed(unsigned flags)
{
	BYTE res = 0;

	if (flags & ISPL_FASTESTWALK) {
		res = 3;
	} else if (flags & ISPL_FASTERWALK) {
		res = 2;
	} else if (flags & ISPL_FASTWALK) {
		res = 1;
	}

	return res;
}

/*
 * Calculate the (hit-)recovery speed from recover-speed modifiers.
 *  ISPL_FASTRECOVER:    +1
 *  ISPL_FASTERRECOVER:  +2
 *  ISPL_FASTESTRECOVER: +3
 */
inline static BYTE RecoverySpeed(unsigned flags)
{
	BYTE res = 0;

	if (flags & ISPL_FASTESTRECOVER) {
		res = 3;
	} else if (flags & ISPL_FASTERRECOVER) {
		res = 2;
	} else if (flags & ISPL_FASTRECOVER) {
		res = 1;
	}

	return res;
}

/*
 * Calculate the base cast speed from cast-speed modifiers.
 *  ISPL_FASTCAST:    +1
 *  ISPL_FASTERCAST:  +2
 *  ISPL_FASTESTCAST: +3
 */
inline static BYTE BaseCastSpeed(unsigned flags)
{
	BYTE res = 0;

	if (flags & ISPL_FASTESTCAST) {
		res = 3;
	} else if (flags & ISPL_FASTERCAST) {
		res = 2;
	} else if (flags & ISPL_FASTCAST) {
		res = 1;
	}

	return res;
}

void CalcPlrItemVals(int pnum, bool Loadgfx)
{
	ItemStruct* pi;
	ItemStruct *wRight, *wLeft;

	BYTE gfx;       // graphics
	int wt;         // weapon-type
	bool bf;        // blockflag
	unsigned pdmod; // player damage mod

	int i;

	BOOLEAN idi = FALSE; // there is an unidentfied item

	int asb = 0;    // bonus to attack speed
	int tac = 0;    // armor class
	int btohit = 0; // bonus chance to hit
	int btoblk = 0; // bonus chance to block

	int iflgs = ISPL_NONE; // item_special_effect flags

	int madd = 0; // added magic
	int vadd = 0; // added vitality

	int br = gnDifficulty * -10;
	int fr = br; // fire resistance
	int lr = br; // lightning resistance
	int mr = br; // magic resistance
	int ar = br; // acid resistance

	// temporary values to calculate armor class/damage of the current item
	int cac, cdmod, cdmodp, mindam, maxdam;
	int absAnyHit = 0; // absorbed hit-damage
	int absPhyHit = 0; // absorbed physical hit damage
	BYTE manasteal = 0;
	BYTE lifesteal = 0;

	int lrad = 10; // light radius

	int ihp = 0;   // increased HP
	int imana = 0; // increased mana

	int skillLvl;            // temporary value to calculate skill levels
	int8_t skillLvlAdds = 0; // increased skill level
	BYTE skillLvlMods[NUM_SPELLS] = { 0 };

	unsigned minsl = 0; // min slash-damage
	unsigned maxsl = 0; // max slash-damage
	unsigned minbl = 0; // min blunt-damage
	unsigned maxbl = 0; // max blunt-damage
	unsigned minpc = 0; // min puncture-damage
	unsigned maxpc = 0; // max puncture-damage
	unsigned fmin = 0;  // min fire damage
	unsigned fmax = 0;  // max fire damage
	unsigned lmin = 0;  // min lightning damage
	unsigned lmax = 0;  // max lightning damage
	unsigned mmin = 0;  // min magic damage
	unsigned mmax = 0;  // max magic damage
	unsigned amin = 0;  // min acid damage
	unsigned amax = 0;  // max acid damage

	unsigned cc = 0; // critical hit chance
	int btochit = 0; // bonus chance to critical hit
	unsigned pmodp = 0; // added power

	// Loadgfx &= plr._pDunLevel == currLvl._dLevelIdx && !plr._pLvlChanging;

	uint64_t spells = 0;
	pi = plr._pInvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE && plr._pStrength >= pi->_iReqStr) {
			cac = pi->_iAC;
			cdmod = 0;
			cdmodp = 0;

			const bool doDam = /*plr._pStrength >= pi->_iReqStr && plr._pMagic >= pi->_iReqMag */plr._pDexterity >= pi->_iReqDex;
			if (pi->_iMagical != ITEM_QUALITY_NORMAL) {
				idi |= pi->_iUnidentified;
				btohit += pi->_iPLToHit;

				madd += pi->_iPLMag;
				vadd += pi->_iPLVit;
				for (unsigned n = 0; n < pi->_iNumAffixes; n++) {
					const ItemAffixStruct* ias = &pi->_iAffixes[n];
					switch (ias->asPower) {
					case IPL_ACP: {
						int tmpac = ias->asValue0 * cac / 100;
						if (tmpac == 0)
							tmpac = ias->asValue0 >= 0 ? 1 : -1;
						cac += tmpac;
					} break;
					case IPL_TOBLOCK:
						btoblk += ias->asValue0;
						break;
					case IPL_FIRERES:
						fr += ias->asValue0;
						break;
					case IPL_LIGHTRES:
						lr += ias->asValue0;
						break;
					case IPL_MAGICRES:
						mr += ias->asValue0;
						break;
					case IPL_ACIDRES:
						ar += ias->asValue0;
						break;
					case IPL_ALLRES: {
						int v = ias->asValue0;
						fr += v;
						lr += v;
						mr += v;
						ar += v;
					} break;
					case IPL_CRITP:
						btochit += ias->asValue0;
						break;
					case IPL_POWMOD:
						pmodp += ias->asValue0;
						break;
					case IPL_SKILLLVL:
						skillLvlMods[ias->asValue1] += ias->asValue0;
						break;
					case IPL_SKILLLEVELS:
						skillLvlAdds += ias->asValue0;
						break;
					case IPL_FIREDAM:
						if (doDam) {
							fmin += ias->asFrom;
							fmax += ias->asTo;
						}
						break;
					case IPL_LIGHTDAM:
						if (doDam) {
							lmin += ias->asFrom;
							lmax += ias->asTo;
						}
						break;
					case IPL_MAGICDAM:
						if (doDam) {
							mmin += ias->asFrom;
							mmax += ias->asTo;
						}
						break;
					case IPL_ACIDDAM:
						if (doDam) {
							amin += ias->asFrom;
							amax += ias->asTo;
						}
						break;
					case IPL_ABS_ANYHIT:
						absAnyHit += ias->asValue0;
						break;
					case IPL_ABS_PHYHIT:
						absPhyHit += ias->asValue0;
						break;
					case IPL_LIFE:
						ihp += ias->asValue0;
						break;
					case IPL_MANA:
						imana += ias->asValue0;
						break;
					case IPL_LIGHT:
						lrad += ias->asValue0;
						break;
					//case IPL_THORNS:
					//	iflgs |= ISPL_THORNS;
					//	break;
					case IPL_NOMANA:
						iflgs |= ISPL_NOMANA;
						break;
					case IPL_KNOCKBACK:
						iflgs |= ISPL_KNOCKBACK;
						break;
					case IPL_STUN:
						iflgs |= ISPL_STUN;
						break;
					case IPL_NO_BLEED:
						iflgs |= ISPL_NO_BLEED;
						break;
					case IPL_BLEED:
						iflgs |= ISPL_BLEED;
						break;
					//case IPL_NOHEALMON:
					//	iflgs |= ISPL_NOHEALMON;
					//	break;
					case IPL_STEALMANA:
						manasteal += ias->asValue0;
						break;
					case IPL_STEALLIFE:
						lifesteal += ias->asValue0;
						break;
					case IPL_PENETRATE_PHYS:
						iflgs |= ISPL_PENETRATE_PHYS;
						break;
					case IPL_FASTATTACK:
						asb += ias->asValue0;
						break;
					case IPL_FASTRECOVER:
						static_assert((ISPL_FASTRECOVER & (ISPL_FASTRECOVER - 1)) == 0, "Optimized CalcPlrItemVals depends simple flag-like hit-recovery modifiers.");
						static_assert(ISPL_FASTRECOVER == ISPL_FASTERRECOVER / 2, "CalcPlrItemVals depends on ordered hit-recovery modifiers I.");
						static_assert(ISPL_FASTERRECOVER == ISPL_FASTESTRECOVER / 2, "CalcPlrItemVals depends on ordered hit-recovery modifiers II.");
						// assert((unsigned)(ias->asValue0 - 1) < 3);
						iflgs |= ISPL_FASTRECOVER << (ias->asValue0 - 1);
						break;
					case IPL_FASTBLOCK:
						iflgs |= ISPL_FASTBLOCK;
						break;
					case IPL_DAMMOD:
						cdmod = ias->asValue0;
						break;
					case IPL_ALLRESZERO:
						iflgs |= ISPL_ALLRESZERO;
						break;
					case IPL_DRAINLIFE:
						iflgs |= ISPL_DRAINLIFE;
						break;
					//case IPL_INFRAVISION:
					//	iflgs |= ISPL_INFRAVISION;
					//	break;
					case IPL_MANATOLIFE:
						iflgs |= ISPL_MANATOLIFE;
						break;
					case IPL_LIFETOMANA:
						iflgs |= ISPL_LIFETOMANA;
						break;
					case IPL_FASTCAST:
						static_assert((ISPL_FASTCAST & (ISPL_FASTCAST - 1)) == 0, "Optimized CalcPlrItemVals depends simple flag-like cast-speed modifiers.");
						static_assert(ISPL_FASTCAST == ISPL_FASTERCAST / 2, "CalcPlrItemVals depends on ordered cast-speed modifiers I.");
						static_assert(ISPL_FASTERCAST == ISPL_FASTESTCAST / 2, "CalcPlrItemVals depends on ordered cast-speed modifiers II.");
						// assert((unsigned)(ias->asValue0 - 1) < 3);
						iflgs |= ISPL_FASTCAST << (ias->asValue0 - 1);
						break;
					case IPL_FASTWALK:
						static_assert((ISPL_FASTWALK & (ISPL_FASTWALK - 1)) == 0, "Optimized CalcPlrItemVals depends simple flag-like walk-speed modifiers.");
						static_assert(ISPL_FASTWALK == ISPL_FASTERWALK / 2, "CalcPlrItemVals depends on ordered walk-speed modifiers I.");
						static_assert(ISPL_FASTERWALK == ISPL_FASTESTWALK / 2, "CalcPlrItemVals depends on ordered walk-speed modifiers II.");
						// assert((unsigned)(ias->asValue0 - 1) < 3);
						iflgs |= ISPL_FASTWALK << (ias->asValue0 - 1);
						break;
					}
				}
				cdmodp = pi->_iPLDam;
			}

			if (pi->_iSpell != SPL_NULL)
				spells |= SPELL_MASK(pi->_iSpell);
			tac += cac;
			pmodp += pi->_iBasePow;
			maxdam = pi->_iMaxDam;
			if (!doDam || maxdam == 0)
				continue;
			cdmodp += 100;
			cc += pi->_iBaseCrit;
			mindam = pi->_iMinDam;
			mindam = mindam * cdmodp + cdmod * 100;
			maxdam = maxdam * cdmodp + cdmod * 100;
			switch (pi->_iDamType) {
			case IDAM_NONE: break;
			case IDAM_SLASH:
				minsl += (unsigned)mindam << 1;
				maxsl += (unsigned)maxdam << 1;
				break;
			case IDAM_BLUNT:
				minbl += (unsigned)mindam << 1;
				maxbl += (unsigned)maxdam << 1;
				break;
			case IDAM_SB_MIX:
				minsl += mindam;
				minbl += mindam;
				maxsl += maxdam;
				maxbl += maxdam;
				break;
			case IDAM_PUNCTURE:
				minpc += (unsigned)mindam << 1;
				maxpc += (unsigned)maxdam << 1;
				break;
			default:
				ASSUME_UNREACHABLE
			}
		}
	}

	plr._pISpells = spells;

	plr._pHasUnidItem = idi;
	plr._pIAbsAnyHit = absAnyHit << 6;
	plr._pIAbsPhyHit = absPhyHit << 6;
	plr._pILifeSteal = lifesteal;
	plr._pIManaSteal = manasteal;

	ihp <<= 6;
	imana <<= 6;

	ihp += vadd << (6 + 1); // BUGFIX: blood boil can cause negative shifts here (see line 557)
	imana += madd << (6 + 1);

	plr._pIFlags = iflgs;
	// plr._pInfraFlag = (iflgs & ISPL_INFRAVISION) != 0 || plr._pTimer[PLTR_INFRAVISION] > 0;

	// calculate walk speed
	plr._pIWalkSpeed = WalkSpeed(iflgs);

	// calculate (hit-)recovery speed
	plr._pIRecoverySpeed = RecoverySpeed(iflgs);

	// calculate base cast speed
	plr._pIBaseCastSpeed = BaseCastSpeed(iflgs);

	if (iflgs & ISPL_LIFETOMANA) {
		ihp -= plr._pMaxHPBase >> 1;
		imana += plr._pMaxHPBase >> 1;
	}
	if (iflgs & ISPL_MANATOLIFE) {
		ihp += plr._pMaxManaBase >> 1;
		imana -= plr._pMaxManaBase >> 1;
	}
	if (iflgs & ISPL_NOMANA) {
		imana = -plr._pManaBase;
	}
	if (iflgs & ISPL_ALLRESZERO) {
		// reset resistances to zero if the respective special effect is active
		fr = 0;
		lr = 0;
		mr = 0;
		ar = 0;
	}

	if (fr > MAXRESIST)
		fr = MAXRESIST;
	plr._pFireResist = fr;

	if (lr > MAXRESIST)
		lr = MAXRESIST;
	plr._pLghtResist = lr;

	if (mr > MAXRESIST)
		mr = MAXRESIST;
	plr._pMagResist = mr;

	if (ar > MAXRESIST)
		ar = MAXRESIST;
	plr._pAcidResist = ar;

	plr._pHitPoints = ihp + plr._pHPBase;
	plr._pMaxHP = ihp + plr._pMaxHPBase;

	plr._pMana = imana + plr._pManaBase;
	plr._pMaxMana = imana + plr._pMaxManaBase;

	vadd += plr._pBaseVit;
	vadd = std::max(0, vadd);
	// use calculated str/dex/mag from CalcItemReqs
	int sadd = plr._pStrength;
	int dadd = plr._pDexterity;
	madd = plr._pMagic;
	plr._pVitality = vadd;
	if (plr._pTimer[PLTR_RAGE] > 0) {
		sadd += 2 * plr._pLevel;
		dadd += plr._pLevel;
		vadd += 2 * plr._pLevel;
	}

	pdmod = (1 << 9) + (32 * madd);
	plr._pIFMinDam = fmin * pdmod >> (-6 + 9);
	plr._pIFMaxDam = fmax * pdmod >> (-6 + 9);
	plr._pILMinDam = lmin * pdmod >> (-6 + 9);
	plr._pILMaxDam = lmax * pdmod >> (-6 + 9);
	plr._pIMMinDam = mmin * pdmod >> (-6 + 9);
	plr._pIMMaxDam = mmax * pdmod >> (-6 + 9);
	plr._pIAMinDam = amin * pdmod >> (-6 + 9);
	plr._pIAMaxDam = amax * pdmod >> (-6 + 9);

	wLeft = &plr._pInvBody[INVLOC_HAND_LEFT];
	wRight = &plr._pInvBody[INVLOC_HAND_RIGHT];

	bf = false;
	wt = SFLAG_MELEE;
	gfx = plr._pStrength >= wLeft->_iReqStr ? wLeft->_itype : ITYPE_NONE;

	switch (gfx) {
	case ITYPE_NONE:
		gfx = ANIM_ID_UNARMED;
		break;
	case ITYPE_SWORD:
		gfx = ANIM_ID_SWORD;
		break;
	case ITYPE_AXE:
		gfx = ANIM_ID_AXE;
		break;
	case ITYPE_BOW:
		wt = SFLAG_RANGED;
		gfx = ANIM_ID_BOW;
		break;
	case ITYPE_MACE:
		gfx = ANIM_ID_MACE;
		break;
	case ITYPE_STAFF:
		gfx = ANIM_ID_STAFF;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

/*#ifdef HELLFIRE
	if (plr._pClass == PC_MONK) {
		if (gfx == ANIM_ID_STAFF) {
			bf = true;
			plr._pIFlags |= ISPL_FASTBLOCK;
		} else if (wRight->_itype == ITYPE_NONE
		 && (wLeft->_itype == ITYPE_NONE || wLeft->_iLoc != ILOC_TWOHAND))
			bf = true;
	}
#endif*/
	maxdam = plr._pMaxHP >> (2 + 1 - 1); // ~1/4 hp - halved by resists, doubled by MissToPlr
	if (wRight->_itype == ITYPE_SHIELD && plr._pStrength >= wRight->_iReqStr
	 && (gfx == ANIM_ID_UNARMED || gfx == ANIM_ID_SWORD || gfx == ANIM_ID_MACE)) {
		static_assert((int)ANIM_ID_UNARMED + 1 == (int)ANIM_ID_UNARMED_SHIELD, "CalcPlrItemVals uses inc to set gfx with shield I.");
		static_assert((int)ANIM_ID_SWORD + 1 == (int)ANIM_ID_SWORD_SHIELD, "CalcPlrItemVals uses inc to set gfx with shield II.");
		static_assert((int)ANIM_ID_MACE + 1 == (int)ANIM_ID_MACE_SHIELD, "CalcPlrItemVals uses inc to set gfx with shield III.");
		gfx++;
		bf = wRight->_iStatFlag;
		if (bf) {
			tac += ((dadd - (1 << 7)) * wRight->_iAC) >> 7;
			maxdam += wRight->_iAC << (6 + 2 + 1 - 1); // 4*AC - halved by resists, doubled by MissToPlr
		}
	}
	plr._pIChMinDam = maxdam >> 1;
	plr._pIChMaxDam = maxdam;

	if (gfx == ANIM_ID_UNARMED || gfx == ANIM_ID_UNARMED_SHIELD) {
		if (gfx == ANIM_ID_UNARMED_SHIELD) {
			minbl = maxbl = 3 << 1;
		} else {
			minbl = maxbl = 1 << 1;
		}
		minbl += plr._pLevel >> (2 - 1);
		maxbl += plr._pLevel >> (1 - 1);
		minbl *= 100;
		maxbl *= 100;
	}

	pi = &plr._pInvBody[INVLOC_CHEST];
	if (pi->_itype == ITYPE_MARMOR && pi->_iStatFlag) {
		gfx |= ANIM_ID_MEDIUM_ARMOR;
	} else if (pi->_itype == ITYPE_HARMOR && pi->_iStatFlag) {
		gfx |= ANIM_ID_HEAVY_ARMOR;
	}

	// calculate bonuses
	cc = cc * (btochit + 100) / 50;
	plr._pIBaseHitBonus = btohit == 0 ? IBONUS_NONE : (btohit >= 0 ? IBONUS_POSITIVE : IBONUS_NEGATIVE);
	plr._pIPower = madd * (100 + pmodp) / 100;
	plr._pIEvasion = dadd / 5 + 2 * plr._pLevel;
	plr._pIAC = tac + plr._pIEvasion;
	btohit += 50; // + plr._pLevel;
	if (wt == SFLAG_MELEE) {
		btohit += 20 + (dadd >> 1);
	} else {
		// assert(wt == SFLAG_RANGED);
		btohit += dadd;
	}
	plr._pIHitChance = btohit;

	// calculate skill flags
	if (plr._pDunLevel != DLV_TOWN)
		wt |= SFLAG_DUNGEON;
	if (bf)
		wt |= SFLAG_BLOCK;
	if (plr._pTimer[PLTR_RAGE] == 0)
		wt |= SFLAG_RAGE;
	plr._pSkillFlags = wt;

	// calculate the damages for each type
	if (maxsl != 0) {
		pdmod = 512 + sadd * 6 + dadd * 2;
		minsl = minsl * pdmod / (100 * 512 / 64);
		maxsl = maxsl * pdmod / (100 * 512 / 64);
	}
	if (maxbl != 0) {
		if (wLeft->_itype == ITYPE_STAFF)
			pdmod = 512 + sadd * 4 + dadd * 4;
		else
			pdmod = 512 + sadd * 6 + vadd * 2;
		minbl = minbl * pdmod / (100 * 512 / 64);
		maxbl = maxbl * pdmod / (100 * 512 / 64);
	}
	if (maxpc != 0) {
		if (wLeft->_itype == ITYPE_BOW)
			pdmod = 512 + dadd * 8;
		else // dagger
			pdmod = 512 + sadd * 2 + dadd * 6;
		minpc = minpc * pdmod / (100 * 512 / 64);
		maxpc = maxpc * pdmod / (100 * 512 / 64);
	}
	if (wRight->_itype != ITYPE_NONE && wRight->_itype != ITYPE_SHIELD) {
		// adjust dual-wield damage
		//if (maxsl != 0) {
			minsl = minsl * 5 / 8;
			maxsl = maxsl * 5 / 8;
		//}
		//if (maxbl != 0) {
			minbl = minbl * 5 / 8;
			maxbl = maxbl * 5 / 8;
		//}
		//if (maxpc != 0) {
			minpc = minpc * 5 / 8;
			maxpc = maxpc * 5 / 8;
		//}
		cc >>= 1;
		asb >>= 1;
	}
	plr._pISlMinDam = minsl;
	plr._pISlMaxDam = maxsl;
	plr._pIBlMinDam = minbl;
	plr._pIBlMaxDam = maxbl;
	plr._pIPcMinDam = minpc;
	plr._pIPcMaxDam = maxpc;
	plr._pICritChance = cc;
	plr._pIBaseAttackSpeed = asb;

	// calculate block chance
	plr._pIBlockChance = (plr._pSkillFlags & SFLAG_BLOCK) ? btoblk + std::min(sadd, dadd) : 0;

	static_assert(SPL_NULL == 0, "CalcPlrItemVals expects SPL_NULL == 0.");
	static_assert(lengthof(plx(0)._pSkillLvlBase) >= NUM_SPELLS, "Base skill-level can not be read from PlayerStruct._pSkillLvlBase");
	static_assert(lengthof(plx(0)._pSkillLvl) >= NUM_SPELLS, "Calculated skill-level can not be stored in PlayerStruct._pSkillLvl");
	for (i = 1; i < NUM_SPELLS; i++) {
		skillLvl = 0;
		//if (plr._pMemSkills & SPELL_MASK(i)) {
			skillLvl = plr._pSkillLvlBase[i] + skillLvlAdds + skillLvlMods[i];
			if (skillLvl < 0)
				skillLvl = 0;
		//}
		plr._pSkillLvl[i] = skillLvl;
	}

	lrad = std::max(2, std::min(MAX_LIGHT_RAD, lrad));
	if (plr._pLightRad != lrad) {
		plr._pLightRad = lrad;
		/*if (Loadgfx) {
			ChangeLightRadius(plr._plid, lrad);
			ChangeVisionRadius(plr._pvid, std::max(PLR_MIN_VISRAD, lrad));
		}*/
	}

#if 0
	if (plr._pmode == PM_DEATH || plr._pmode == PM_DYING) {
		PlrSetHp(pnum, 0);
		PlrSetMana(pnum, 0);
		gfx = ANIM_ID_UNARMED;
	}
	if (plr._pgfxnum != gfx) {
		plr._pgfxnum = gfx;
		plr._pGFXLoad = 0;
		if (Loadgfx) {
			SetPlrAnims(pnum);

			PlrStartStand(pnum);
		}
	}

	if (pnum == mypnum)
		gbRedrawFlags = REDRAW_RECALC_FLASKS; // gbRedrawFlags |= REDRAW_RECALC_FLASKS | REDRAW_SPELL_ICON;
#else
    plr._pgfxnum = gfx;
#endif
}

static void CalcPlrSpells(int pnum)
{
	PlayerStruct* p;

	p = &plr;
	// switch between normal attacks
	if (p->_pSkillFlags & SFLAG_MELEE) {
		if (p->_pMainSkill._psAttack._suSkill == SPL_RATTACK)
			p->_pMainSkill._psAttack._suSkill = SPL_ATTACK;
		if (p->_pAltSkill._psAttack._suSkill == SPL_RATTACK)
			p->_pAltSkill._psAttack._suSkill = SPL_ATTACK;
	} else {
		if (p->_pMainSkill._psAttack._suSkill == SPL_ATTACK)
			p->_pMainSkill._psAttack._suSkill = SPL_RATTACK;
		if (p->_pAltSkill._psAttack._suSkill == SPL_ATTACK)
			p->_pAltSkill._psAttack._suSkill = SPL_RATTACK;
	}
}

static void CalcItemReqs(int pnum)
{
	int i;
	ItemStruct* pi;
	int sa, ma, da, sc, mc, dc;
	int strReq[NUM_INVLOC];

	sa = plr._pBaseStr;
	ma = plr._pBaseMag;
	da = plr._pBaseDex;

	pi = plr._pInvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++) {
		if (pi->_itype != ITYPE_NONE) {
			pi->_iStatFlag = TRUE;
			//if (!pi->_iUnidentified) {
				sa += pi->_iPLStr;
				ma += pi->_iPLMag;
				da += pi->_iPLDex;
				strReq[i] = pi->_iReqStr == 0 ? INT_MIN : pi->_iReqStr + (pi->_iPLStr > 0 ? pi->_iPLStr : 0);
			//}
		}
	}
recheck:
	pi = plr._pInvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++) {
		if (pi->_itype == ITYPE_NONE)
			continue;
		if (sa >= strReq[i])
			continue;
		if (pi->_iStatFlag) {
			pi->_iStatFlag = FALSE;
			//if (!pi->_iUnidentified) {
				sa -= pi->_iPLStr;
				ma -= pi->_iPLMag;
				da -= pi->_iPLDex;
			//}
			goto recheck;
		}
	}

	sc = std::max(0, sa);
	mc = std::max(0, ma);
	dc = std::max(0, da);

	plr._pStrength = sc;
	plr._pMagic = mc;
	plr._pDexterity = dc;

	pi = &plr._pHoldItem;
	ItemStatOk(pnum, pi);

	pi = plr._pInvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++)
		ItemStatOk(pnum, pi);

	pi = plr._pInvList;
	for (i = NUM_INV_GRID_ELEM; i != 0; i--, pi++)
		ItemStatOk(pnum, pi);

	pi = plr._pSpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++)
		ItemStatOk(pnum, pi);
}

void CalcPlrInv(int pnum, bool Loadgfx)
{
    auto dunLevel = plr._pDunLevel;
    plr._pDunLevel = DLV_CATHEDRAL1;

	CalcItemReqs(pnum);
	CalcPlrItemVals(pnum, Loadgfx);
	//if (pnum == mypnum) {
		CalcPlrSpells(pnum);
		//CalcPlrBookVals(pnum);
		//CalcPlrCharges(pnum);
	//}
    plr._pDunLevel = dunLevel;
}

void SetItemData(int ii, int idata)
{
	SetItemSData(&items[ii], idata);
}

void SetItemSData(ItemStruct* is, int idata)
{
	const ItemData* ids;

	// zero-initialize struct
	memset(is, 0, sizeof(*is));

	is->_iIdx = idata;
	ids = &AllItemList[idata];
	is->_iCurs = ids->iCurs;
	is->_itype = ids->itype;
	is->_iMiscId = ids->iMiscId;
	is->_iSpell = ids->iSpell;
	is->_iClass = ids->iClass;
	is->_iLoc = ids->iLoc;
	is->_iDamType = ids->iDamType;
	is->_iMinDam = ids->iMinDam;
	is->_iMaxDam = ids->iMaxDam;
	is->_iBaseCrit = ids->iBaseCrit;
	is->_iReqStr = ids->iReqStr;
	is->_iReqMag = ids->iReqMag;
	is->_iReqDex = ids->iReqDex;
	is->_iUsable = ids->iUsable;
	ac_rnd = is->_iAC = ids->iMinAC == ids->iMaxAC ? ids->iMinAC : RandRangeLow(ids->iMinAC, ids->iMaxAC);
	is->_iDurability = ids->iUsable ? 1 : ids->iDurability; // STACK
	is->_iMaxDur = ids->iDurability;
	is->_ivalue = ids->iValue;
	is->_iIvalue = ids->iValue;

	if (is->_itype == ITYPE_STAFF && is->_iSpell != SPL_NULL) {
		is->_iCharges = BASESTAFFCHARGES;
		is->_iMaxCharges = is->_iCharges;

		// assert(is->_iNumAffixes == 0);
		is->_iAffixes[0].asPower = IPL_SETSKILL;
		is->_iAffixes[0].asValue0 = is->_iSpell;
		is->_iNumAffixes = 1;
	}

	static_assert(ITEM_QUALITY_NORMAL == 0, "Zero-fill expects ITEM_QUALITY_NORMAL == 0.");
	//is->_iMagical = ITEM_QUALITY_NORMAL;
	static_assert(SPL_NULL == 0, "Zero-fill expects SPL_NULL == 0.");
	//is->_iPLSkill = SPL_NULL;
}

/**
 * @brief Set a new unique seed value on the given item
 * @param is Item to update
 */
void GetItemSeed(ItemStruct* is)
{
	is->_iSeed = NextRndSeed();
}

void CreateBaseItem(ItemStruct* is, int idata)
{
	SetItemSData(is, idata);
	GetItemSeed(is);
}

void SetGoldItemValue(ItemStruct* is, int value)
{
	is->_ivalue = value;
	if (value >= GOLD_MEDIUM_LIMIT)
		is->_iCurs = ICURS_GOLD_LARGE;
	else if (value <= GOLD_SMALL_LIMIT)
		is->_iCurs = ICURS_GOLD_SMALL;
	else
		is->_iCurs = ICURS_GOLD_MEDIUM;
}

void CreatePlrItems(int pnum)
{
	ItemStruct* pi;
	//int i;

	static_assert(ITYPE_NONE == 0, "CreatePlrItems skips item initialization by expecting ITYPE_NONE to be zero.");
	/*plr._pHoldItem._itype = ITYPE_NONE;

	pi = plr._pInvBody;
	for (i = NUM_INVLOC; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}

	pi = plr._pSpdList;
	for (i = MAXBELTITEMS; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}

	pi = plr._pInvList;
	for (i = NUM_INV_GRID_ELEM; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}*/

	switch (plr._pClass) {
	case PC_WARRIOR:
		CreateBaseItem(&plr._pInvBody[INVLOC_HAND_LEFT], IDI_WARRSWORD);
		CreateBaseItem(&plr._pInvBody[INVLOC_HAND_RIGHT], IDI_WARRSHLD);

		CreateBaseItem(&plr._pSpdList[0], IDI_HEAL);
		CreateBaseItem(&plr._pSpdList[1], IDI_HEAL);
		break;
	case PC_ROGUE:
		CreateBaseItem(&plr._pInvBody[INVLOC_HAND_LEFT], IDI_ROGUEBOW);

		CreateBaseItem(&plr._pSpdList[0], IDI_HEAL);
		CreateBaseItem(&plr._pSpdList[1], IDI_HEAL);
		break;
	case PC_SORCERER:
		CreateBaseItem(&plr._pInvBody[INVLOC_HAND_LEFT], IDI_SORCSTAFF);

#ifdef HELLFIRE
		CreateBaseItem(&plr._pSpdList[0], IDI_HEAL);
		CreateBaseItem(&plr._pSpdList[1], IDI_HEAL);
#else
		CreateBaseItem(&plr._pSpdList[0], IDI_MANA);
		CreateBaseItem(&plr._pSpdList[1], IDI_MANA);
#endif
		break;
#ifdef HELLFIRE
	case PC_MONK:
		CreateBaseItem(&plr._pInvBody[INVLOC_HAND_LEFT], IDI_MONKSTAFF);

		CreateBaseItem(&plr._pSpdList[0], IDI_HEAL);
		CreateBaseItem(&plr._pSpdList[1], IDI_HEAL);
		break;
#endif
	}

	pi = &plr._pInvList[0];
	CreateBaseItem(pi, IDI_GOLD);
	SetGoldItemValue(pi, 100);
	plr._pGold = 100;
	// commented out, because it is not necessary (see CreatePlayer)
	// CalcPlrInv(pnum, false);
}

BYTE GetBookSpell(unsigned lvl, int idx)
{
	static_assert((int)NUM_SPELLS < UCHAR_MAX, "GetBookSpell stores spell-ids in BYTEs.");
	BYTE ss[NUM_SPELLS];
	int bs, ns;

	if (lvl < BOOK_MIN)
		lvl = BOOK_MIN;

	ns = 0;
	for (bs = 0; bs < (IsHellfireGame ? NUM_SPELLS : NUM_SPELLS_DIABLO); bs++) {
		if (spelldata[bs].sBookLvl != SPELL_NA && lvl >= spelldata[bs].sBookLvl) {
			// assert(IsMultiGame || bs != SPL_RESURRECT);
			ss[ns] = bs;
			ns++;
		}
	}
	// assert(ns > 0);
    if (idx < -1)
        return ns;
    if (idx >= 0)
        return ss[idx];
	return ss[random_low(14, ns)];
}

static void SetBookSpell(ItemStruct* is, unsigned lvl)
{
	const SpellData* sd;
	int bs;

	bs = GetBookSpell(lvl);

	is->_iSpell = bs;
	sd = &spelldata[bs];

	is->_iReqMag = sd->sReqMag;
	// assert(is->_ivalue == 0 && is->_iIvalue == 0);
	is->_ivalue = sd->sBookCost;
	is->_iIvalue = sd->sBookCost;
	switch (sd->sType) {
	case STYPE_FIRE:
		bs = ICURS_BOOK_RED;
		break;
	case STYPE_LIGHTNING:
		bs = ICURS_BOOK_BLUE;
		break;
	case STYPE_MAGIC:
	case STYPE_NONE:
		bs = ICURS_BOOK_GRAY;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	is->_iCurs = bs;
}

BYTE GetScrollSpell(unsigned lvl, int idx)
{
	static_assert((int)NUM_SPELLS < UCHAR_MAX, "GetScrollSpell stores spell-ids in BYTEs.");
#ifdef HELLFIRE
	static_assert((int)SPL_RUNE_LAST + 1 == (int)NUM_SPELLS, "GetScrollSpell skips spells at the end of the enum.");
	BYTE ss[SPL_RUNE_FIRST];
#else
	BYTE ss[NUM_SPELLS];
#endif
	int bs, ns;

	if (lvl < SCRL_MIN)
		lvl = SCRL_MIN;

	ns = 0;
	for (bs = 0; bs < (IsHellfireGame ? SPL_RUNE_FIRST : NUM_SPELLS_DIABLO); bs++) {
		if (spelldata[bs].sScrollLvl != SPELL_NA && lvl >= spelldata[bs].sScrollLvl
		 && (IsMultiGame || bs != SPL_RESURRECT)) {
			ss[ns] = bs;
			ns++;
		}
	}
	// assert(ns > 0);
    if (idx < -1)
        return ns;
    if (idx >= 0)
        return ss[idx];
	return ss[random_low(14, ns)];
}

static void SetScrollSpell(ItemStruct* is, unsigned lvl)
{
	const SpellData* sd;
	int bs;

	bs = GetScrollSpell(lvl);

	is->_iSpell = bs;
	sd = &spelldata[bs];

	is->_iReqMag = sd->sReqMag > SCRL_MAG ? sd->sReqMag - SCRL_MAG : 0;
	// assert(is->_ivalue == 0 && is->_iIvalue == 0);
	is->_ivalue = sd->sStaffCost;
	is->_iIvalue = sd->sStaffCost;
}

#ifdef HELLFIRE
BYTE GetRuneSpell(unsigned lvl, int idx)
{
	static_assert((int)NUM_SPELLS < UCHAR_MAX, "GetRuneSpell stores spell-ids in BYTEs.");
	BYTE ss[SPL_RUNE_LAST - SPL_RUNE_FIRST + 1];
	int bs, ns;

	if (lvl < RUNE_MIN)
		lvl = RUNE_MIN;

	ns = 0;
	for (bs = SPL_RUNE_FIRST; bs <= SPL_RUNE_LAST; bs++) {
		if (/*spelldata[bs].sScrollLvl != SPELL_NA &&*/ lvl >= spelldata[bs].sScrollLvl
		 /*&& (IsMultiGame || bs != SPL_RESURRECT)*/) {
			ss[ns] = bs;
			ns++;
		}
	}
	// assert(ns > 0);
    if (idx < -1)
        return ns;
    if (idx >= 0)
        return ss[idx];
	return ss[random_low(14, ns)];
}

static void SetRuneSpell(ItemStruct* is, unsigned lvl)
{
	const SpellData* sd;
	int bs;

	bs = GetRuneSpell(lvl);

	is->_iSpell = bs;
	sd = &spelldata[bs];

	is->_iReqMag = sd->sReqMag;
	// assert(is->_ivalue == 0 && is->_iIvalue == 0);
	is->_ivalue = sd->sStaffCost;
	is->_iIvalue = sd->sStaffCost;

	static_assert(ICURS_RUNE_OF_WAVE == ICURS_RUNE_OF_FIRE + SPL_RUNEWAVE - SPL_RUNEFIRE, "SetRuneSpell requires ordered ICURS_RUNE_/SPL_RUNE enums I.");
	static_assert(ICURS_RUNE_OF_LIGHTNING == ICURS_RUNE_OF_FIRE + SPL_RUNELIGHT - SPL_RUNEFIRE, "SetRuneSpell requires ordered ICURS_RUNE_/SPL_RUNE enums II.");
	static_assert(ICURS_RUNE_OF_NOVA == ICURS_RUNE_OF_FIRE + SPL_RUNENOVA - SPL_RUNEFIRE, "SetRuneSpell requires ordered ICURS_RUNE_/SPL_RUNE enums III.");
	static_assert(ICURS_RUNE_OF_STONE == ICURS_RUNE_OF_FIRE + SPL_RUNESTONE - SPL_RUNEFIRE, "SetRuneSpell requires ordered ICURS_RUNE_/SPL_RUNE enums IV.");
	is->_iCurs = ICURS_RUNE_OF_FIRE + bs - SPL_RUNEFIRE;
}
#endif

BYTE GetStaffSpell(unsigned lvl, int idx)
{
	static_assert((int)NUM_SPELLS < UCHAR_MAX, "GetStaffSpell stores spell-ids in BYTEs.");
	BYTE ss[NUM_SPELLS];
	int bs, ns;

	if (lvl < STAFF_MIN)
		lvl = STAFF_MIN;

	ns = 0;
	for (bs = 0; bs < (IsHellfireGame ? NUM_SPELLS : NUM_SPELLS_DIABLO); bs++) {
		if (spelldata[bs].sStaffLvl != SPELL_NA && lvl >= spelldata[bs].sStaffLvl
		 && (IsMultiGame || bs != SPL_RESURRECT)) {
			ss[ns] = bs;
			ns++;
		}
	}
	// assert(ns > 0);
    if (idx < -1)
        return ns;
    if (idx >= 0)
        return ss[idx];
	return ss[random_low(18, ns)];
}

static void SetStaffSpell(ItemStruct* is, unsigned lvl)
{
	const SpellData* sd;
	int bs, v;

	bs = GetStaffSpell(lvl);

	sd = &spelldata[bs];

	is->_iSpell = bs;
	is->_iCharges = RandRangeLow(sd->sStaffMin, sd->sStaffMax);
	is->_iMaxCharges = is->_iCharges;

	// assert(is->_iNumAffixes == 0);
	is->_iAffixes[0].asPower = IPL_SETSKILL;
	is->_iAffixes[0].asValue0 = bs;
	is->_iNumAffixes = 1;

	is->_iReqMag = sd->sReqMag;
	v = is->_iCharges * sd->sStaffCost;
	is->_ivalue += v;
	is->_iIvalue += v;
}

static void GetItemAttrs(int ii, int idata, unsigned lvl)
{
	ItemStruct* is;
	int rndv;

	SetItemData(ii, idata);

	is = &items[ii];
	if (is->_iMiscId == IMISC_BOOK)
		SetBookSpell(is, lvl);
	else if (is->_iMiscId == IMISC_SCROLL)
		SetScrollSpell(is, lvl);
#ifdef HELLFIRE
	else if (is->_iMiscId == IMISC_RUNE)
		SetRuneSpell(is, lvl);
#endif
	else if (is->_itype == ITYPE_GOLD) {
		lvl = items_get_currlevel();
		rndv = RandRangeLow(2 * lvl, 8 * lvl);
		if (rndv > GOLD_MAX_LIMIT)
			rndv = GOLD_MAX_LIMIT;

		SetGoldItemValue(is, rndv);
	}
}

static int PLVal(const AffixData* affix, int pv)
{
	int dp, dv, rv;
	int p1 = affix->PLParam1;
	int p2 = affix->PLParam2;
	int minv = affix->PLMinVal;
	int maxv = affix->PLMaxVal;

	rv = minv;
	dp = p2 - p1;
	if (dp != 0) {
		dv = maxv - minv;
		if (dv != 0)
			rv += dv * (pv - p1) / dp;
	}
	return rv;
}

static int SaveItemPower(ItemStruct* is, int power, int param1, int param2)
{
	ItemAffixStruct* ias;
	int r2;

	ias = &is->_iAffixes[is->_iNumAffixes];
	is->_iNumAffixes++;
	ias->asPower = power;

	const int r = param1 == param2 ? param1 : RandRangeLow(param1, param2);
	ias->asValue0 = r;

	switch (power) {
	case IPL_TOHIT:
		is->_iPLToHit = r;
		break;
	case IPL_DAMP:
		is->_iPLDam = r;
		break;
	case IPL_TOHIT_DAMP:
		is->_iPLDam = r;
		r2 = RandRangeLow(param1 >> 2, param2 >> 2);
		is->_iPLToHit = r2;
		break;
	case IPL_ACP:
	case IPL_TOBLOCK:
	case IPL_FIRERES:
	case IPL_LIGHTRES:
	case IPL_MAGICRES:
	case IPL_ACIDRES:
	case IPL_ALLRES:
	case IPL_CRITP:
	case IPL_POWMOD:
		break;
	case IPL_SKILLLVL:
		ias->asValue1 = GetBookSpell(is->_iCreateInfo & CF_LEVEL);
		break;
	case IPL_SKILLLEVELS:
		break;
	case IPL_CHARGES:
		is->_iCharges *= r;
		is->_iMaxCharges = is->_iCharges;
		break;
	case IPL_FIREDAM:
	case IPL_LIGHTDAM:
	case IPL_MAGICDAM:
	case IPL_ACIDDAM:
		ias->asFrom = param1;
		ias->asTo = param2;
		break;
	case IPL_STR:
		is->_iPLStr = r;
		break;
	case IPL_MAG:
		is->_iPLMag = r;
		break;
	case IPL_DEX:
		is->_iPLDex = r;
		break;
	case IPL_VIT:
		is->_iPLVit = r;
		break;
	case IPL_ATTRIBS:
		is->_iPLStr = r;
		is->_iPLMag = r;
		is->_iPLDex = r;
		is->_iPLVit = r;
		break;
	case IPL_ABS_ANYHIT:
	case IPL_ABS_PHYHIT:
	case IPL_LIFE:
	case IPL_MANA:
		break;
	case IPL_DUR:
		r2 = r * is->_iMaxDur / 100;
		is->_iDurability = is->_iMaxDur = is->_iMaxDur + r2;
		break;
	case IPL_INDESTRUCTIBLE:
		is->_iDurability = is->_iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IPL_LIGHT:
		break;
	// case IPL_INVCURS:
	//	is->_iCurs = param1;
	//	break;
	//case IPL_THORNS:
	//	break;
	case IPL_NOMANA:
	case IPL_KNOCKBACK:
	case IPL_STUN:
	case IPL_NO_BLEED:
	case IPL_BLEED:
	//case IPL_NOHEALMON:
	case IPL_STEALMANA:
	case IPL_STEALLIFE:
	case IPL_PENETRATE_PHYS:
	case IPL_FASTATTACK:
	case IPL_FASTRECOVER:
	case IPL_FASTBLOCK:
	case IPL_DAMMOD:
		break;
	case IPL_SETDAM:
		is->_iMinDam = param1;
		is->_iMaxDam = param2;
		break;
	case IPL_SETDUR:
		is->_iDurability = is->_iMaxDur = r;
		break;
	case IPL_REQSTR:
		is->_iReqStr += r;
		break;
	case IPL_SKILL:
		param1 = GetStaffSpell(is->_iCreateInfo & CF_LEVEL);
		param2 = RandRangeLow(spelldata[param1].sStaffMin, spelldata[param1].sStaffMax);

		r2 = param2 * spelldata[param1].sStaffCost;
		is->_ivalue += r2;
		is->_iIvalue += r2;
		/* fall-through */
	case IPL_SETSKILL:
		ias->asValue0 = param1;

		is->_iSpell = param1;
		is->_iCharges = param2;
		is->_iMaxCharges = param2;
		is->_iReqMag = spelldata[param1].sReqMag;
		break;
	case IPL_ONEHAND:
		is->_iLoc = ILOC_ONEHAND;
		break;
	case IPL_ALLRESZERO:
	case IPL_DRAINLIFE:
	//case IPL_INFRAVISION:
		break;
	case IPL_SETAC:
		is->_iAC = r;
		break;
	case IPL_ACMOD:
		is->_iAC += r;
		break;
	case IPL_CRYSTALLINE:
		is->_iPLDam = r * 2;
		is->_iDurability = is->_iMaxDur = r < 100 ? (is->_iMaxDur - r * is->_iMaxDur / 100) : 1;
		break;
	case IPL_MANATOLIFE:
	case IPL_LIFETOMANA:
	case IPL_FASTCAST:
	case IPL_FASTWALK:
		break;
	default:
		ASSUME_UNREACHABLE
	}
	return r;
}

static void AddItemAffix(const AffixData *pres, int flgs, BYTE range, unsigned lvl, BOOLEAN good, ItemStruct* is, INTPAIR& valmod, int i)
{
	int v, tw = 0;
	std::pair<const AffixData*, int> lw[ITEM_RNDAFFIX_MAX];
	std::pair<const AffixData*, int>* lwp = &lw[0];
	for ( ; pres->PLRnd != 0; pres++) {
		if ((flgs & pres->PLIType)
			&& pres->PLRanges[range].from <= lvl && pres->PLRanges[range].to >= lvl
			// && (!onlygood || pres->PLOk)) {
			&& (good <= pres->PLOk)) {
			tw += pres->PLRnd;
			lwp->first = pres;
			lwp->second = tw;
			lwp++;
		}
	}
	if (tw != 0) {
		// assert(tw <= 0x7FFF);
		tw = random_low(23, tw);
		lwp = &lw[0];
		while (tw >= lwp->second) {
			lwp++;
		}
		pres = lwp->first;
		is->_iMagical = ITEM_QUALITY_MAGIC;
		is->_iUnidentified = TRUE;
		affix_rnd[i] = v = SaveItemPower(
			is,
			pres->PLPower,
			pres->PLParam1,
			pres->PLParam2);
		valmod.v1 += PLVal(pres, v);
		valmod.v0 += pres->PLMultVal;
	}
}

static void GetItemPower(ItemStruct* is, unsigned lvl, BYTE range, int flgs, bool onlygood)
{
	int v;
	INTPAIR valmod = { 0 , 0 };
	BYTE affix;
	BOOLEAN good;

	// assert(is->_iMagical == ITEM_QUALITY_NORMAL);
	if (flgs != PLT_JEWEL) // is->_itype != ITYPE_RING && is->_itype != ITYPE_AMULET)
		lvl = lvl > AllItemList[is->_iIdx].iMinMLvl ? lvl - AllItemList[is->_iIdx].iMinMLvl : 0;

	// select affixes (3: both, 2: prefix, 1: suffix)
	v = random_(23, 128);
	affix = v < 21 ? 3 : (v < 48 ? 2 : 1);
	static_assert(TRUE > FALSE, "GetItemPower assumes TRUE is greater than FALSE.");
	good = (onlygood || random_(0, 3) != 0) ? TRUE : FALSE;
	if (affix >= 2) {
		AddItemAffix(PL_Prefix, flgs, range, lvl, good, is, valmod, 0);
	}
	if (affix & 1) {
		AddItemAffix(PL_Suffix, flgs, range, lvl, good, is, valmod, 1);
	}
	// prefix or suffix added -> recalculate the value of the item
	if (is->_iMagical != ITEM_QUALITY_NORMAL) {
		if (is->_iMiscId != IMISC_MAP) {
			v = valmod.v0;
			if (v >= 0) {
				v *= is->_ivalue;
			} else {
				v = is->_ivalue / -v;
			}
			v += valmod.v1;
			if (v <= 0) {
				v = 1;
			}
		} else {
			v = 6;
			for (unsigned i = 0; i < is->_iNumAffixes; i++) {
				const ItemAffixStruct* ias = &is->_iAffixes[i];
				if (ias->asPower == IMP_AREAMOD) {
					v -= ias->asValue0;
				}
			}
			v = ((1 << MAXCAMPAIGNSIZE) - 1) >> v;
			is->_ivalue = v;
		}
		is->_iIvalue = v;
	}
}

static void GetItemBonus(int ii, unsigned lvl, BYTE range, bool onlygood, bool allowspells)
{
	int flgs;
	ItemStruct* is = &items[ii];

	switch (is->_itype) {
	case ITYPE_MISC:
		if (items[ii]._iMiscId != IMISC_MAP)
			return;
		flgs = PLT_MAP;
		break;
	case ITYPE_SWORD:
	case ITYPE_AXE:
	case ITYPE_MACE:
		flgs = PLT_MELEE;
		break;
	case ITYPE_BOW:
		flgs = PLT_BOW;
		break;
	case ITYPE_SHIELD:
		flgs = PLT_SHLD;
		break;
	case ITYPE_LARMOR:
		flgs = PLT_LARMOR;
		break;
	case ITYPE_HELM:
		flgs = PLT_HELM;
		break;
	case ITYPE_MARMOR:
		flgs = PLT_MARMOR;
		break;
	case ITYPE_HARMOR:
		flgs = PLT_HARMOR;
		break;
	case ITYPE_STAFF:
		flgs = PLT_STAFF;
		if (allowspells && random_(17, 4) != 0) {
			SetStaffSpell(is, lvl);
			if (random_(51, 2) != 0)
				return;
			flgs |= PLT_CHRG;
		}
		break;
	case ITYPE_GOLD:
		return;
	case ITYPE_RING:
	case ITYPE_AMULET:
		flgs = PLT_JEWEL;
		break;
	default:
		ASSUME_UNREACHABLE
		return;
	}

	GetItemPower(is, lvl, range, flgs, onlygood);
}

static int RndDropItem(bool func(const ItemData& item, void* arg), void* arg, unsigned lvl)
{
#if UNOPTIMIZED_RNDITEMS
	int i, j, ri;
	int ril[ITEM_RNDDROP_MAX];

	ri = 0;
	for (i = IDI_RNDDROP_FIRST; i < NUM_IDI; i++) {
		if (!func(AllItemList[i], arg) || lvl < AllItemList[i].iMinMLvl)
			continue;
		for (j = AllItemList[i].iRnd; j > 0; j--) {
			ril[ri] = i;
			ri++;
		}
	}
	assert(ri != 0);
	return ril[random_(50, ri)];
#else
	int i, ri;
	int ril[NUM_IDI - IDI_RNDDROP_FIRST];

	for (i = IDI_RNDDROP_FIRST; i < (IsHellfireGame ? NUM_IDI : NUM_IDI_DIABLO); i++) {
		ril[i - IDI_RNDDROP_FIRST] = (!func(AllItemList[i], arg) || lvl < AllItemList[i].iMinMLvl) ? 0 : AllItemList[i].iRnd;
	}
	ri = 0;
	for (i = 0; i < ((IsHellfireGame ? NUM_IDI : NUM_IDI_DIABLO) - IDI_RNDDROP_FIRST); i++)
		ri += ril[i];
	// assert(ri != 0 && ri <= 0x7FFF);
	ri = random_low(50, ri);
	for (i = 0; ; i++) {
		ri -= ril[i];
		if (ri < 0)
			break;
	}
	return i + IDI_RNDDROP_FIRST;
#endif
}

static bool RndUItemOk(const ItemData& item, void* arg)
{
	// assert(item.itype != ITYPE_GOLD);
	return item.itype != ITYPE_MISC || item.iMiscId == IMISC_BOOK || item.iMiscId == IMISC_MAP;
}

static int RndUItem(unsigned lvl)
{
	return RndDropItem(RndUItemOk, NULL, lvl);
}

static bool RndItemOk(const ItemData& item, void* arg)
{
	return true;
}

static int RndAnyItem(unsigned lvl)
{
	if (random_(26, 128) > 32)
		return IDI_GOLD;

	return RndDropItem(RndItemOk, NULL, lvl);
}

typedef struct RndTypeItemParam {
	int itype;
	int imid;
} RndTypeItemParam;

static bool RndTypeItemOk(const ItemData& item, void* arg)
{
	RndTypeItemParam* param = (RndTypeItemParam*)arg;
	return item.itype == param->itype && /*imid == IMISC_INVALID ||*/ item.iMiscId == param->imid;
}

static int RndTypeItem(int itype, int imid, unsigned lvl)
{
	RndTypeItemParam param = { itype, imid };
	// assert(itype != ITYPE_GOLD);
	return RndDropItem(RndTypeItemOk, &param, lvl);
}

static int CheckUnique(int ii, unsigned lvl, unsigned quality)
{
	int i, ui;
	BYTE uok[NUM_UITEM];
	const ItemData &item = AllItemList[items[ii]._iIdx];
	const BYTE uid = item.iUniqType;
	if (uid == UITYPE_NONE || (item.iRnd != 0 && random_(28, 100) > (quality == CFDQ_UNIQUE ? 15 : 1)))
		return -1;

	static_assert(NUM_UITEM <= UCHAR_MAX, "Unique index must fit to a BYTE in CheckUnique.");

	ui = 0;
	for (i = 0; i < (IsHellfireGame ? NUM_UITEM : NUM_UITEM_DIABLO); i++) {
		if (UniqueItemList[i].UIUniqType == uid
		 && lvl >= UniqueItemList[i].UIMinLvl) {
			uok[ui] = i;
			ui++;
		}
	}

	if (ui == 0)
		return -1;

	return uok[random_low(29, ui)];
}

static void GetUniqueItem(int ii, int uid)
{
	const UniqItemData* ui;
	ItemStruct* is = &items[ii];

	ui = &UniqueItemList[uid];
	affix_rnd[0] = SaveItemPower(is, ui->UIPower1, ui->UIParam1a, ui->UIParam1b);

	if (ui->UIPower2 != IPL_INVALID) {
		affix_rnd[1] = SaveItemPower(is, ui->UIPower2, ui->UIParam2a, ui->UIParam2b);
	if (ui->UIPower3 != IPL_INVALID) {
		affix_rnd[2] = SaveItemPower(is, ui->UIPower3, ui->UIParam3a, ui->UIParam3b);
	if (ui->UIPower4 != IPL_INVALID) {
		affix_rnd[3] = SaveItemPower(is, ui->UIPower4, ui->UIParam4a, ui->UIParam4b);
	if (ui->UIPower5 != IPL_INVALID) {
		affix_rnd[4] = SaveItemPower(is, ui->UIPower5, ui->UIParam5a, ui->UIParam5b);
	if (ui->UIPower6 != IPL_INVALID) {
		affix_rnd[5] = SaveItemPower(is, ui->UIPower6, ui->UIParam6a, ui->UIParam6b);
	}}}}}

	is->_iCurs = ui->UICurs;
	is->_iIvalue = ui->UIValue;

	is->_iUid = uid;
	is->_iMagical = ITEM_QUALITY_UNIQUE;
	is->_iUnidentified = TRUE;
	// is->_iCreateInfo |= CF_UNIQUE;
}

static void ItemRndDur(int ii)
{
	// skip STACKable and non-durable items
	if (!items[ii]._iUsable && items[ii]._iMaxDur > 1 && items[ii]._iMaxDur != DUR_INDESTRUCTIBLE) {
		// assert((items[ii]._iMaxDur >> 1) <= 0x7FFF);
		items[ii]._iDurability = random_low(0, items[ii]._iMaxDur >> 1) + (items[ii]._iMaxDur >> 2) + 1;
	}
}

static void SetupItem(int ii, int idx, int32_t iseed, unsigned lvl, unsigned quality)
{
	int uid;

	SetRndSeed(iseed);
	GetItemAttrs(ii, idx, lvl);
	items[ii]._iSeed = iseed;
	items[ii]._iCreateInfo = lvl;

	items[ii]._iCreateInfo |= quality << 11;

		if (quality >= CFDQ_GOOD
		 || items[ii]._itype == ITYPE_STAFF
		 || items[ii]._itype == ITYPE_RING
		 || items[ii]._itype == ITYPE_AMULET
		 || random_(32, 128) < 14 || (unsigned)random_(33, 128) <= lvl) {
			uid = CheckUnique(ii, lvl, quality);
			if (uid < 0) {
				GetItemBonus(ii, lvl, IAR_DROP, quality >= CFDQ_GOOD, true);
			} else {
				GetUniqueItem(ii, uid);
				return;
			}
		}
		// if (items[ii]._iMagical != ITEM_QUALITY_UNIQUE)
			ItemRndDur(ii);
}

static void PrintEquipmentPower(BYTE idx, const ItemStruct* is)
{
	const ItemAffixStruct* ias = &is->_iAffixes[idx];
	BYTE plidx = ias->asPower;
	switch (plidx) {
	case IPL_TOHIT:
		snprintf(tempstr, sizeof(tempstr), "chance to hit: %+d%%", is->_iPLToHit);
		break;
	case IPL_DAMP:
		snprintf(tempstr, sizeof(tempstr), "%+d%% damage", is->_iPLDam);
		break;
	case IPL_TOHIT_DAMP:
		snprintf(tempstr, sizeof(tempstr), "to hit: %+d%%, %+d%% damage", is->_iPLToHit, is->_iPLDam);
		break;
	case IPL_ACP:
		snprintf(tempstr, sizeof(tempstr), "%+d%% armor", ias->asValue0);
		break;
	case IPL_TOBLOCK:
		snprintf(tempstr, sizeof(tempstr), "%+d%% block chance", ias->asValue0);
		break;
	case IPL_FIRERES:
	case IPL_LIGHTRES:
	case IPL_MAGICRES:
	case IPL_ACIDRES:
	case IPL_ALLRES: {
		const char* element;
		switch (plidx) {
		case IPL_FIRERES:  element = "fire";      break;
		case IPL_LIGHTRES: element = "lightning"; break;
		case IPL_MAGICRES: element = "magic";     break;
		case IPL_ACIDRES:  element = "acid";      break;
		case IPL_ALLRES:   element = "all";       break;
		default: ASSUME_UNREACHABLE;              break;
		}
		//if (ias->asValue0 < 75)
		snprintf(tempstr, sizeof(tempstr), "resist %s: %+d%%", element, ias->asValue0);
		//else
		//	copy_cstr(tempstr, "resist %s: 75% MAX", element);
	} break;
	case IPL_CRITP:
		snprintf(tempstr, sizeof(tempstr), "%d%% increased crit. chance", ias->asValue0);
		break;
	case IPL_POWMOD:
		snprintf(tempstr, sizeof(tempstr), "%d%% increased magic power", ias->asValue0);
		break;
	case IPL_SKILLLVL:
		snprintf(tempstr, sizeof(tempstr), "%+d to %s", ias->asValue0, spelldata[ias->asValue1].sNameText);
		break;
	case IPL_SKILLLEVELS:
		snprintf(tempstr, sizeof(tempstr), "%+d to skill levels", ias->asValue0);
		break;
	case IPL_CHARGES:
		copy_cstr(tempstr, "extra charges");
		break;
	case IPL_FIREDAM:
	case IPL_LIGHTDAM:
	case IPL_MAGICDAM:
	case IPL_ACIDDAM:
	{
		const char* element;
		BYTE mindam = ias->asFrom, maxdam = ias->asTo;
		switch (plidx) {
		case IPL_FIREDAM:  element = "fire";      break;
		case IPL_LIGHTDAM: element = "lightning"; break;
		case IPL_MAGICDAM: element = "magic";     break;
		case IPL_ACIDDAM:  element = "acid";      break;
		default: ASSUME_UNREACHABLE;              break;
		}
		if (mindam != maxdam)
			snprintf(tempstr, sizeof(tempstr), "%s damage: %d-%d", element, mindam, maxdam);
		else
			snprintf(tempstr, sizeof(tempstr), "%s damage: %d", element, mindam);
	} break;
	case IPL_STR:
		snprintf(tempstr, sizeof(tempstr), "%+d to strength", is->_iPLStr);
		break;
	case IPL_MAG:
		snprintf(tempstr, sizeof(tempstr), "%+d to magic", is->_iPLMag);
		break;
	case IPL_DEX:
		snprintf(tempstr, sizeof(tempstr), "%+d to dexterity", is->_iPLDex);
		break;
	case IPL_VIT:
		snprintf(tempstr, sizeof(tempstr), "%+d to vitality", is->_iPLVit);
		break;
	case IPL_ATTRIBS:
		snprintf(tempstr, sizeof(tempstr), "%+d to all attributes", is->_iPLStr);
		break;
	case IPL_ABS_ANYHIT:
		snprintf(tempstr, sizeof(tempstr), "%+d damage taken", -ias->asValue0);
		break;
	case IPL_ABS_PHYHIT:
		snprintf(tempstr, sizeof(tempstr), "%+d phys. damage taken", -ias->asValue0);
		break;
	case IPL_LIFE:
		snprintf(tempstr, sizeof(tempstr), "hit points: %+d", ias->asValue0);
		break;
	case IPL_MANA:
		snprintf(tempstr, sizeof(tempstr), "mana: %+d", ias->asValue0);
		break;
	case IPL_DUR:
	case IPL_SETDUR:
		copy_cstr(tempstr, "altered durability");
		break;
	case IPL_INDESTRUCTIBLE:
		copy_cstr(tempstr, "indestructible");
		break;
	case IPL_LIGHT:
		snprintf(tempstr, sizeof(tempstr), "%+d%% light radius", 10 * ias->asValue0);
		break;
	// case IPL_INVCURS:
	//	copy_cstr(tempstr, " ");
	//	break;
	//case IPL_THORNS:
	//	copy_cstr(tempstr, "attacker takes 1-3 damage");
	//	break;
	case IPL_NOMANA:
		copy_cstr(tempstr, "user loses all mana");
		break;
	case IPL_KNOCKBACK:
		copy_cstr(tempstr, "knocks target back");
		break;
	case IPL_STUN:
		copy_cstr(tempstr, "reduces stun threshold");
		break;
	case IPL_NO_BLEED:
		copy_cstr(tempstr, "immune to bleeding");
		break;
	case IPL_BLEED:
		copy_cstr(tempstr, "increased chance to bleed");
		break;
	//case IPL_NOHEALMON:
	//	copy_cstr(tempstr, "hit monster doesn't heal");
	//	break;
	case IPL_STEALMANA:
	case IPL_STEALLIFE:
		snprintf(tempstr, sizeof(tempstr), "hit steals %d%% %s", (ias->asValue0 * 100 + 64) >> 7, plidx == IPL_STEALMANA ? "mana" : "life");
		break;
	case IPL_PENETRATE_PHYS:
		copy_cstr(tempstr, "penetrates target's armor");
		break;
	case IPL_FASTATTACK:
	{
		int v = (ias->asValue0 < 0 ? 12 : 24) * ias->asValue0;
		snprintf(tempstr, sizeof(tempstr), "%+d%% attack speed", v);
	} break;
	case IPL_FASTRECOVER:
		if (ias->asValue0 == 3)
			copy_cstr(tempstr, "fastest hit recovery");
		else if (ias->asValue0 == 2)
			copy_cstr(tempstr, "faster hit recovery");
		else // if (ias->asValue0 == 1)
			copy_cstr(tempstr, "fast hit recovery");
		break;
	case IPL_FASTBLOCK:
		copy_cstr(tempstr, "fast block");
		break;
	case IPL_DAMMOD:
		snprintf(tempstr, sizeof(tempstr), "adds %d points to damage", ias->asValue0);
		break;
	case IPL_SETDAM:
		copy_cstr(tempstr, "unusual item damage");
		break;
	case IPL_REQSTR:
		copy_cstr(tempstr, "altered requirements");
		break;
	case IPL_SKILL:
	case IPL_SETSKILL:
		snprintf(tempstr, sizeof(tempstr), "%s (%d/%d)", spelldata[ias->asValue0].sNameText, is->_iCharges, is->_iMaxCharges);
		break;
	case IPL_ONEHAND:
		copy_cstr(tempstr, "one handed sword");
		break;
	case IPL_ALLRESZERO:
		copy_cstr(tempstr, "all Resistance equals 0");
		break;
	case IPL_DRAINLIFE:
		copy_cstr(tempstr, "constantly lose hit points");
		break;
	//case IPL_INFRAVISION:
	//	copy_cstr(tempstr, "see with infravision");
	//	break;
	case IPL_SETAC:
	case IPL_ACMOD:
		snprintf(tempstr, sizeof(tempstr), "armor class: %d", is->_iAC);
		break;
	case IPL_CRYSTALLINE:
		snprintf(tempstr, sizeof(tempstr), "alt. dur, %+d%% damage", is->_iPLDam);
		break;
	case IPL_MANATOLIFE:
		copy_cstr(tempstr, "50% Mana moved to Health");
		break;
	case IPL_LIFETOMANA:
		copy_cstr(tempstr, "50% Health moved to Mana");
		break;
	case IPL_FASTCAST:
		if (ias->asValue0 == 3)
			copy_cstr(tempstr, "fastest cast");
		else if (ias->asValue0 == 2)
			copy_cstr(tempstr, "faster cast");
		else // if (ias->asValue0 == 1)
			copy_cstr(tempstr, "fast cast");
		break;
	case IPL_FASTWALK:
		if (ias->asValue0 == 3)
			copy_cstr(tempstr, "fastest walk");
		else if (ias->asValue0 == 2)
			copy_cstr(tempstr, "faster walk");
		else // if (ias->asValue0 == 1)
			copy_cstr(tempstr, "fast walk");
		break;
	default:
		ASSUME_UNREACHABLE
        snprintf(tempstr, sizeof(tempstr), "unhandled affix %d", plidx);
		break;
	}
}

static void PrintMapPower(BYTE idx, const ItemStruct* is)
{
	const ItemAffixStruct* ias = &is->_iAffixes[idx];
	BYTE plidx = ias->asPower;
	int v = ias->asValue0;
	switch (plidx) {
	case IMP_LVLMOD:
		snprintf(tempstr, sizeof(tempstr), "%+d to map levels", v);
		break;
	case IMP_LVLGAIN:
		snprintf(tempstr, sizeof(tempstr), "%+d to level gain", v);
		break;
	case IMP_SETLVL:
		snprintf(tempstr, sizeof(tempstr), "starting level %d", v);
		break;
	case IMP_AREAMOD: {
		snprintf(tempstr, sizeof(tempstr), abs(v) == 1 ? "%+d area" : "%+d areas", v);
	} break;
	default:
		ASSUME_UNREACHABLE
        snprintf(tempstr, sizeof(tempstr), "unhandled map affix %d", plidx);
		break;
	}
}

void PrintItemPower(BYTE plidx, const ItemStruct* is)
{
	if (is->_iMiscId != IMISC_MAP)
		PrintEquipmentPower(plidx, is);
	else
		PrintMapPower(plidx, is);
}

const char* ItemName(const ItemStruct* is)
{
	const char* name = AllItemList[is->_iIdx].iName;
	if (is->_iIdx == IDI_EAR) {
		snprintf(tempstr, sizeof(tempstr), "%s of %s", name, is->_iPlrName);
		name = tempstr;
	} else if (is->_iMagical == ITEM_QUALITY_UNIQUE) {
		if (!is->_iUnidentified)
			name = UniqueItemList[is->_iUid].UIName;
	} else if (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_BOOK
#ifdef HELLFIRE
		|| is->_iMiscId == IMISC_RUNE
#endif
		) {
		snprintf(tempstr, sizeof(tempstr), "%s of %s", name, spelldata[is->_iSpell].sNameText);
		name = tempstr;
	}
	return name;
}

void ItemStatOk(int pnum, ItemStruct* is)
{
	is->_iStatFlag = plr._pStrength >= is->_iReqStr
				  && plr._pDexterity >= is->_iReqDex
				  && plr._pMagic >= is->_iReqMag;
}

static bool SmithItemOk(const ItemData& item, void* arg)
{
	return item.itype != ITYPE_MISC
	 && item.itype != ITYPE_GOLD
	 && item.itype != ITYPE_RING
	 && item.itype != ITYPE_AMULET;
}

static int RndSmithItem(unsigned lvl)
{
	return RndDropItem(SmithItemOk, NULL, lvl);
}

static bool WitchItemOk(const ItemData& item, void* arg)
{
	return item.itype == ITYPE_STAFF
		|| item.iMiscId == IMISC_SCROLL
#ifdef HELLFIRE
		|| item.iMiscId == IMISC_RUNE
#endif
		;
}

static int RndWitchItem(unsigned lvl)
{
	return RndDropItem(WitchItemOk, NULL, lvl);
}

static bool HealerItemOk(const ItemData& item, void* arg)
{
	return item.iMiscId == IMISC_REJUV
		|| item.iMiscId == IMISC_FULLREJUV
		|| item.iMiscId == IMISC_SCROLL;
}

static int RndHealerItem(unsigned lvl)
{
	return RndDropItem(HealerItemOk, NULL, lvl);
}

ItemStruct* PlrItem(int pnum, int cii)
{
	ItemStruct* pi;

	if (cii <= INVITEM_INV_LAST) {
		if (cii < INVITEM_INV_FIRST) {
			pi = &plr._pInvBody[cii];
		} else {
			pi = &plr._pInvList[cii - INVITEM_INV_FIRST];
			if (pi->_itype == ITYPE_PLACEHOLDER)
				pi = &plr._pInvList[pi->_iPHolder];
		}
	} else {
		pi = &plr._pSpdList[cii - INVITEM_BELT_FIRST];
	}
	return pi;
}

static void BubbleSwapItem(ItemStruct* a, ItemStruct* b)
{
	ItemStruct h;

	copy_pod(h, *a);
	copy_pod(*a, *b);
	copy_pod(*b, h);
}

/*
 * dst_ii: the destination of the item (inv_item). INVITEM_NONE to discard the source item
 * src_ii: the source of the item (inv_item). INVITEM_NONE to place the destination item somewhere in the inventory
 */
bool SwapPlrItem(int pnum, int dst_ii, int src_ii)
{
    if (dst_ii == src_ii) {
        // LogErrorF("SwapPlrItem no swap 0 %d", dst_ii);
        return false;
    }

    ItemStruct* si;
    if (src_ii != INVITEM_NONE) {
        si = PlrItem(pnum, src_ii);
        // LogErrorF("SwapPlrItem swap 0 %d:%d", dst_ii, src_ii);
        if (dst_ii == INVITEM_NONE) {
            if (si->_itype == ITYPE_NONE)
                return false;
            si->_itype = ITYPE_NONE;
            return true;
        }
    } else {
        si = &items[MAXITEMS];
        si->_itype = ITYPE_NONE;
        // LogErrorF("SwapPlrItem swap 1 %d:%d", dst_ii, src_ii);
        //if (dst_ii < INVITEM_INV_FIRST) { // INVITEM_BODY_LAST
            for (int ii = INVITEM_INV_FIRST; ii <= INVITEM_INV_LAST; ii++) {
                ItemStruct *ci = PlrItem(pnum, ii);
                if (ci->_itype == ITYPE_NONE) {
                    si = ci;
                    // LogErrorF("SwapPlrItem swap 2 %d", ii);
                    break;
                }
            }
        //}
    }

    ItemStruct* di = PlrItem(pnum, dst_ii);
    if (si->_itype == ITYPE_NONE && di->_itype == ITYPE_NONE)
        return false;
    // LogErrorF("SwapPlrItem swap 3 %d / %d", dst_ii, di != nullptr);
    BubbleSwapItem(si, di);
    // LogErrorF("SwapPlrItem swap done");
    return true;
}
static void RecreateSmithItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	// SetRndSeed(iseed);
	GetItemAttrs(ii, RndSmithItem(lvl), lvl);

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_SMITH;
}

static void RecreatePremiumItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	// SetRndSeed(iseed);
	GetItemAttrs(ii, RndSmithItem(lvl), lvl);
	GetItemBonus(ii, lvl, IAR_SHOP, true, false);

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_SMITHPREMIUM;
}

static void RecreateBoyItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	// SetRndSeed(iseed);
	GetItemAttrs(ii, RndSmithItem(lvl), lvl);
	GetItemBonus(ii, lvl, IAR_SHOP, true, true);

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_BOY;
}

static void RecreateWitchItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	/*if (idx == IDI_MANA || idx == IDI_FULLMANA || idx == IDI_PORTAL) {
		SetItemData(ii, idx);
	} else {*/
		// SetRndSeed(iseed);
		GetItemAttrs(ii, RndWitchItem(lvl), lvl);
		// if (random_(51, 100) <= 5 || items[ii]._itype == ITYPE_STAFF)
			GetItemBonus(ii, lvl, IAR_SHOP, true, true);
	//}

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_WITCH;
}

static void RecreateHealerItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	/*if (idx == IDI_HEAL || idx == IDI_FULLHEAL || idx == IDI_RESURRECT) {
		SetItemData(ii, idx);
	} else {*/
		// SetRndSeed(iseed);
		GetItemAttrs(ii, RndHealerItem(lvl), lvl);
	//}

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_HEALER;
}

static void RecreateCraftedItem(int ii/*, int iseed*/, int idx, unsigned lvl)
{
	// SetRndSeed(iseed);
	GetItemAttrs(ii, idx, lvl);
	if (random_(51, 2) != 0)
		GetItemBonus(ii, lvl, IAR_CRAFT, true, true);

	//items[ii]._iSeed = iseed;
	//items[ii]._iCreateInfo = lvl | CF_CRAFTED;
}

static void RecreateTownItem(int ii, uint16_t idx, int32_t iseed, uint16_t icreateinfo)
{
	int loc;
	unsigned lvl;

	loc = (icreateinfo & CF_TOWN) >> 8;
	lvl = icreateinfo & CF_LEVEL;
	SetRndSeed(iseed);
	switch (loc) {
	case CFL_SMITH:
		RecreateSmithItem(ii, /*iseed, */idx, lvl);
		break;
	case CFL_SMITHPREMIUM:
		RecreatePremiumItem(ii, /*iseed, */idx, lvl);
		break;
	case CFL_BOY:
		RecreateBoyItem(ii, /*iseed, */idx, lvl);
		break;
	case CFL_WITCH:
		RecreateWitchItem(ii, /*iseed, */idx, lvl);
		break;
	case CFL_HEALER:
		RecreateHealerItem(ii, /*iseed, */idx, lvl);
		break;
	case CFL_CRAFTED:
		RecreateCraftedItem(ii, /*iseed, */idx, lvl);
		break;
	default:
		ASSUME_UNREACHABLE;
		break;
	}
}

void RecreateItem(int32_t iseed, uint16_t wIndex, uint16_t wCI)
{
	if (wIndex == IDI_GOLD) {
		SetItemData(MAXITEMS, IDI_GOLD);
		//items[MAXITEMS]._iSeed = iseed;
		//items[MAXITEMS]._iCreateInfo = wCI;
	} else {
		if ((wCI & ~CF_LEVEL) == 0) {
			SetItemData(MAXITEMS, wIndex);
			//items[MAXITEMS]._iSeed = iseed;
			//items[MAXITEMS]._iCreateInfo = wCI;
		} else {
			if (wCI & CF_TOWN) {
				RecreateTownItem(MAXITEMS, wIndex, iseed, wCI);
			//	items[MAXITEMS]._iSeed = iseed;
			//	items[MAXITEMS]._iCreateInfo = wCI;
			//} else if ((wCI & CF_USEFUL) == CF_USEFUL) {
			//	SetupUsefulItem(MAXITEMS, iseed, wCI & CF_LEVEL);
			} else {
				SetupItem(MAXITEMS, wIndex, iseed, wCI & CF_LEVEL, (wCI & CF_DROP_QUALITY) >> 11); //, onlygood);
			}
		}
	}
	items[MAXITEMS]._iSeed = iseed;
	items[MAXITEMS]._iCreateInfo = wCI;

    snprintf(items[MAXITEMS]._iName, sizeof(items[MAXITEMS]._iName), "%s", ItemName(&items[MAXITEMS]));
}

int GetItemBonusFlags(int itype, int misc_id)
{
    int flgs = 0;
    switch (itype) {
    case ITYPE_MISC:
        if (misc_id != IMISC_MAP) {
            if (misc_id == IMISC_BOOK || misc_id == IMISC_SCROLL || misc_id == IMISC_RUNE)
                flgs = PLT_MISC;
            break;
        }
        flgs = PLT_MAP;
        break;
    case ITYPE_SWORD:
    case ITYPE_AXE:
    case ITYPE_MACE:
        flgs = PLT_MELEE;
        break;
    case ITYPE_BOW:
        flgs = PLT_BOW;
        break;
    case ITYPE_SHIELD:
        flgs = PLT_SHLD;
        break;
    case ITYPE_LARMOR:
        flgs = PLT_ARMO | PLT_LARMOR;
        break;
    case ITYPE_HELM:
        flgs = PLT_ARMO;
        break;
    case ITYPE_MARMOR:
        flgs = PLT_ARMO | PLT_MARMOR;
        break;
    case ITYPE_HARMOR:
        flgs = PLT_ARMO | PLT_HARMOR;
        break;
    case ITYPE_STAFF:
        flgs = PLT_STAFF | PLT_CHRG;
        break;
    case ITYPE_GOLD:
        break;
    case ITYPE_RING:
    case ITYPE_AMULET:
        flgs = PLT_JEWEL;
        break;
    }
    return flgs;
}

float ItemDropChance(int wIndex, int sn, int lvl, int numPlayers, bool uniqueMonster)
{
    int quality = CFDQ_UNIQUE;
    int mpl = 1;
    int dvs = 1;
    if (!uniqueMonster) {
        dvs *= 128;
        mpl *= 47 + numPlayers * 4;

        quality = CFDQ_NORMAL;
    }

    bool (*func)(const ItemData& item, void* arg);
    void* arg = NULL;
    if (quality >= CFDQ_GOOD) {
        func = &RndUItemOk;
    } else {
        if (wIndex == IDI_GOLD) {
            mpl *= 128 - 33;
            dvs *= 128;
            return (float)mpl / (float)dvs;
        }
        mpl *= 33;
        dvs *= 128;
        func = &RndItemOk;
    }

    int i, ri;
    int ril[NUM_IDI - IDI_RNDDROP_FIRST];

    for (i = IDI_RNDDROP_FIRST; i < (IsHellfireGame ? NUM_IDI : NUM_IDI_DIABLO); i++) {
        ril[i - IDI_RNDDROP_FIRST] = (!func(AllItemList[i], arg) || lvl < AllItemList[i].iMinMLvl) ? 0 : AllItemList[i].iRnd;
    }
    ri = 0;
    for (i = 0; i < ((IsHellfireGame ? NUM_IDI : NUM_IDI_DIABLO) - IDI_RNDDROP_FIRST); i++)
        ri += ril[i];

    if (ri == 0) {
        return 0;
    }
    dvs *= ri;
    if (wIndex < NUM_IDI) {
        mpl *= (wIndex >= IDI_RNDDROP_FIRST && (wIndex - IDI_RNDDROP_FIRST) < ((IsHellfireGame ? NUM_IDI : NUM_IDI_DIABLO) - IDI_RNDDROP_FIRST)) ? ril[wIndex - IDI_RNDDROP_FIRST] : 0;
    } else {
        int n;
        switch (wIndex) {
        case NUM_IDI + 0: wIndex = IDI_BOOK1;   n = 4; break;
        case NUM_IDI + 1: wIndex = IDI_SCROLL1; n = 7; break;
        case NUM_IDI + 2: wIndex = IDI_RUNE1;   n = 7; break;
        case NUM_IDI + 3: wIndex = IDI_RING1;   n = 5; break;
        case NUM_IDI + 4: wIndex = IDI_AMULET1; n = 3; break;
        }
        ri = 0;
        for (i = wIndex; i < wIndex + n; i++) {
            ri += ril[i - IDI_RNDDROP_FIRST];
        }
        mpl *= ri;
    }

    if (sn != SPL_NULL) {
        switch (AllItemList[wIndex].iMiscId) {
        case IMISC_BOOK:   dvs *= GetBookSpell(lvl, -2);   break;
        case IMISC_SCROLL: dvs *= GetScrollSpell(lvl, -2); break;
        case IMISC_RUNE:   dvs *= GetRuneSpell(lvl, -2);   break;
        case IMISC_NONE:   dvs *= GetStaffSpell(lvl, -2);  break;
        }
        if (dvs == 0)
            return 0;
    }

    return (float)mpl / (float)dvs;
}

DEVILUTION_END_NAMESPACE
