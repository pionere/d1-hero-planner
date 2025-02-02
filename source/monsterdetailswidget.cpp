#include "monsterdetailswidget.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QWidgetAction>

#include "d1hro.h"
#include "mainwindow.h"
#include "sidepanelwidget.h"
#include "ui_monsterdetailswidget.h"

#include "dungeon/all.h"

Q_DECLARE_METATYPE(dungeon_type)

MonsterDetailsWidget::MonsterDetailsWidget(SidePanelWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MonsterDetailsWidget())
{
    ui->setupUi(this);

    QObject::connect(this->ui->dunLevelBonusEdit, SIGNAL(cancel_signal()), this, SLOT(on_dunLevelBonusEdit_escPressed()));
}

MonsterDetailsWidget::~MonsterDetailsWidget()
{
    delete ui;
}

void MonsterDetailsWidget::initialize(D1Hero *h)
{
    this->hero = h;
    this->ui->heroSkillsComboBox->setHero(h);

    this->updateFields();

    this->setVisible(true);
}

void MonsterDetailsWidget::displayFrame()
{
    this->updateFields();
}

static QString MonLeaderText(BYTE flag, BYTE packsize)
{
    QString result;
    switch (flag) {
    case MLEADER_NONE:
        result = "-";
        break;
    case MLEADER_PRESENT:
        result = QApplication::tr("Minion");
        break;
    case MLEADER_AWAY:
        result = QApplication::tr("Lost");
        break;
    case MLEADER_SELF:
        // result = QApplication::tr("Leader(%1)").arg(packsize);
        result = QApplication::tr("Leader");
        break;
    default:
        result = "???";
        break;
    }
    return result;
}

static void MonResistText(unsigned resist, unsigned idx, QProgressBar *label)
{
    int value = 0;
    QString tooltip;
    switch ((resist >> idx) & 3) {
    case MORT_NONE:
        value = 0;
        tooltip = QApplication::tr("Vulnerable to %1 damage");
        break;
    case MORT_PROTECTED:
        value = 50 - 50 / 4;
        tooltip = QApplication::tr("Protected against %1 damage");
        break;
    case MORT_RESIST:
        value = 75;
        tooltip = QApplication::tr("Resists %1 damage");
        break;
    case MORT_IMMUNE:
        value = 100;
        tooltip = QApplication::tr("Immune to %1 damage");
        break;
    default:
        tooltip = QApplication::tr( "???");
        break;
    }
    QString type;
    switch (idx) {
    case MORS_IDX_SLASH:     type = QApplication::tr("slash");     break;
    case MORS_IDX_BLUNT:     type = QApplication::tr("blunt");     break;
    case MORS_IDX_PUNCTURE:  type = QApplication::tr("puncture");  break;
    case MORS_IDX_FIRE:      type = QApplication::tr("fire");      break;
    case MORS_IDX_LIGHTNING: type = QApplication::tr("lightning"); break;
    case MORS_IDX_MAGIC:     type = QApplication::tr("magic");     break;
    case MORS_IDX_ACID:      type = QApplication::tr("acid");      break;
    }
    label->setValue(value);
    label->setToolTip(tooltip.arg(type));
}

static void displayDamage(QLabel *label, int minDam, int maxDam)
{
    if (maxDam != 0) {
        if (minDam != maxDam)
            label->setText(QString("%1 - %2").arg(minDam).arg(maxDam));
        else
            label->setText(QString("%1").arg(minDam));
    } else {
        label->setText(QString("-"));
    }
}

static std::pair<RANGE, RANGE> monLevelRange(int mtype, int dtype)
{
    std::pair<RANGE, RANGE> result = std::pair<RANGE, RANGE>({ DLV_INVALID, DLV_INVALID }, { DLV_INVALID, DLV_INVALID });
    //if (dtype != DTYPE_TOWN) {
        for (int n = 0; n < NUM_STDLVLS; n++) {
            if (!IsHellfireGame && AllLevels[n].dType > DTYPE_HELL) continue;
            if (dtype != DTYPE_TOWN && AllLevels[n].dType != dtype) continue;
            for (int m = 0; AllLevels[n].dMonTypes[m] != MT_INVALID; m++) {
                if (AllLevels[n].dMonTypes[m] == mtype) {
                    if (result.first.from == DLV_INVALID)
                        result.first.from = n;
                    result.first.to = n;
                    break;
                }
            }
            if ((n == questlist[Q_WARLORD]._qdlvl && (mtype == MT_BBLACK))
             || (n == questlist[Q_BETRAYER]._qdlvl && (mtype == MT_RSUCC || mtype == MT_BMAGE))
             || (n == questlist[Q_BLOOD]._qdlvl && (mtype == MT_NRHINO))
             || (n == questlist[Q_ANVIL]._qdlvl && (mtype == MT_DRHINO || mtype == MT_GGOATBW))
             || (n == questlist[Q_BANNER]._qdlvl && (mtype == MT_NFAT || mtype == MT_BFALLSD))
             || (n == questlist[Q_BLIND]._qdlvl && (mtype == MT_YSNEAK))
             || (n == DLV_HELL4 && (mtype == MT_GBLACK || mtype == MT_BMAGE || mtype == MT_NBLACK || mtype == MT_DIABLO))
             || (n == DLV_NEST2 && (mtype == MT_HORKSPWN))
             || (n == DLV_NEST3 && (mtype == MT_HORKSPWN || mtype == MT_HORKDMN))
             || (n == DLV_NEST4 && (mtype == MT_DEFILER))
             || (n == DLV_CRYPT4 && (mtype == MT_ARCHLICH || mtype == MT_NAKRUL))) {
                if (result.first.from == DLV_INVALID)
                    result.first.from = n;
                result.first.to = n;
            }
        }
        for (int n = NUM_STDLVLS; n < NUM_SETLVLS; n++) {
            if (!IsHellfireGame && AllLevels[n].dType > DTYPE_HELL) continue;
            if (dtype != DTYPE_TOWN && AllLevels[n].dType != dtype) continue;
            for (int m = 0; AllLevels[n].dMonTypes[m] != MT_INVALID; m++) {
                if (AllLevels[n].dMonTypes[m] == mtype) {
                    if (result.second.from == DLV_INVALID)
                        result.second.from = n;
                    result.second.to = n;
                    break;
                }
            }
            if ((n == questlist[Q_PWATER]._qslvl && (mtype == MT_NGOATBW || mtype == MT_DFALLSP || mtype == MT_YFALLSD || mtype == MT_NGOATMC))
             || (n == questlist[Q_SKELKING]._qslvl && (mtype == MT_TSKELBW || mtype == MT_RSKELBW || mtype == MT_XSKELBW || mtype == MT_RSKELSD || mtype == MT_RSKELAX))
             || (n == questlist[Q_BETRAYER]._qslvl && (mtype == MT_RSUCC || mtype == MT_BMAGE))
             || (n == questlist[Q_BCHAMB]._qslvl && (mtype == MT_XSKELSD || mtype == MT_BSNEAK || mtype == MT_NRHINO))) {
                if (result.second.from == DLV_INVALID)
                    result.second.from = n;
                result.second.to = n;
            }
        }
    //} else {
    //    result = { 0, NUM_FIXLVLS - 1};
    // }
    return result;
}

static std::pair<int, int> uniqMonLevel(int uniqindex, int dtype)
{
    const UniqMonData &mon = uniqMonData[uniqindex];
    std::pair<int, int> result = std::pair<int, int>(DLV_INVALID, DLV_INVALID);
    int ml = mon.muLevelIdx;
    if (ml != 0) {
        if (ml < NUM_STDLVLS)
            result.first = ml;
        else
            result.second = ml;
    }
    switch (uniqindex) {
    case UMT_GARBUD:                                                                break;
    case UMT_SKELKING:   result.second = SL_SKELKING;                               break;
    case UMT_ZHAR:                                                                  break;
    case UMT_SNOTSPIL:   result.first = DLV_CATHEDRAL4;                             break;
    case UMT_LAZARUS:    result.first = DLV_HELL3; result.second = SL_VILEBETRAYER; break;
    case UMT_RED_VEX:    result.first = DLV_HELL3; result.second = SL_VILEBETRAYER; break;
    case UMT_BLACKJADE:  result.first = DLV_HELL3; result.second = SL_VILEBETRAYER; break;
    case UMT_LACHDAN:                                                               break;
    case UMT_WARLORD:    result.first = DLV_HELL1;                                  break;
    case UMT_BUTCHER:    result.first = DLV_CATHEDRAL2;                             break;
    case UMT_DIABLO:     result.first = DLV_HELL4;                                  break;
    case UMT_ZAMPHIR:                                                               break;
#ifdef HELLFIRE
    case UMT_HORKDMN:                                                               break;
    case UMT_DEFILER:                                                               break;
    case UMT_NAKRUL:     result.first = DLV_CRYPT4;                                 break;
#endif
    }
    // assert(result.first != DLV_INVALID || result.second != DLV_INVALID);
    if (dtype != DTYPE_TOWN) {
        if (result.first != DLV_INVALID && AllLevels[result.first].dType != dtype)
            result.first = DLV_INVALID;
        if (result.second != DLV_INVALID && AllLevels[result.second].dType != dtype)
            result.second = DLV_INVALID;
    }
    if (!IsHellfireGame) {
        if (result.first != DLV_INVALID && AllLevels[result.first].dType > DTYPE_HELL)
            result.first = DLV_INVALID;
        if (result.second != DLV_INVALID && AllLevels[result.second].dType > DTYPE_HELL)
            result.second = DLV_INVALID;
    }
    return result;
}

typedef struct MonsterDamage {
    bool hth;
    bool mis;

    int minHth;
    int maxHth;
    int chanceHth;
    BYTE resHth;
    bool blockHth;

    int minMis;
    int maxMis;
    int chanceMis;
    BYTE resMis;
    bool blockMis;

} MonsterDamage;

static int GetMinMaxDamage(int damage, const D1Hero *hero, bool phys)
{
    damage += hero->getGetHit();
    if (phys)
        damage += hero->getGetPhysHit();
    if (damage < 1)
        damage = 1;
    return damage;
}

static MonsterDamage GetMonsterDamage(const MonsterStruct *mon, int dist, const D1Hero *hero)
{
    bool hth = false;
    bool special = false; // hit2/dam2 + afnum2
    bool charge = false;  // (hit * 8)/dam2
    bool flash = false;
    int mtype = -1;
    bool ranged_special; // aiParam1 + afnum2
    switch (mon->_mAI.aiType) {
    case AI_LACHDAN:
        break;
    case AI_RHINO: // + charge
    case AI_SNAKE: // + charge
        hth = true;
        charge = true;
        special = true;
        break;
    case AI_ZOMBIE:
    case AI_SKELSD:
    case AI_SCAV:
    case AI_FALLEN:
    case AI_SKELKING:
    case AI_BAT:   // + MIS_LIGHTNING if MT_XBAT
    case AI_CLEAVER:
    case AI_SNEAK:
    //case AI_FIREMAN: // MIS_KRULL
    case AI_GOLUM:
    case AI_SNOTSPIL:
    case AI_WARLORD:
#ifdef HELLFIRE
    case AI_HORKDMN:
#endif
        hth = true;
        break;  // hth  --  MOFILE_MAGMA (hit + 10, dam - 2), MOFILE_THIN (hit - 20, dam + 4)
    case AI_FAT:
        hth = true;
        special = true;
        break;
    case AI_ROUND: // + special if aiParam1
    case AI_GARG:  // + AI_ROUND
    case AI_GARBUD: // + AI_ROUND
        hth = true;
        special = mon->_mAI.aiParam1;
        break;
    case AI_SKELBOW:
        mtype = MIS_ARROW;
        break;
    case AI_RANGED: // special ranged / ranged if aiParam2
    case AI_LAZHELP: // AI_RANGED
        ranged_special = mon->_mAI.aiParam2;
        mtype = mon->_mAI.aiParam1;
        break;
    case AI_ROUNDRANGED: // special ranged / hth
    case AI_ROUNDRANGED2: // AI_ROUNDRANGED
        hth = true;
        ranged_special = true;
        mtype = mon->_mAI.aiParam1;
        break;
    case AI_ZHAR: // AI_COUNSLR
    case AI_COUNSLR: // ranged + MIS_FLASH
    case AI_LAZARUS: // AI_COUNSLR
        mtype = mon->_mAI.aiParam1; // + MIS_FLASH
        flash = true;
        break;
    case AI_MAGE:
        mtype = MIS_MAGE; // + MIS_FLASH + param1
        flash = true;
        break;
    }

    MonsterDamage result = { 0 };
    static_assert((int)MISR_NONE == 0, "GetMonsterDamage must initialize resHth and resMis");
    if (hth) {
        result.hth = true;
        result.blockHth = true;
        // result.resHth = MISR_NONE;
        result.minHth = GetMinMaxDamage(mon->_mMinDamage, hero, true);
        result.maxHth = GetMinMaxDamage(mon->_mMaxDamage, hero, true);
        result.chanceHth = 30 + mon->_mHit + (2 * mon->_mLevel) - hero->getAC();
    }
    if (flash) {
        result.hth = true;
        result.resHth = GetMissileElement(MIS_FLASH);
        result.blockHth = !(missiledata[MIS_FLASH].mdFlags & MIF_NOBLOCK);
        int mindam, maxdam;
        GetMissileDamage(MIS_FLASH, mon, &mindam, &maxdam);
        mindam = hero->calcPlrDam(result.resHth, mindam);
        maxdam = hero->calcPlrDam(result.resHth, maxdam);
        if (maxdam != 0) {
            if (!(missiledata[MIS_FLASH].mdFlags & MIF_DOT)) {
                // assert(!(missiledata[MIS_FLASH].mdFlags & MIF_ARROW));
                mindam = GetMinMaxDamage(mindam, hero, false);
                maxdam = GetMinMaxDamage(maxdam, hero, false);
            }
        }
        result.minHth = mindam;
        result.maxHth = maxdam;
        result.chanceHth = MissPlrHitByMonChance(MIS_FLASH, dist, mon, hero);
    }
    if (special) {
        result.mis = true;
        // result.resMis = MISR_NONE;
        result.blockMis = true;
        result.minMis = GetMinMaxDamage(mon->_mMinDamage2, hero, false);
        result.maxMis = GetMinMaxDamage(mon->_mMaxDamage2, hero, false);
        result.chanceMis = 30 + (2 * mon->_mLevel) - hero->getAC();
        result.chanceMis += charge ? mon->_mHit * 8 : mon->_mHit2;
    }
    if (mtype != -1 && mtype != MIS_SWAMPC) {
        result.mis = true;
        mtype = GetBaseMissile(mtype);
        result.resMis = GetMissileElement(mtype);
        result.blockMis = !(missiledata[mtype].mdFlags & MIF_NOBLOCK);
        int mindam, maxdam;
        GetMissileDamage(mtype, mon, &mindam, &maxdam);
        mindam = hero->calcPlrDam(result.resMis, mindam);
        maxdam = hero->calcPlrDam(result.resMis, maxdam);
        if (maxdam != 0) {
            if (!(missiledata[mtype].mdFlags & MIF_DOT)) {
                mindam = GetMinMaxDamage(mindam, hero, (missiledata[mtype].mdFlags & MIF_ARROW) != 0);
                maxdam = GetMinMaxDamage(maxdam, hero, (missiledata[mtype].mdFlags & MIF_ARROW) != 0);
            }
        }
        result.minMis = mindam;
        result.maxMis = maxdam;
        result.chanceMis = MissPlrHitByMonChance(mtype, dist, mon, hero);
    }

    return result;
}

typedef struct PlayerDamage {
    bool hth;
    bool mis;

    int minHth;
    int maxHth;
    int chanceHth;

    int minMis;
    int maxMis;
    int chanceMis;
    BYTE resMis;
    bool blockMis;

} PlayerDamage;

static PlayerDamage GetPlayerDamage(const D1Hero *hero, int sn, int dist, const MonsterStruct *mon)
{
    int sl = hero->getSkillLvl(sn);
    bool mis = spelldata[sn].sType != STYPE_NONE || (spelldata[sn].sUseFlags & SFLAG_RANGED);
    bool hth = !mis;
    PlayerDamage result = { 0 };
    static_assert((int)MISR_NONE == 0, "GetPlayerDamage must initialize resMis");
    if (mis) {
        result.mis = true;
        int mtype = spelldata[sn].sMissile;
        mtype = GetBaseMissile(mtype);
        result.resMis = GetMissileElement(mtype);
        result.blockMis = !(missiledata[mtype].mdFlags & MIF_NOBLOCK);
        int mindam, maxdam;
        hero->getMonSkillDamage(sn, sl, dist, mon, &mindam, &maxdam);
        result.minMis = mindam;
        result.maxMis = maxdam;
        result.chanceMis = MissMonHitByPlrChance(mtype, dist, hero, mon);
        if (sn == SPL_CHARGE)
            result.chanceMis = sl * 16 - mon->_mArmorClass;
    }

    if (hth) {
        result.hth = true;
        result.chanceHth = hero->getHitChance() - mon->_mArmorClass;
        if (sn == SPL_SWIPE) {
            result.chanceHth -= 30 - sl * 2;
        }
        int mindam, maxdam;
        hero->getMonDamage(sn, sl, mon, &mindam, &maxdam);
        result.minHth = mindam;
        result.maxHth = maxdam;
    }
    return result;
}

void MonsterDetailsWidget::updateFields()
{
    // QMessageBox::critical(nullptr, "Error", QApplication::tr("MonsterDetailsWidget::updateFields meteor%1").arg(this->hero->getSkillLvl(SPL_METEOR)));
    int mi;
    // update dun-type combobox
    QComboBox *dunComboBox = this->ui->dunTypeComboBox;
    mi = dunComboBox->currentData().value<int>();
    if (IsHellfireGame) {
        if (dunComboBox->count() == DTYPE_HELL + 1) {
            dunComboBox->addItem(tr("(L5) Crypt"), QVariant::fromValue(DTYPE_CRYPT));
            dunComboBox->addItem(tr("(L6) Nest"), QVariant::fromValue(DTYPE_NEST));
        }
    } else {
        while (dunComboBox->count() > DTYPE_HELL + 1) {
            dunComboBox->removeItem(DTYPE_HELL + 1);
        }
    }
    mi = dunComboBox->findData(mi);
    if (mi < 0) mi = 0;
    dunComboBox->setCurrentIndex(mi);

    QComboBox *typesComboBox = this->ui->monTypeComboBox;
    mi = typesComboBox->currentData().value<int>();
    typesComboBox->clear();
    
    int dtype = this->ui->dunTypeComboBox->currentIndex();
    for (int i = 0; i < NUM_MTYPES; i++) {
        const std::pair<RANGE, RANGE> ranges = monLevelRange(i, dtype);
        if (ranges.first.from == DLV_INVALID && ranges.second.from == DLV_INVALID)
            continue;
        /*if (loc != DTYPE_TOWN) {
            int n;
            for (n = 0; n < NUM_FIXLVLS; n++) {
                if (AllLevels[n].dType != loc) continue;
                int m;
                for (m = 0; AllLevels[n].dMonTypes[m] != MT_INVALID; m++) {
                    if (AllLevels[n].dMonTypes[m] == i) break;
                }
                if (AllLevels[n].dMonTypes[m] != MT_INVALID) break;
            }
            if (n >= NUM_FIXLVLS) continue;
        }*/
        typesComboBox->addItem(monsterdata[i].mName, QVariant::fromValue(i + 1));
    }
    for (int i = 0; ; i++) {
        const UniqMonData &mon = uniqMonData[i];
        if (mon.mtype == MT_INVALID)
            break;
        const std::pair<int, int> ml = uniqMonLevel(i, dtype);
        if (ml.first == DLV_INVALID && ml.second == DLV_INVALID) continue;
        typesComboBox->addItem(QString("**%1**").arg(mon.mName), QVariant::fromValue(-(i + 1)));
    }
    // typesComboBox->adjustSize();
    mi = typesComboBox->findData(mi);
    if (mi < 0) mi = 0;
    typesComboBox->setCurrentIndex(mi);

    int type = typesComboBox->currentData().value<int>();
    bool unique = type < 0;
    bool minion;
    if (unique) {
        type = -(type + 1);
        const std::pair<int, int> ml = uniqMonLevel(type, dtype);
        QString tooltip;
        if (ml.first != DLV_INVALID)
            tooltip = tr("Dungeon Level %1 (%2)").arg(AllLevels[ml.first].dLevelName).arg(ml.first);
        if (ml.second != DLV_INVALID) {
            if (ml.first != DLV_INVALID)
                tooltip += " ";
            tooltip += tr("Set Level %1 (%2)").arg(AllLevels[ml.second].dLevelName).arg(ml.second);
        }
        typesComboBox->setToolTip(tooltip);
        minion = (uniqMonData[type].mUnqFlags & UMF_GROUP) != 0;
        this->ui->minionCheckBox->setVisible(minion);
        minion &= this->ui->minionCheckBox->isChecked();
    } else {
        type = type - 1;
        const std::pair<RANGE, RANGE> ranges = monLevelRange(type, dtype);
        QString tooltip;
        if (ranges.first.from != DLV_INVALID) {
            if (ranges.first.from == ranges.first.to)
                tooltip = tr("Dungeon Level %1 (%2)").arg(AllLevels[ranges.first.from].dLevelName).arg(ranges.first.from);
            else
                tooltip = tr("Dungeon Level %1-%2").arg(ranges.first.from).arg(ranges.first.to);
        }
        if (ranges.second.from != DLV_INVALID) {
            if (ranges.first.from != DLV_INVALID)
                tooltip += " ";
            if (ranges.second.from == ranges.second.to)
                tooltip += tr("Set Level %1 (%2)").arg(AllLevels[ranges.second.from].dLevelName).arg(ranges.second.from);
            else
                tooltip += tr("Set Level %1-%2").arg(ranges.second.from).arg(ranges.second.to);
        }
        typesComboBox->setToolTip(tooltip);
        this->ui->minionCheckBox->setVisible(false);
    }
    bool multi = this->hero->isMulti();
    this->ui->plrCountSpinBox->setEnabled(multi);
    if (!multi) {
        this->ui->plrCountSpinBox->changeValue(1);
    }
    int numplrs = this->ui->plrCountSpinBox->value();
    int lvlbonus = this->dunLevelBonus;
    this->ui->dunLevelBonusEdit->setText(QString::number(lvlbonus));
    bool lvlrel = this->ui->dunLevelBonusCheckBox->isChecked();
    this->ui->dunLevelBonusCheckBox->setToolTip(lvlrel ? tr("Level Bonus is relative") : tr("Level Bonus is absolute"));

    if (unique) {
        if (lvlrel)
            lvlbonus -= uniqMonData[type].muLevel;
        InitUniqMonster(type, numplrs, lvlbonus, minion);
    } else {
        if (lvlbonus != 0 && false) {
        typedef struct MonHealth {
            const char* name;
            int type;
            int minValue;
            int maxValue;
            int frames;
            int toHit;
        } MonHealth;
#define OFFENSIVE 0
        typedef struct MonDamage {
            const char* name;
            int type;
            int minValue;
            int maxValue;
            int frames;
            int toHit;
        } MonDamage;
#if OFFENSIVE
        typedef MonDamage EntryType;
#else
        typedef MonHealth EntryType;
#endif
        std::map<int, std::vector<EntryType>> normalMaxMons, uniqMaxMons;
        std::map<int, std::vector<EntryType>> normalAvgMons, uniqAvgMons;

        std::map<int, std::vector<EntryType>> normalMaxRangedMons, uniqMaxRangedMons;
        std::map<int, std::vector<EntryType>> normalAvgRangedMons, uniqAvgRangedMons;

        std::map<int, std::vector<EntryType>> normalMaxFireMons, uniqMaxFireMons;
        std::map<int, std::vector<EntryType>> normalAvgFireMons, uniqAvgFireMons;
        std::map<int, std::vector<EntryType>> normalMaxLghtMons, uniqMaxLghtMons;
        std::map<int, std::vector<EntryType>> normalAvgLghtMons, uniqAvgLghtMons;
        std::map<int, std::vector<EntryType>> normalMaxMagicMons, uniqMaxMagicMons;
        std::map<int, std::vector<EntryType>> normalAvgMagicMons, uniqAvgMagicMons;
        std::map<int, std::vector<EntryType>> normalMaxAcidMons, uniqMaxAcidMons;
        std::map<int, std::vector<EntryType>> normalAvgAcidMons, uniqAvgAcidMons;
        for (int n = 0; n < typesComboBox->count(); n++) {
            int tt = typesComboBox->itemData(n).value<int>();
            int lb = lvlbonus;
            if (tt < 0) {
                tt = -(tt + 1);
                if (lvlrel)
                    lb -= uniqMonData[tt].muLevel;
                InitUniqMonster(tt, numplrs, lb, false);

                MonsterStruct *mon = &monsters[MAX_MINIONS];
                if (mon->_mMaxDamage == 0)
                    continue;
                std::map<int, std::vector<EntryType>>* uniqMaxMonsMagic;
                std::map<int, std::vector<EntryType>>* uniqAvgMonsMagic;
                uniqMaxMonsMagic = &uniqMaxMons;
                uniqAvgMonsMagic = &uniqAvgMons;
                if (mon->_mAI.aiType == AI_MAGE || mon->_mAI.aiType == AI_COUNSLR
                 || mon->_mAI.aiType == AI_LAZHELP || mon->_mAI.aiType == AI_LAZARUS || mon->_mAI.aiType == AI_ZHAR
                 || (mon->_mAI.aiType == AI_ROUNDRANGED && mon->_mAI.aiParam1 != MIS_ARROW)
                 || (mon->_mAI.aiType == AI_ROUNDRANGED2 && mon->_mAI.aiParam1 != MIS_ARROW)
                 || (mon->_mAI.aiType == AI_RANGED && mon->_mAI.aiParam1 != MIS_ARROW)) {
#if OFFENSIVE
                    int mres = missiledata[mon->_mAI.aiType == AI_MAGE ? MIS_MAGE : mon->_mAI.aiParam1].mResist;
                    switch (mon->_mAI.aiParam1) {
                    case MIS_LIGHTNINGC:
                    case MIS_LIGHTNINGC2:
                    case MIS_CBOLTC: mres = MISR_LIGHTNING; break;
                    case MIS_INFERNOC: mres = MISR_FIRE; break;
                    }
                    switch (mres) {
                    case MISR_LIGHTNING:
                        uniqMaxMonsMagic = &uniqMaxLghtMons;
                        uniqAvgMonsMagic = &uniqAvgLghtMons;
                        break;
                    case MISR_ACID:
                        uniqMaxMonsMagic = &uniqMaxAcidMons;
                        uniqAvgMonsMagic = &uniqAvgAcidMons;
                        break;
                    case MISR_FIRE:
                        uniqMaxMonsMagic = &uniqMaxFireMons;
                        uniqAvgMonsMagic = &uniqAvgFireMons;
                        break;
                    case MISR_MAGIC:
                        uniqMaxMonsMagic = &uniqMaxMagicMons;
                        uniqAvgMonsMagic = &uniqAvgMagicMons;
                        break;
                    default:
                        if (mon->_mAI.aiParam1 == MIS_APOCAC2) {
                            LogErrorF("Apoc damage %d - %d (%d) ar:%d", mon->_mMinDamage, mon->_mMaxDamage, (mon->_mMinDamage + mon->_mMaxDamage) / 2, mon->_mHit);
                        } else
                            LogErrorF("Unknown missile damage type:%d res%d", mon->_mAI.aiParam1, mres);
                        continue;
                    }
#else
                    uniqMaxMonsMagic = &uniqMaxRangedMons;
                    uniqAvgMonsMagic = &uniqAvgRangedMons;
#endif
                } else if (mon->_mAI.aiType == AI_SKELBOW
                        || mon->_mAI.aiType == AI_ROUNDRANGED
                        || mon->_mAI.aiType == AI_ROUNDRANGED2
                        || mon->_mAI.aiType == AI_RANGED) {
                    uniqMaxMonsMagic = &uniqMaxRangedMons;
                    uniqAvgMonsMagic = &uniqAvgRangedMons;
                }
#if OFFENSIVE
                MonDamage monDmg = { mon->_mName, tt, mon->_mMinDamage, mon->_mMaxDamage, monfiledata[monsterdata[mapMonTypes[mon->_mMTidx].cmType].moFileNum].moAnimFrames[MA_ATTACK], mon->_mHit };
                (*uniqMaxMonsMagic)[mon->_mMaxDamage].push_back(monDmg);
                int avgDam = (mon->_mMinDamage + mon->_mMaxDamage) / 2;
                (*uniqAvgMonsMagic)[avgDam].push_back(monDmg);
#else
                int minHp = mon->_mmaxhp; // mapMonTypes[mon->_mMTidx].cmMinHP;
                int maxHp = minHp; // mapMonTypes[mon->_mMTidx].cmMaxHP;
                MonHealth monHp = { mon->_mName, tt, minHp, maxHp, monfiledata[monsterdata[mapMonTypes[mon->_mMTidx].cmType].moFileNum].moAnimFrames[MA_ATTACK], mon->_mArmorClass };
                (*uniqMaxMonsMagic)[maxHp].push_back(monHp);
                int avgHp = (minHp + maxHp) / 2;
                (*uniqAvgMonsMagic)[avgHp].push_back(monHp);
#endif
            } else {
                tt = tt - 1;
                if (lvlrel)
                    lb -= monsterdata[tt].mLevel;
                InitLvlMonster(tt, numplrs, lb);

                MonsterStruct *mon = &monsters[MAX_MINIONS];
                std::map<int, std::vector<EntryType>> *normalMaxMonsMagic;
                std::map<int, std::vector<EntryType>> *normalAvgMonsMagic;
                normalMaxMonsMagic = &normalMaxMons;
                normalAvgMonsMagic = &normalAvgMons;
                if (mon->_mAI.aiType == AI_MAGE || mon->_mAI.aiType == AI_COUNSLR
                 || (mon->_mAI.aiType == AI_ROUNDRANGED && mon->_mAI.aiParam1 != MIS_ARROW)
                 || (mon->_mAI.aiType == AI_ROUNDRANGED2 && mon->_mAI.aiParam1 != MIS_ARROW)
                 || (mon->_mAI.aiType == AI_RANGED && mon->_mAI.aiParam1 != MIS_ARROW)) {

#if OFFENSIVE
                    int mres = missiledata[mon->_mAI.aiType == AI_MAGE ? MIS_MAGE : mon->_mAI.aiParam1].mResist;
                    switch (mon->_mAI.aiParam1) {
                    case MIS_LIGHTNINGC:
                    case MIS_LIGHTNINGC2:
                    case MIS_CBOLTC: mres = MISR_LIGHTNING; break;
                    case MIS_INFERNOC: mres = MISR_FIRE; break;
                    }
                    switch (mres) {
                    case MISR_LIGHTNING:
                        normalMaxMonsMagic = &normalMaxLghtMons;
                        normalAvgMonsMagic = &normalAvgLghtMons;
                        break;
                    case MISR_ACID:
                        normalMaxMonsMagic = &normalMaxAcidMons;
                        normalAvgMonsMagic = &normalAvgAcidMons;
                        break;
                    case MISR_FIRE:
                        normalMaxMonsMagic = &normalMaxFireMons;
                        normalAvgMonsMagic = &normalAvgFireMons;
                        break;
                    case MISR_MAGIC:
                        normalMaxMonsMagic = &normalMaxMagicMons;
                        normalAvgMonsMagic = &normalAvgMagicMons;
                        break;
                    default:
                        if (mon->_mAI.aiParam1 == MIS_APOCAC2) {
                            LogErrorF("Apoc damage %d - %d (%d) ar: %d", mon->_mMinDamage, mon->_mMaxDamage, (mon->_mMinDamage + mon->_mMaxDamage) / 2, mon->_mHit);
                        } else
                            LogErrorF("Unknown missile damage type:%d res%d", mon->_mAI.aiParam1, mres);
                        continue;
                    }
#else
                    normalMaxMonsMagic = &normalMaxRangedMons;
                    normalAvgMonsMagic = &normalAvgRangedMons;
#endif
                } else if (mon->_mAI.aiType == AI_SKELBOW
                        || mon->_mAI.aiType == AI_ROUNDRANGED
                        || mon->_mAI.aiType == AI_ROUNDRANGED2
                        || mon->_mAI.aiType == AI_RANGED) {
                    normalMaxMonsMagic = &normalMaxRangedMons;
                    normalAvgMonsMagic = &normalAvgRangedMons;
                }
#if OFFENSIVE
                MonDamage monDmg = { mon->_mName, tt, mon->_mMinDamage, mon->_mMaxDamage, monfiledata[monsterdata[mapMonTypes[mon->_mMTidx].cmType].moFileNum].moAnimFrames[MA_ATTACK], mon->_mHit };
                (*normalMaxMonsMagic)[mon->_mMaxDamage].push_back(monDmg);
                int avgDam = (mon->_mMinDamage + mon->_mMaxDamage) / 2;
                (*normalAvgMonsMagic)[avgDam].push_back(monDmg);
#else
                int minHp = mon->_mhitpoints; //  mapMonTypes[mon->_mMTidx].cmMinHP;
                int maxHp = mon->_mmaxhp; // mapMonTypes[mon->_mMTidx].cmMaxHP;
                MonHealth monHp = { mon->_mName, tt, minHp, maxHp, monfiledata[monsterdata[mapMonTypes[mon->_mMTidx].cmType].moFileNum].moAnimFrames[MA_ATTACK], mon->_mArmorClass };
                (*normalMaxMonsMagic)[maxHp].push_back(monHp);
                int avgHp = (minHp + maxHp) / 2;
                (*normalAvgMonsMagic)[avgHp].push_back(monHp);
#endif
            }
        }
#if OFFENSIVE
        const char* attribute = "Damage";
#else
        const char* attribute = "Hp";
#endif
        { // physical
            LogErrorF("Normal Monsters (max):");
            for (auto it = normalMaxMons.begin(); it != normalMaxMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (avg):");
            for (auto it = normalAvgMons.begin(); it != normalAvgMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (max):");
            for (auto it = uniqMaxMons.begin(); it != uniqMaxMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (avg):");
            for (auto it = uniqAvgMons.begin(); it != uniqAvgMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }
        { // ranged
            LogErrorF("Normal Monsters (ranged-max):");
            for (auto it = normalMaxRangedMons.begin(); it != normalMaxRangedMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (ranged-avg):");
            for (auto it = normalAvgRangedMons.begin(); it != normalAvgRangedMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (ranged-max):");
            for (auto it = uniqMaxRangedMons.begin(); it != uniqMaxRangedMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (ranged-avg):");
            for (auto it = uniqAvgRangedMons.begin(); it != uniqAvgRangedMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }
        { // fire
            LogErrorF("Normal Monsters (fire-max):");
            for (auto it = normalMaxFireMons.begin(); it != normalMaxFireMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (fire-avg):");
            for (auto it = normalAvgFireMons.begin(); it != normalAvgFireMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (fire-max):");
            for (auto it = uniqMaxFireMons.begin(); it != uniqMaxFireMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (fire-avg):");
            for (auto it = uniqAvgFireMons.begin(); it != uniqAvgFireMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }
        { // ligthning
            LogErrorF("Normal Monsters (lght-max):");
            for (auto it = normalMaxLghtMons.begin(); it != normalMaxLghtMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (lght-avg):");
            for (auto it = normalAvgLghtMons.begin(); it != normalAvgLghtMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (lght-max):");
            for (auto it = uniqMaxLghtMons.begin(); it != uniqMaxLghtMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (lght-avg):");
            for (auto it = uniqAvgLghtMons.begin(); it != uniqAvgLghtMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }
        { // magic
            LogErrorF("Normal Monsters (magic-max):");
            for (auto it = normalMaxMagicMons.begin(); it != normalMaxMagicMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (magic-avg):");
            for (auto it = normalAvgMagicMons.begin(); it != normalAvgMagicMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (magic-max):");
            for (auto it = uniqMaxMagicMons.begin(); it != uniqMaxMagicMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (magic-avg):");
            for (auto it = uniqAvgMagicMons.begin(); it != uniqAvgMagicMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }
        { // acid
            LogErrorF("Normal Monsters (acid-max):");
            for (auto it = normalMaxAcidMons.begin(); it != normalMaxAcidMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Normal Monsters (acid-avg):");
            for (auto it = normalAvgAcidMons.begin(); it != normalAvgAcidMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (acid-max):");
            for (auto it = uniqMaxAcidMons.begin(); it != uniqMaxAcidMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d - %d (frames%d ar:%d)", vit->name, vit->type, attribute, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
            LogErrorF("Unique Monsters (acid-avg):");
            for (auto it = uniqAvgAcidMons.begin(); it != uniqAvgAcidMons.end(); it++) {
                for (auto vit = it->second.begin(); vit != it->second.end(); vit++) {
                    LogErrorF("Name: %s (%d) %s: %d (%d - %d, frames%d ar:%d)", vit->name, vit->type, attribute, it->first, vit->minValue, vit->maxValue, vit->frames, vit->toHit);
                }
            }
        }

        }
        if (lvlrel)
            lvlbonus -= monsterdata[type].mLevel;
        InitLvlMonster(type, numplrs, lvlbonus);
    }

    MonsterStruct *mon = &monsters[MAX_MINIONS];

    const char* color = "black";
    if (mon->_mNameColor == COL_BLUE)
        color = "blue";
    else if (mon->_mNameColor == COL_GOLD)
        color = "orange";
    this->ui->monsterName->setText(QString("<u><font color='%1'>%2</font></u>").arg(color).arg(mon->_mName));
    this->ui->monsterLevel->setText(tr("(Level %1)").arg(mon->_mLevel));
    this->ui->monsterExp->setText(QString::number(mon->_mExp));
    this->ui->monsterStatus->setText(MonLeaderText(mon->_mleaderflag, mon->_mpacksize));

    // MonsterAI _mAI;
    this->ui->monsterInt->setText(QString::number(mon->_mAI.aiInt));

    this->ui->monsterHit->setText(QString::number(mon->_mHit));
    displayDamage(this->ui->monsterDamage, mon->_mMinDamage, mon->_mMaxDamage);
    this->ui->monsterHit2->setText(QString::number(mon->_mHit2));
    displayDamage(this->ui->monsterDamage2, mon->_mMinDamage2, mon->_mMaxDamage2);
    this->ui->monsterMagic->setText(QString::number(mon->_mMagic));

    displayDamage(this->ui->monsterHp, mon->_mhitpoints, mon->_mmaxhp);
    this->ui->monsterArmorClass->setText(QString::number(mon->_mArmorClass));
    this->ui->monsterEvasion->setText(QString::number(mon->_mEvasion));

    unsigned res = mon->_mMagicRes;
    MonResistText(res, MORS_IDX_SLASH, this->ui->monsterResSlash);
    MonResistText(res, MORS_IDX_BLUNT, this->ui->monsterResBlunt);
    MonResistText(res, MORS_IDX_PUNCTURE, this->ui->monsterResPunct);
    MonResistText(res, MORS_IDX_FIRE, this->ui->monsterResFire);
    MonResistText(res, MORS_IDX_LIGHTNING, this->ui->monsterResLight);
    MonResistText(res, MORS_IDX_MAGIC, this->ui->monsterResMagic);
    MonResistText(res, MORS_IDX_ACID, this->ui->monsterResAcid);

    unsigned flags = mon->_mFlags;
    this->ui->monsterHiddenCheckBox->setChecked((flags & MFLAG_HIDDEN) != 0);
    this->ui->monsterGargCheckBox->setChecked((flags & MFLAG_GARG_STONE) != 0);
    this->ui->monsterStealCheckBox->setChecked((flags & MFLAG_LIFESTEAL) != 0);
    this->ui->monsterOpenCheckBox->setChecked((flags & MFLAG_CAN_OPEN_DOOR) != 0);
    this->ui->monsterSearchCheckBox->setChecked((flags & MFLAG_SEARCH) != 0);
    this->ui->monsterNoStoneCheckBox->setChecked((flags & MFLAG_NOSTONE) != 0);
    this->ui->monsterBleedCheckBox->setChecked((flags & MFLAG_CAN_BLEED) != 0);
    this->ui->monsterNoDropCheckBox->setChecked((flags & MFLAG_NODROP) != 0);
    this->ui->monsterKnockbackCheckBox->setChecked((flags & MFLAG_KNOCKBACK) != 0);

    // player vs. monster info
    // - update skill combobox
    /*QComboBox *skillsComboBox = this->ui->heroSkillsComboBox;
    mi = skillsComboBox->currentData().value<int>();
    // QMessageBox::critical(nullptr, "Error", QApplication::tr("Selected skill %1.").arg(mi));
    skillsComboBox->clear();
    for (int sn = 0; sn < (IsHellfireGame ? NUM_SPELLS : NUM_SPELLS_DIABLO); sn++) {
        if ((spelldata[sn].sUseFlags & this->hero->getSkillFlags()) != spelldata[sn].sUseFlags) continue;
        // if (sn != SPL_ATTACK) {
            if (!HasSkillDamage(sn)) continue;
            / *if (spelldata[sn].sBookLvl == SPELL_NA && spelldata[sn].sStaffLvl == SPELL_NA && !SPELL_RUNE(sn)) {
                continue;
            }* /
            if (this->hero->getSkillLvl(sn) == 0 && !(this->hero->getFixedSkills() & SPELL_MASK(sn)) && !SPELL_RUNE(sn)) {
                continue;
            }
        // }
        GetSkillName(sn);
        skillsComboBox->addItem(infostr, QVariant::fromValue(sn));
    }
    mi = skillsComboBox->findData(mi);
    // QMessageBox::critical(nullptr, "Error", QApplication::tr("Skill index %1.").arg(mi));
    if (mi < 0) mi = 0;
    skillsComboBox->setCurrentIndex(mi);*/

    int dist = this->ui->plrDistSpinBox->value();
    this->ui->plrDistSpinBox->setToolTip(tr("Distance to target in ticks. Charge distance to target: %1").arg(dist * this->hero->getChargeSpeed()));

    int hper, mindam, maxdam;
    int sn = this->ui->heroSkillsComboBox->update(); //  skillsComboBox->currentData().value<int>();
    // QMessageBox::critical(nullptr, "Error", QApplication::tr("Using skill meteor %1.").arg(this->hero->getSkillLvl(SPL_METEOR)));
    const PlayerDamage plrDam = GetPlayerDamage(this->hero, sn, dist, mon);
    if (plrDam.hth) {
        hper = plrDam.chanceHth;
        hper = CheckHit(hper);
        this->ui->plrHitChance->setText(QString("%1%").arg(hper));
    } else {
        this->ui->plrHitChance->setText(QString("-"));
    }
    this->ui->plrHitChanceSep->setVisible(plrDam.mis);
    this->ui->plrHitChance2->setVisible(plrDam.mis);
    if (plrDam.mis) {
        hper = plrDam.chanceMis;
        hper = CheckHit(hper);
        this->ui->plrHitChance2->setText(QString("%1%").arg(hper));
    }

    const MonsterDamage monDamage = GetMonsterDamage(mon, dist, this->hero);
    if (monDamage.hth) {
        hper = monDamage.chanceHth;
        hper = CheckHit(hper);
        this->ui->monHitChance->setText(QString("%1%").arg(hper));
    } else {
        this->ui->monHitChance->setText(QString("-"));
    }
    this->ui->monHitChanceSep->setVisible(monDamage.mis);
    this->ui->monHitChance2->setVisible(monDamage.mis);
    if (monDamage.mis) {
        hper = monDamage.chanceMis;
        hper = CheckHit(hper);
        this->ui->monHitChance2->setText(QString("%1%").arg(hper));
    }

    this->ui->plrDamageSep->setVisible(plrDam.mis);
    this->ui->plrDamage2->setVisible(plrDam.mis);
    if (plrDam.mis) {
        mindam = plrDam.minMis;
        maxdam = plrDam.maxMis;
        displayDamage(this->ui->plrDamage2, mindam, maxdam);
        this->ui->plrDamage2->setStyleSheet(GetElementColor(plrDam.resMis));
    }
    mindam = plrDam.minHth;
    maxdam = plrDam.maxHth;
    displayDamage(this->ui->plrDamage, mindam, maxdam);

    this->ui->monDamageSep->setVisible(monDamage.mis);
    this->ui->monDamage2->setVisible(monDamage.mis);
    if (monDamage.mis) {
        mindam = monDamage.minMis;
        maxdam = monDamage.maxMis;
        displayDamage(this->ui->monDamage2, mindam, maxdam);
        this->ui->monDamage2->setStyleSheet(GetElementColor(monDamage.resMis));
    }
    mindam = monDamage.minHth;
    maxdam = monDamage.maxHth;
    displayDamage(this->ui->monDamage, mindam, maxdam);
    this->ui->monDamage->setStyleSheet(GetElementColor(monDamage.resHth));

    hper = -1;
    if ((this->hero->getSkillFlags() & SFLAG_BLOCK) && ((monDamage.hth && monDamage.blockHth) || (monDamage.mis && monDamage.blockMis))) {
        hper = this->hero->getBlockChance();
        if (hper != 0) {
            hper -= 2 * mon->_mLevel;
            hper = CheckHit(hper);
        }
    }
    if (hper < 0) {
        this->ui->plrBlockChance->setText(QString("-"));
    } else if (monDamage.hth && monDamage.blockHth && monDamage.mis && !monDamage.blockMis) {
        this->ui->plrBlockChance->setText(QString("%1% | -").arg(hper));
    } else {
        this->ui->plrBlockChance->setText(QString("%1%").arg(hper));
    }
    hper = 0;
    this->ui->monBlockChance->setText(QString("%1%").arg(hper));

    // this->adjustSize(); // not sure why this is necessary...
}


void MonsterDetailsWidget::on_dunTypeComboBox_activated(int index)
{
    this->updateFields();
}

void MonsterDetailsWidget::on_dunLevelBonusCheckBox_clicked()
{
    this->updateFields();
}

void MonsterDetailsWidget::on_monTypeComboBox_activated(int index)
{
    this->updateFields();
}

void MonsterDetailsWidget::on_minionCheckBox_clicked()
{
    this->updateFields();
}

void MonsterDetailsWidget::on_dunLevelBonusEdit_returnPressed()
{
    this->dunLevelBonus = this->ui->dunLevelBonusEdit->text().toShort();

    this->on_dunLevelBonusEdit_escPressed();
}

void MonsterDetailsWidget::on_dunLevelBonusEdit_escPressed()
{
    // update dunLevelBonusEdit
    this->updateFields();
    this->ui->dunLevelBonusEdit->clearFocus();
}

void MonsterDetailsWidget::on_plrCountSpinBox_valueChanged(int value)
{
    this->updateFields();
}

void MonsterDetailsWidget::on_heroSkillsComboBox_activated(int index)
{
    // QMessageBox::critical(nullptr, "Error", QApplication::tr("heroSkillsComboBox new index %1.").arg(index));

    this->updateFields();
}

void MonsterDetailsWidget::on_plrDistSpinBox_valueChanged(int value)
{
    this->updateFields();
}

void MonsterDetailsWidget::on_closeButton_clicked()
{
    this->setVisible(false);
}
