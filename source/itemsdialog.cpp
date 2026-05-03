#include "itemsdialog.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
//#include <QToolTip>
#include <QWidgetAction>

//#include "d1hro.h"
#include "mainwindow.h"
#include "sidepanelwidget.h"
#include "ui_itemsdialog.h"

#include "dungeon/all.h"

#define AFFIX_ANY   -1
#define AFFIX_NONE  -2
#define AFFIX_SKILL  -3

Q_DECLARE_METATYPE(item_equip_type)
Q_DECLARE_METATYPE(item_type)

ItemsDialog::ItemsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ItemsDialog())
{
    ui->setupUi(this);

    QHBoxLayout *layout = this->ui->seedWithRefreshButtonLayout;
    PushButtonWidget::addButton(this, layout, QStyle::SP_BrowserReload, tr("Generate"), this, &ItemsDialog::on_actionGenerateSeed_triggered);
    layout->addStretch();

    this->itemProps = new ItemPropertiesWidget(this);
    this->ui->itemProperties->addWidget(this->itemProps);

    QObject::connect(this->ui->playersEdit, SIGNAL(cancel_signal()), this, SLOT(on_playersEdit_escPressed()));
    QObject::connect(this->ui->itemSeedEdit, SIGNAL(cancel_signal()), this, SLOT(on_itemSeedEdit_escPressed()));
    QObject::connect(this->ui->itemLevelEdit, SIGNAL(cancel_signal()), this, SLOT(on_itemLevelEdit_escPressed()));

    /*this->ui->itemTypeComboBox->view()->setTextElideMode(Qt::ElideNone);
    this->ui->itemLocComboBox->view()->setTextElideMode(Qt::ElideNone);
    this->ui->itemIdxComboBox->view()->setTextElideMode(Qt::ElideNone);

    this->ui->itemSourceComboBox->view()->setTextElideMode(Qt::ElideNone);
    this->ui->itemQualityComboBox->view()->setTextElideMode(Qt::ElideNone);

    this->ui->itemUniquesComboBox->view()->setTextElideMode(Qt::ElideNone);
    this->ui->itemPrefixComboBox->view()->setTextElideMode(Qt::ElideNone);
    this->ui->itemSuffixComboBox->view()->setTextElideMode(Qt::ElideNone);*/
}

ItemsDialog::~ItemsDialog()
{
    delete ui;
    delete this->is;
}

void ItemsDialog::initialize()
{
    this->invIdx = INVITEM_NONE;

    delete this->is;
    this->is = new ItemStruct();

    memset(this->is, 0, sizeof(ItemStruct));
    this->is->_iCreateInfo = 1 | (CFL_NONE << 8) | (CFDQ_NORMAL << 11);

    this->itemType = ITYPE_NONE;
    this->itemSeed = 0;

    QComboBox *locComboBox = this->ui->itemLocComboBox;
    locComboBox->clear();

    locComboBox->addItem(tr("Any"), QVariant::fromValue(ILOC_UNEQUIPABLE));
    locComboBox->addItem(tr("Helm"), QVariant::fromValue(ILOC_HELM));
    locComboBox->addItem(tr("Amulet"), QVariant::fromValue(ILOC_AMULET));
    locComboBox->addItem(tr("Armor"), QVariant::fromValue(ILOC_ARMOR));
    locComboBox->addItem(tr("One handed"), QVariant::fromValue(ILOC_ONEHAND));
    locComboBox->addItem(tr("Two handed"), QVariant::fromValue(ILOC_TWOHAND));
    locComboBox->addItem(tr("Ring"), QVariant::fromValue(ILOC_RING));

    this->updateFilters();
    this->updateFields();
}

void ItemsDialog::updateFilters()
{
    QComboBox *locComboBox = this->ui->itemLocComboBox;
    QComboBox *typeComboBox = this->ui->itemTypeComboBox;
    QComboBox *idxComboBox = this->ui->itemIdxComboBox;
    typeComboBox->clear();
    idxComboBox->clear();

    int iloc = locComboBox->currentData().value<int>();
    switch (iloc) {
    case ILOC_HELM:
        typeComboBox->addItem(tr("Helm"), QVariant::fromValue(ITYPE_HELM));
        break;
    case ILOC_AMULET:
        typeComboBox->addItem(tr("Amulet"), QVariant::fromValue(ITYPE_AMULET));
        break;
    case ILOC_ARMOR:
        typeComboBox->addItem(tr("Armor"), QVariant::fromValue(ITYPE_NONE));
        typeComboBox->addItem(tr("Light Armor"), QVariant::fromValue(ITYPE_LARMOR));
        typeComboBox->addItem(tr("Medium Armor"), QVariant::fromValue(ITYPE_MARMOR));
        typeComboBox->addItem(tr("Heavy Armor"), QVariant::fromValue(ITYPE_HARMOR));
        break;
    case ILOC_ONEHAND:
        typeComboBox->addItem(tr("All"), QVariant::fromValue(ITYPE_NONE));
        typeComboBox->addItem(tr("Sword"), QVariant::fromValue(ITYPE_SWORD));
        typeComboBox->addItem(tr("Axe"), QVariant::fromValue(ITYPE_AXE));
        typeComboBox->addItem(tr("Mace"), QVariant::fromValue(ITYPE_MACE));
        typeComboBox->addItem(tr("Shield"), QVariant::fromValue(ITYPE_SHIELD));
        break;
    case ILOC_TWOHAND:
        typeComboBox->addItem(tr("All"), QVariant::fromValue(ITYPE_NONE));
        typeComboBox->addItem(tr("Sword"), QVariant::fromValue(ITYPE_SWORD));
        typeComboBox->addItem(tr("Axe"), QVariant::fromValue(ITYPE_AXE));
        typeComboBox->addItem(tr("Bow"), QVariant::fromValue(ITYPE_BOW));
        typeComboBox->addItem(tr("Mace"), QVariant::fromValue(ITYPE_MACE));
        typeComboBox->addItem(tr("Staff"), QVariant::fromValue(ITYPE_STAFF));
        break;
    case ILOC_RING:
        typeComboBox->addItem(tr("Ring"), QVariant::fromValue(ITYPE_RING));
        break;
    default:
        typeComboBox->addItem(tr("All"), QVariant::fromValue(ITYPE_NONE));
        if (this->invIdx == INVITEM_NONE) {
            typeComboBox->addItem(tr("Helm"), QVariant::fromValue(ITYPE_HELM));
            typeComboBox->addItem(tr("Amulet"), QVariant::fromValue(ITYPE_AMULET));
            typeComboBox->addItem(tr("Ring"), QVariant::fromValue(ITYPE_RING));
            typeComboBox->addItem(tr("Light Armor"), QVariant::fromValue(ITYPE_LARMOR));
            typeComboBox->addItem(tr("Medium Armor"), QVariant::fromValue(ITYPE_MARMOR));
            typeComboBox->addItem(tr("Heavy Armor"), QVariant::fromValue(ITYPE_HARMOR));
        } else {
            // assert(this->invIdx == INVITEM_HAND_LEFT || this->invIdx == INVITEM_HAND_RIGHT);
        }
        typeComboBox->addItem(tr("Sword"), QVariant::fromValue(ITYPE_SWORD));
        typeComboBox->addItem(tr("Axe"), QVariant::fromValue(ITYPE_AXE));
        typeComboBox->addItem(tr("Bow"), QVariant::fromValue(ITYPE_BOW));
        typeComboBox->addItem(tr("Mace"), QVariant::fromValue(ITYPE_MACE));
        typeComboBox->addItem(tr("Staff"), QVariant::fromValue(ITYPE_STAFF));
        typeComboBox->addItem(tr("Shield"), QVariant::fromValue(ITYPE_SHIELD));
        break;
    }

    int idx = typeComboBox->findData(QVariant::fromValue((item_type)this->itemType));
    if (idx < 0) idx = 0;
    typeComboBox->setCurrentIndex(idx);

    int itype = typeComboBox->currentData().value<int>();
    for (int i = 0; i < NUM_IDI; ++i) {
        const ItemData &id = AllItemList[i];
        //if (id.iClass != this->is->_iClass) {
        //    continue;
        //}
        if (itype != ITYPE_NONE && itype != id.itype) {
            continue;
        }
        if (iloc != ILOC_UNEQUIPABLE && iloc != id.iLoc) {
            continue;
        }
        idxComboBox->addItem(QString("%1 (%2)").arg(id.iName).arg(i), QVariant::fromValue(i));
    }
    if (iloc == ILOC_UNEQUIPABLE) {
        idxComboBox->addItem(QString("%1 (%2)").arg(tr("Book")).arg(tr("Any")), QVariant::fromValue(NUM_IDI + 0));
        idxComboBox->addItem(QString("%1 (%2)").arg(tr("Scroll")).arg(tr("Any")), QVariant::fromValue(NUM_IDI + 1));
        idxComboBox->addItem(QString("%1 (%2)").arg(tr("Rune")).arg(tr("Any")), QVariant::fromValue(NUM_IDI + 2));
    }
    if (iloc == ILOC_UNEQUIPABLE || iloc == ILOC_RING) {
        idxComboBox->addItem(QString("%1 (%2)").arg(tr("Ring")).arg(tr("Any")), QVariant::fromValue(NUM_IDI + 3));
    }
    if (iloc == ILOC_UNEQUIPABLE || itype == ILOC_AMULET) {
        idxComboBox->addItem(QString("%1 (%2)").arg(tr("Amulet")).arg(tr("Any")), QVariant::fromValue(NUM_IDI + 4));
    }

    idx = idxComboBox->findData(QVariant::fromValue(this->is->_iIdx));
    if (idx < 0) idx = 0;
    idxComboBox->setCurrentIndex(idx);
}

QString ItemsDialog::AffixPowerName(int power)
{
    QString result = "";
    switch (power) {
    case IPL_TOHIT:          result = QApplication::tr("to hit");                break;
    case IPL_DAMP:           result = QApplication::tr("damage %");              break;
    case IPL_TOHIT_DAMP:     result = QApplication::tr("to hit + damage");       break;
    case IPL_ACP:            result = QApplication::tr("armor %");               break;
    case IPL_TOBLOCK:        result = QApplication::tr("block %");               break;
    case IPL_FIRERES:        result = QApplication::tr("fire res.");             break;
    case IPL_LIGHTRES:       result = QApplication::tr("light res.");            break;
    case IPL_MAGICRES:       result = QApplication::tr("magic res.");            break;
    case IPL_ACIDRES:        result = QApplication::tr("acid res.");             break;
    case IPL_ALLRES:         result = QApplication::tr("all res.");              break;
    case IPL_CRITP:          result = QApplication::tr("crit. %");               break;
    case IPL_SKILLLVL:       result = QApplication::tr("skill");                 break;
    case IPL_SKILLLEVELS:    result = QApplication::tr("skills");                break;
    case IPL_CHARGES:        result = QApplication::tr("charges");               break;
    case IPL_FIREDAM:        result = QApplication::tr("fire damage");           break;
    case IPL_LIGHTDAM:       result = QApplication::tr("lightning  damage");     break;
    case IPL_MAGICDAM:       result = QApplication::tr("magic damage");          break;
    case IPL_ACIDDAM:        result = QApplication::tr("acid damage");           break;
    case IPL_STR:            result = QApplication::tr("strength");              break;
    case IPL_MAG:            result = QApplication::tr("magic");                 break;
    case IPL_DEX:            result = QApplication::tr("dexterity");             break;
    case IPL_VIT:            result = QApplication::tr("vitality");              break;
    case IPL_ATTRIBS:        result = QApplication::tr("attributes");            break;
    case IPL_ABS_ANYHIT:     result = QApplication::tr("damage taken");          break;
    case IPL_ABS_PHYHIT:     result = QApplication::tr("phy. damage taken");     break;
    case IPL_LIFE:           result = QApplication::tr("life");                  break;
    case IPL_MANA:           result = QApplication::tr("mana");                  break;
    case IPL_DUR:            result = QApplication::tr("durability");            break;
    case IPL_INDESTRUCTIBLE: result = QApplication::tr("indestructible");        break;
    case IPL_LIGHT:          result = QApplication::tr("light range");           break;
    //case IPL_INVCURS: result = QApplication::tr("xxx"); break;
    //case IPL_THORNS: result = QApplication::tr("xxx"); break;
    case IPL_NOMANA:         result = QApplication::tr("no mana");               break;
    case IPL_KNOCKBACK:      result = QApplication::tr("knockback");             break;
    case IPL_STUN:           result = QApplication::tr("stun");                  break;
    //case IPL_NOHEALMON: result = QApplication::tr("xxx"); break;
    case IPL_NO_BLEED:       result = QApplication::tr("no bleed");              break;
    case IPL_BLEED:          result = QApplication::tr("bleed");                 break;
    case IPL_STEALMANA:      result = QApplication::tr("steal mana");            break;
    case IPL_STEALLIFE:      result = QApplication::tr("steal life");            break;
    case IPL_PENETRATE_PHYS: result = QApplication::tr("penetrate phy.");        break;
    case IPL_FASTATTACK:     result = QApplication::tr("attack speed");          break;
    case IPL_FASTRECOVER:    result = QApplication::tr("recovery speed");        break;
    case IPL_FASTBLOCK:      result = QApplication::tr("block speed");           break;
    case IPL_DAMMOD:         result = QApplication::tr("damage +");              break;
    case IPL_SETDAM:         result = QApplication::tr("damage *");              break;
    case IPL_SETDUR:         result = QApplication::tr("durability *");          break;
    case IPL_REQSTR:         result = QApplication::tr("altered requirements");  break;
    case IPL_SKILL:          result = QApplication::tr("rnd spell");             break;
    case IPL_SETSKILL:       result = QApplication::tr("fix spell");             break;
    case IPL_ONEHAND:        result = QApplication::tr("one handed");            break;
    case IPL_ALLRESZERO:     result = QApplication::tr("all res. zero");         break;
    case IPL_DRAINLIFE:      result = QApplication::tr("drain life");            break;
    //case IPL_INFRAVISION: result = QApplication::tr("xxx"); break;
    case IPL_SETAC:          result = QApplication::tr("armor *");               break;
    case IPL_ACMOD:          result = QApplication::tr("armor +");               break;
    case IPL_CRYSTALLINE:    result = QApplication::tr("damage % durability -"); break;
    case IPL_MANATOLIFE:     result = QApplication::tr("mana to life");          break;     /* only used in hellfire */
    case IPL_LIFETOMANA:     result = QApplication::tr("life to mana");          break;     /* only used in hellfire */
    case IPL_FASTCAST:       result = QApplication::tr("cast speed");            break;
    case IPL_FASTWALK:       result = QApplication::tr("walk speed");            break;
    }
    return result;
}

static QString AffixName(const AffixData *affix)
{
    return ItemsDialog::AffixPowerName(affix->PLPower);
}

static void addUniqueOption(int power, int paramA, int paramB, int idx, QComboBox *preComboBox, QComboBox *sufComboBox)
{
    QComboBox *comboBox = preComboBox->count() == 0 ? preComboBox : sufComboBox;
    if (power == IPL_SKILLLVL && paramA == paramB) {
        paramA = 0;
        paramB = GetBookSpell(INT_MAX, -2) - 1;
    }
    if (paramA == paramB) {
        return;
    }
    comboBox->addItem(QString("%1 (%2..%3)").arg(ItemsDialog::AffixPowerName(power)).arg(paramA).arg(paramB), QVariant::fromValue(idx));
}

static QString ItemColor(const ItemStruct* is)
{
    QString color;
    if (is->_itype != ITYPE_NONE) {
        if (is->_iMagical == ITEM_QUALITY_MAGIC)
            color = QString("color:#58638d;"); // blue
        if (is->_iMagical == ITEM_QUALITY_UNIQUE)
            return QString("color:#ccb775;"); // beer #ddc47e
    }
    return color;
}

void ItemsDialog::updateFields()
{
    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->ui->isHellfireCheckBox->isChecked();
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->ui->isMultiCheckBox->isChecked();

    // QComboBox *typeComboBox = this->ui->itemTypeComboBox;
    // QComboBox *locComboBox = this->ui->itemLocComboBox;
    QComboBox *idxComboBox = this->ui->itemIdxComboBox;

    // locComboBox->setCurrentIndex(locComboBox->findData(this->is->_iLoc));
    // typeComboBox->setCurrentIndex(typeComboBox->findData(this->is->_itype));
    // idxComboBox->setCurrentIndex(idxComboBox->findData(this->is->_iIdx));

    int idx = idxComboBox->currentData().value<int>();
    if (idx >= NUM_IDI) {
        switch (idx) {
        case NUM_IDI + 0: idx = IDI_BOOK1;   break;
        case NUM_IDI + 1: idx = IDI_SCROLL1; break;
        case NUM_IDI + 2: idx = IDI_RUNE1;   break;
        case NUM_IDI + 3: idx = IDI_RING1;   break;
        case NUM_IDI + 4: idx = IDI_AMULET1; break;
        }
    }
    bool drop = AllItemList[idx].iRnd != 0;
    if (idx != this->is->_iIdx) {
        this->is->_iIdx = idx;
        this->is->_itype = ITYPE_NONE;
        if (!drop)
            this->is->_iCreateInfo &= CF_LEVEL;
        this->resetSlider = 7;
    }
    const int ci = this->is->_iCreateInfo;
    this->ui->itemSeedEdit->setText(QString::number(this->itemSeed));
    this->ui->itemLevelEdit->setText(QString::number(ci & CF_LEVEL));
    static_assert(((int)CF_TOWN & ((1 << 8) - 1)) == 0, "ItemsDialog hardcoded CF_TOWN must be adjusted I.");
    static_assert((((int)CF_TOWN >> 8) & ((((int)CF_TOWN >> 8) + 1))) == 0, "ItemsDialog hardcoded CF_TOWN must be adjusted II.");
    this->ui->itemSourceComboBox->setCurrentIndex((ci & CF_TOWN) >> 8);
    this->ui->itemSourceComboBox->setEnabled(drop);
    static_assert(((int)CF_DROP_QUALITY & ((1 << 11) - 1)) == 0, "ItemsDialog hardcoded CF_DROP_QUALITY must be adjusted I.");
    static_assert((((int)CF_DROP_QUALITY >> 11) & ((((int)CF_DROP_QUALITY >> 11) + 1))) == 0, "ItemsDialog hardcoded CF_DROP_QUALITY must be adjusted II.");
    this->ui->itemQualityComboBox->setCurrentIndex((ci & CF_DROP_QUALITY) >> 11);
    this->ui->itemQualityComboBox->setEnabled(drop);

    this->ui->itemName->setText(this->is->_itype != ITYPE_NONE ? ItemName(this->is) : "");
    this->ui->itemName->setStyleSheet(ItemColor(this->is));
    this->itemProps->initialize(this->is);
    this->itemProps->adjustSize();
    this->ui->itemPropertiesBox->adjustSize();
    this->itemProps->setVisible(this->is->_itype != ITYPE_NONE);

    // update whish-lists
    int flgs = GetItemBonusFlags(AllItemList[idx].itype /* this->is->_itype*/, AllItemList[idx].iMiscId/* this->is->_iMiscId*/);
    int source = (ci & CF_TOWN) >> 8;
    int range = source == CFL_NONE ? IAR_DROP : (source == CFL_CRAFTED ? IAR_CRAFT : IAR_SHOP);
    int lvl = ci & CF_LEVEL;
    int si, limitMode;
    bool active;
    Qt::CheckState cs;

    // add possible AC range
    active = AllItemList[idx].iMinAC != AllItemList[idx].iMaxAC;
    cs = this->ui->itemACLimitedCheckBox->checkState();
    this->ui->itemACLimitedCheckBox->setEnabled(active);
    this->ui->itemACLimitedCheckBox->setToolTip(cs == Qt::Unchecked ? tr("unrestricted") : (cs == Qt::PartiallyChecked ? tr("lower limited to:") : tr("upper limited to:")));
    this->ui->itemACLimitSlider->setEnabled(cs != Qt::Unchecked && active);
    if (active) {
        int minval, maxval;
        minval = AllItemList[idx].iMinAC;
        maxval = AllItemList[idx].iMaxAC;
        this->ui->itemACLimitSlider->setMinimum(minval);
        this->ui->itemACLimitSlider->setMaximum(maxval);
        if (this->resetSlider & 4) {
            this->resetSlider &= ~4;
            this->ui->itemACLimitSlider->changeValue(cs == Qt::Checked ? maxval : minval);
        }
    }

    // update possible uniques
    QComboBox *uniqComboBox = this->ui->itemUniquesComboBox;
    uniqComboBox->clear();

    uniqComboBox->addItem(tr("Any"), QVariant::fromValue(-1));

    if ((ci & ~CF_LEVEL) != 0) {
        for (int i = 0; i < (IsHellfireGame ? NUM_UITEM : NUM_UITEM_DIABLO); i++) {
            const UniqItemData &uid = UniqueItemList[i];
            if (uid.UIUniqType == AllItemList[idx].iUniqType && uid.UIMinLvl <= lvl) {
                uniqComboBox->addItem(uid.UIName, QVariant::fromValue(i));
            }
        }
    }
    if (uniqComboBox->count() > 1)
        uniqComboBox->addItem(tr("None"), QVariant::fromValue(-2));
    uniqComboBox->setEnabled(drop);
    uniqComboBox->adjustSize();
    si = uniqComboBox->findData(QVariant::fromValue(this->wishUniq));
    if (si < 0) si = 0;
    uniqComboBox->setCurrentIndex(si);

    // update possible affixes
    QComboBox *preComboBox = this->ui->itemPrefixComboBox;
    QComboBox *sufComboBox = this->ui->itemSuffixComboBox;
    preComboBox->clear();
    sufComboBox->clear();

    int uniqIdx = uniqComboBox->currentData().value<int>();
    if (uniqIdx < 0) {
        preComboBox->addItem(tr("Any"), QVariant::fromValue(AFFIX_ANY));
        sufComboBox->addItem(tr("Any"), QVariant::fromValue(AFFIX_ANY));

        if ((ci & ~CF_LEVEL) != 0) {
            const BOOLEAN good = ((ci & CF_DROP_QUALITY) >> 11) >= CFDQ_GOOD;
            int alvl = lvl;
            if (flgs != PLT_JEWEL) // items[ii]._itype != ITYPE_RING && items[ii]._itype != ITYPE_AMULET)
                alvl = alvl > AllItemList[idx].iMinMLvl ? alvl - AllItemList[idx].iMinMLvl : 0;
            si = 0;
            for (const AffixData *pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++, si++) {
                if ((flgs & pres->PLIType && good <= pres->PLOk)
                    && pres->PLRanges[range].from <= alvl && pres->PLRanges[range].to >= alvl) {
                    preComboBox->addItem(QString("%1 (%2..%3)").arg(AffixName(pres)).arg(pres->PLParam1).arg(pres->PLParam2), QVariant::fromValue(si));
                }
            }
            si = 0;
            for (const AffixData *sufs = PL_Suffix; sufs->PLPower != IPL_INVALID; sufs++, si++) {
                if ((flgs & sufs->PLIType && good <= sufs->PLOk)
                    && sufs->PLRanges[range].from <= alvl && sufs->PLRanges[range].to >= alvl) {
                    sufComboBox->addItem(QString("%1 (%2..%3)").arg(AffixName(sufs)).arg(sufs->PLParam1).arg(sufs->PLParam2), QVariant::fromValue(si));
                }
            }
        }
        if (flgs == PLT_MISC) {
            preComboBox->addItem(QString("%1").arg(AffixPowerName(IPL_SETSKILL)), AFFIX_SKILL);
        } else {
            if (preComboBox->count() > 1)
                preComboBox->addItem(tr("None"), QVariant::fromValue(AFFIX_NONE));
            if (sufComboBox->count() > 1)
                sufComboBox->addItem(tr("None"), QVariant::fromValue(AFFIX_NONE));
        }
    } else {
        // if ((ci & ~CF_LEVEL) != 0) {
            const UniqItemData* ui = &UniqueItemList[uniqIdx];
            addUniqueOption(ui->UIPower1, ui->UIParam1a, ui->UIParam1b, 0, preComboBox, sufComboBox);
            if (ui->UIPower2 != IPL_INVALID) {
                addUniqueOption(ui->UIPower2, ui->UIParam2a, ui->UIParam2b, 1, preComboBox, sufComboBox);
            if (ui->UIPower3 != IPL_INVALID) {
                addUniqueOption(ui->UIPower3, ui->UIParam3a, ui->UIParam3b, 2, preComboBox, sufComboBox);
            if (ui->UIPower4 != IPL_INVALID) {
                addUniqueOption(ui->UIPower4, ui->UIParam4a, ui->UIParam4b, 3, preComboBox, sufComboBox);
            if (ui->UIPower5 != IPL_INVALID) {
                addUniqueOption(ui->UIPower5, ui->UIParam5a, ui->UIParam5b, 4, preComboBox, sufComboBox);
            if (ui->UIPower6 != IPL_INVALID) {
                addUniqueOption(ui->UIPower6, ui->UIParam6a, ui->UIParam6b, 5, preComboBox, sufComboBox);
            }}}}}
        // }
        if (preComboBox->count() == 0)
            preComboBox->addItem(tr("Any"), QVariant::fromValue(AFFIX_ANY));
        if (sufComboBox->count() == 0)
            sufComboBox->addItem(tr("Any"), QVariant::fromValue(AFFIX_ANY));
    }
    preComboBox->setEnabled(drop);
    sufComboBox->setEnabled(drop);

    preComboBox->adjustSize();
    sufComboBox->adjustSize();
    si = preComboBox->findData(QVariant::fromValue(this->wishPre));
    if (si < 0) si = 0;
    preComboBox->setCurrentIndex(si);
    si = sufComboBox->findData(QVariant::fromValue(this->wishSuf));
    if (si < 0) si = 0;
    sufComboBox->setCurrentIndex(si);

    si = preComboBox->currentData().value<int>();
    active = (si != AFFIX_ANY && si != AFFIX_NONE) && (si == AFFIX_SKILL || uniqIdx >= 0 || PL_Prefix[si].PLPower == IPL_SKILLLVL || (PL_Prefix[si].PLParam1 != PL_Prefix[si].PLParam2));
    this->ui->itemPrefixLimitedCheckBox->setEnabled(active);
    cs = this->ui->itemPrefixLimitedCheckBox->checkState();
    active &= cs != Qt::Unchecked;
    limitMode = cs == Qt::Unchecked ? 0 : cs == Qt::PartiallyChecked ? 1 : cs == Qt::Checked ? 2 : cs;
    if (active) {
        int minval, maxval;
        if (si == AFFIX_SKILL) {
            minval = maxval = 0;
            if (limitMode == 1) {
                switch (AllItemList[idx].iMiscId) {
                case IMISC_BOOK:   maxval = GetBookSpell(lvl, -2);   break;
                case IMISC_SCROLL: maxval = GetScrollSpell(lvl, -2); break;
                case IMISC_RUNE:   maxval = GetRuneSpell(lvl, -2);   break;
                }
                maxval--;
                limitMode = 3;
            }
        } else if (uniqIdx >= 0) {
            const UniqItemData* ui = &UniqueItemList[uniqIdx];
            int power;
            switch (si) {
            case 0: power = ui->UIPower1; minval = ui->UIParam1a; maxval = ui->UIParam1b; break;
            case 1: power = ui->UIPower2; minval = ui->UIParam2a; maxval = ui->UIParam2b; break;
            case 2: power = ui->UIPower3; minval = ui->UIParam3a; maxval = ui->UIParam3b; break;
            case 3: power = ui->UIPower4; minval = ui->UIParam4a; maxval = ui->UIParam4b; break;
            case 4: power = ui->UIPower5; minval = ui->UIParam5a; maxval = ui->UIParam5b; break;
            case 5: power = ui->UIPower6; minval = ui->UIParam6a; maxval = ui->UIParam6b; break;
            }
            if (power == IPL_SKILLLVL && limitMode == 1) {
                minval = 0;
                maxval = GetBookSpell(lvl, -2) - 1;
                limitMode = 3;
            }
        } else if (PL_Prefix[si].PLPower == IPL_SKILLLVL && limitMode == 1) {
            minval = 0;
            maxval = GetBookSpell(lvl, -2) - 1;
            limitMode = 3;
        } else {
            minval = PL_Prefix[si].PLParam1;
            maxval = PL_Prefix[si].PLParam2;
        }
        active &= minval < maxval;
        if (active) {
            this->ui->itemPrefixLimitSlider->setMinimum(minval);
            this->ui->itemPrefixLimitSlider->setMaximum(maxval);
            if (this->resetSlider & 1) {
                this->resetSlider &= ~1;
                this->ui->itemPrefixLimitSlider->changeValue(limitMode == 2 ? maxval : minval);
            }
        }
    }
    this->ui->itemPrefixLimitSlider->setItemLevel(lvl);
    this->ui->itemPrefixLimitSlider->setItemMiscId(AllItemList[idx].iMiscId);
    this->ui->itemPrefixLimitSlider->setLimitMode(active ? limitMode : -1);
    this->ui->itemPrefixLimitedCheckBox->setToolTip((!active || limitMode == 0) ? tr("unrestricted") : (limitMode == 1 ? tr("lower limited to:") : (limitMode == 2 ? tr("upper limited to:") : tr("limited to:"))));

    si = sufComboBox->currentData().value<int>();
    active = (si != AFFIX_ANY && si != AFFIX_NONE) && (uniqIdx >= 0 || PL_Suffix[si].PLPower == IPL_SKILLLVL || (PL_Suffix[si].PLParam1 != PL_Suffix[si].PLParam2));
    this->ui->itemSuffixLimitedCheckBox->setEnabled(active);
    cs = this->ui->itemSuffixLimitedCheckBox->checkState();
    active &= cs != Qt::Unchecked;
    limitMode = cs == Qt::Unchecked ? 0 : cs == Qt::PartiallyChecked ? 1 : cs == Qt::Checked ? 2 : cs;
    if (active) {
        int minval, maxval;
        if (uniqIdx >= 0) {
            const UniqItemData* ui = &UniqueItemList[uniqIdx];
            int power;
            switch (si) {
            case 0: power = ui->UIPower1; minval = ui->UIParam1a; maxval = ui->UIParam1b; break;
            case 1: power = ui->UIPower2; minval = ui->UIParam2a; maxval = ui->UIParam2b; break;
            case 2: power = ui->UIPower3; minval = ui->UIParam3a; maxval = ui->UIParam3b; break;
            case 3: power = ui->UIPower4; minval = ui->UIParam4a; maxval = ui->UIParam4b; break;
            case 4: power = ui->UIPower5; minval = ui->UIParam5a; maxval = ui->UIParam5b; break;
            case 5: power = ui->UIPower6; minval = ui->UIParam6a; maxval = ui->UIParam6b; break;
            }
            if (power == IPL_SKILLLVL && limitMode == 1) {
                minval = 0;
                maxval = GetBookSpell(lvl, -2) - 1;
                limitMode = 3;
            }
        } else if (PL_Prefix[si].PLPower == IPL_SKILLLVL && limitMode == 1) {
            minval = 0;
            maxval = GetBookSpell(lvl, -2) - 1;
            limitMode = 3;
        } else {
            minval = PL_Suffix[si].PLParam1;
            maxval = PL_Suffix[si].PLParam2;
        }
        active &= minval < maxval;
        if (active) {
            this->ui->itemSuffixLimitSlider->setMinimum(minval);
            this->ui->itemSuffixLimitSlider->setMaximum(maxval);
            if (this->resetSlider & 2) {
                this->resetSlider &= ~2;
                this->ui->itemSuffixLimitSlider->changeValue(limitMode == 2 ? maxval : minval);
            }
        }
    }
    this->ui->itemSuffixLimitSlider->setItemLevel(lvl);
    this->ui->itemSuffixLimitSlider->setItemMiscId(AllItemList[idx].iMiscId);
    this->ui->itemSuffixLimitSlider->setLimitMode(active ? limitMode : -1);
    this->ui->itemSuffixLimitedCheckBox->setToolTip(limitMode == 0 ? tr("unrestricted") : (limitMode == 1 ? tr("lower limited to:") : (limitMode == 2 ? tr("upper limited to:") : tr("limited to:"))));

    IsMultiGame = gameMulti;
    IsHellfireGame = gameHellfire;
}

void ItemsDialog::on_playersEdit_returnPressed()
{
    this->numPlayers = this->ui->playersEdit->text().toUShort();

    this->on_playersEdit_escPressed();
}

void ItemsDialog::on_playersEdit_escPressed()
{
    this->updateFields();
    this->ui->playersEdit->clearFocus();
}

void ItemsDialog::on_itemTypeComboBox_activated(int index)
{
    this->itemType = this->ui->itemTypeComboBox->currentData().value<int>();
    this->updateFilters();
}

void ItemsDialog::on_itemLocComboBox_activated(int index)
{
    this->updateFilters();
}

void ItemsDialog::on_itemIdxComboBox_activated(int index)
{
    // this->is->_iIdx = this->ui->itemIdxComboBox->currentData().value<int>();
    this->updateFields();
}

void ItemsDialog::on_itemSeedEdit_returnPressed()
{
    bool ok;
    QString seedTxt = this->ui->itemSeedEdit->text();
    int seed = seedTxt.toInt(&ok);
    if (ok || seedTxt.isEmpty()) {
        this->itemSeed = seed;
    } else {
        QMessageBox::critical(this, "Error", "Failed to parse the seed to a 32-bit integer.");
    }

    this->on_itemSeedEdit_escPressed();
}

void ItemsDialog::on_itemSeedEdit_escPressed()
{
    this->updateFields();
    this->ui->itemSeedEdit->clearFocus();
}

void ItemsDialog::on_actionGenerateSeed_triggered()
{
    QRandomGenerator *gen = QRandomGenerator::global();
    this->itemSeed = (int)gen->generate();
    this->updateFields();
}

void ItemsDialog::on_itemLevelEdit_returnPressed()
{
    this->is->_iCreateInfo = (this->is->_iCreateInfo & ~CF_LEVEL) | (this->ui->itemLevelEdit->text().toShort() & CF_LEVEL);

    this->on_itemLevelEdit_escPressed();
}

void ItemsDialog::on_itemLevelEdit_escPressed()
{
    this->updateFields();
    this->ui->itemLevelEdit->clearFocus();
}

void ItemsDialog::on_itemSourceComboBox_activated(int index)
{
    this->is->_iCreateInfo = (this->is->_iCreateInfo & ~CF_TOWN) | (index << 8);
    this->updateFields();
}

void ItemsDialog::on_itemQualityComboBox_activated(int index)
{
    this->is->_iCreateInfo = (this->is->_iCreateInfo & ~CF_DROP_QUALITY) | (index << 11);
    this->updateFields();
}

void ItemsDialog::on_itemUniquesComboBox_activated(int index)
{
    this->wishUniq = this->ui->itemUniquesComboBox->itemData(index).value<int>();
    this->resetSlider = 7;
    this->updateFields();
}

void ItemsDialog::on_itemPrefixComboBox_activated(int index)
{
    this->wishPre = this->ui->itemPrefixComboBox->itemData(index).value<int>();
    this->resetSlider |= 1;
    this->updateFields();
}

void ItemsDialog::on_itemSuffixComboBox_activated(int index)
{
    this->wishSuf = this->ui->itemSuffixComboBox->itemData(index).value<int>();
    this->resetSlider |= 2;
    this->updateFields();
}

void ItemsDialog::on_itemPrefixLimitedCheckBox_clicked()
{
    this->updateFields();
}

void ItemsDialog::on_itemSuffixLimitedCheckBox_clicked()
{
    this->updateFields();
}

/*void ItemsDialog::on_itemPrefixLimitSlider_valueChanged(int value)
{
    QString text;
    if (this->preLimitMode != 3) {
        text = QString::number(value);
    } else {
        text = spelldata[GetBookSpell(lvl, value)].sNameText;
    }
    this->ui->itemPrefixLimitSlider->setToolTip(text);
    QToolTip::showText(this->ui->itemPrefixLimitSlider->mapToGlobal(QPoint(0, 0)), text);
}

void ItemsDialog::on_itemSuffixLimitSlider_valueChanged(int value)
{
    QString text;
    if (this->sufLimitMode != 3) {
        text = QString::number(value);
    } else {
        text = spelldata[GetBookSpell(lvl, value)].sNameText;
    }
    this->ui->itemSuffixLimitSlider->setToolTip(text);
    QToolTip::showText(this->ui->itemSuffixLimitSlider->mapToGlobal(QPoint(0, 0)), text);
}*/

void ItemsDialog::on_itemACLimitedCheckBox_clicked()
{
    this->updateFields();
}

void ItemsDialog::on_itemACLimitSlider_valueChanged(int value)
{
    this->ui->itemACLimitSlider->setToolTip(QString::number(value));
}

bool ItemsDialog::recreateItem()
{
    int seed = this->itemSeed;
    int wCI = this->is->_iCreateInfo;
    int wIdx = this->is->_iIdx;

    int uniqIdx = this->ui->itemUniquesComboBox->currentData().value<int>();
    int preIdx = this->ui->itemPrefixComboBox->currentData().value<int>();
    int sufIdx = this->ui->itemSuffixComboBox->currentData().value<int>();
    int acLowest = 0, acHighest = INT_MAX;
    if (this->ui->itemACLimitSlider->isEnabled()) {
        acLowest = AllItemList[wIdx].iMinAC;
        acHighest = AllItemList[wIdx].iMaxAC;
        int val = this->ui->itemACLimitSlider->value();
        if (this->ui->itemACLimitedCheckBox->checkState() == Qt::PartiallyChecked) {
            acLowest = val;
        } else {
            acHighest = val;
        }
    }
    int lvl = wCI & CF_LEVEL;

    typedef struct UIAffixData {
        bool active;
        BYTE power;
        int param1;
        int param2;
    } UIAffixData;
    UIAffixData prefix = { false, 0 };
    UIAffixData suffix = { false, 0 };
    prefix.active = preIdx != AFFIX_ANY;
    if (prefix.active) {
        if (preIdx != AFFIX_NONE) {
            if (preIdx == AFFIX_SKILL) {
                prefix.power = IPL_SKILL;
                prefix.param1 = INT_MIN;
                prefix.param2 = INT_MAX;
            } else if (uniqIdx >= 0) {
                const UniqItemData* ui = &UniqueItemList[uniqIdx];
                switch (preIdx) {
                case 0: prefix.power = ui->UIPower1; prefix.param1 = ui->UIParam1a; prefix.param2 = ui->UIParam1b; break;
                case 1: prefix.power = ui->UIPower2; prefix.param1 = ui->UIParam2a; prefix.param2 = ui->UIParam2b; break;
                case 2: prefix.power = ui->UIPower3; prefix.param1 = ui->UIParam3a; prefix.param2 = ui->UIParam3b; break;
                case 3: prefix.power = ui->UIPower4; prefix.param1 = ui->UIParam4a; prefix.param2 = ui->UIParam4b; break;
                case 4: prefix.power = ui->UIPower5; prefix.param1 = ui->UIParam5a; prefix.param2 = ui->UIParam5b; break;
                case 5: prefix.power = ui->UIPower6; prefix.param1 = ui->UIParam6a; prefix.param2 = ui->UIParam6b; break;
                }
            } else {
                const AffixData *affix = &PL_Prefix[preIdx];
                prefix.power = affix->PLPower;
                prefix.param1 = affix->PLParam1;
                prefix.param2 = affix->PLParam2;
            }
            if (this->ui->itemPrefixLimitSlider->isEnabled()) {
                int val = this->ui->itemPrefixLimitSlider->value();
                Qt::CheckState cs = this->ui->itemPrefixLimitedCheckBox->checkState();
                if ((prefix.power == IPL_SKILLLVL || prefix.power == IPL_SKILL) && cs == Qt::PartiallyChecked) {
                    switch (AllItemList[wIdx].iMiscId) {
                    case IMISC_SCROLL: prefix.param1 = GetScrollSpell(lvl, val); break;
                    case IMISC_RUNE:   prefix.param1 = GetRuneSpell(lvl, val);   break;
                    case IMISC_BOOK:
                    default:           prefix.param1 = GetBookSpell(lvl, val);   break;
                    }
                    prefix.param2 = MAXSPLLEVEL + 1;
                } else if (cs == Qt::PartiallyChecked) {
                    prefix.param1 = val;
                } else {
                    prefix.param2 = val;
                }
            }
        } else {
            prefix.power = IPL_INVALID;
        }
    }
    suffix.active = sufIdx != AFFIX_ANY;
    if (suffix.active) {
        if (sufIdx != AFFIX_NONE) {
            if (uniqIdx >= 0) {
                const UniqItemData* ui = &UniqueItemList[uniqIdx];
                switch (sufIdx) {
                case 0: suffix.power = ui->UIPower1; suffix.param1 = ui->UIParam1a; suffix.param2 = ui->UIParam1b; break;
                case 1: suffix.power = ui->UIPower2; suffix.param1 = ui->UIParam2a; suffix.param2 = ui->UIParam2b; break;
                case 2: suffix.power = ui->UIPower3; suffix.param1 = ui->UIParam3a; suffix.param2 = ui->UIParam3b; break;
                case 3: suffix.power = ui->UIPower4; suffix.param1 = ui->UIParam4a; suffix.param2 = ui->UIParam4b; break;
                case 4: suffix.power = ui->UIPower5; suffix.param1 = ui->UIParam5a; suffix.param2 = ui->UIParam5b; break;
                case 5: suffix.power = ui->UIPower6; suffix.param1 = ui->UIParam6a; suffix.param2 = ui->UIParam6b; break;
                }
            } else {
                const AffixData *affix = &PL_Suffix[sufIdx];
                suffix.power = affix->PLPower;
                suffix.param1 = affix->PLParam1;
                suffix.param2 = affix->PLParam2;
            }
            if (this->ui->itemSuffixLimitSlider->isEnabled()) {
                int val = this->ui->itemSuffixLimitSlider->value();
                Qt::CheckState cs = this->ui->itemSuffixLimitedCheckBox->checkState();
                if (suffix.power == IPL_SKILLLVL && cs == Qt::PartiallyChecked) {
                    suffix.param1 = GetBookSpell(lvl, val);
                    suffix.param2 = MAXSPLLEVEL + 1;
                } else if (cs == Qt::PartiallyChecked) {
                    suffix.param1 = val;
                } else {
                    suffix.param2 = val;
                }
            }
        } else {
            suffix.power = IPL_INVALID;
        }
    }

    int counter = 0;
    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->ui->isHellfireCheckBox->isChecked();
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->ui->isMultiCheckBox->isChecked();
start:
    RecreateItem(seed, wIdx, wCI);

    if (ac_rnd < acLowest || ac_rnd > acHighest) {
        goto restart;
    }
    if (uniqIdx >= 0) {
        if (items[MAXITEMS]._iMagical != ITEM_QUALITY_UNIQUE || items[MAXITEMS]._iUid != uniqIdx) {
            // if (items[MAXITEMS]._iMagical != ITEM_QUALITY_UNIQUE)
                // LogErrorF("missed uniq-quality %d vs [%d%d] (%d) seed%d", affix_rnd[preIdx], prefix.param1, prefix.param2, preIdx, seed);
            // else
            //     LogErrorF("missed uniq-index %d vs %d seed%d", items[MAXITEMS]._iUid, uniqIdx, seed);
            goto restart;
        }
        if (prefix.active) {
            if (prefix.power == IPL_SKILLLVL && prefix.param2 == MAXSPLLEVEL + 1) {
                const ItemAffixStruct *ia = items[MAXITEMS]._iNumAffixes == 0 ? NULL : &items[MAXITEMS]._iAffixes[0];
                if (ia == NULL || ia->asPower != IPL_SKILLLVL || ia->asValue1 != prefix.param1) {
                    // LogErrorF("missed uniq-prefix %d vs %d (%d) seed%d", ia == NULL ? -1 : ia->asPower, prefix.param1, preIdx, seed);
                    goto restart;
                }
            } else if (affix_rnd[preIdx] < prefix.param1 || affix_rnd[preIdx] > prefix.param2) {
                // LogErrorF("missed uniq-prefix %d vs [%d%d] (%d) seed%d", affix_rnd[preIdx], prefix.param1, prefix.param2, preIdx, seed);
                goto restart;
            }
        }
        if (suffix.active) {
            if (suffix.power == IPL_SKILLLVL && suffix.param2 == MAXSPLLEVEL + 1) {
                const ItemAffixStruct *ia = items[MAXITEMS]._iNumAffixes == 0 ? NULL : (items[MAXITEMS]._iNumAffixes == 1 ? &items[MAXITEMS]._iAffixes[0] : &items[MAXITEMS]._iAffixes[1]);
                if (ia == NULL || ia->asPower != IPL_SKILLLVL || ia->asValue1 != suffix.param1) {
                    // LogErrorF("missed uniq-suffix %d vs %d (%d) seed%d", ia == NULL ? -1 : ia->asPower, suffix.param1, sufIdx, seed);
                    goto restart;
                }
            } else if (affix_rnd[sufIdx] < suffix.param1 || affix_rnd[sufIdx] > suffix.param2) {
                // LogErrorF("missed uniq-suffix %d vs [%d%d] (%d) seed%d", affix_rnd[sufIdx], suffix.param1, suffix.param2, sufIdx, seed);
                goto restart;
            }
        }
    } else {
        if (uniqIdx == -2 && items[MAXITEMS]._iMagical == ITEM_QUALITY_UNIQUE) {
            goto restart;
        }
        if (prefix.active) {
            if (items[MAXITEMS]._iMiscId != IMISC_NONE) {
                if (prefix.param2 == MAXSPLLEVEL + 1 && items[MAXITEMS]._iSpell != prefix.param1) {
                    goto restart;
                }
            } else {
            if (items[MAXITEMS]._iNumAffixes == 0 || items[MAXITEMS]._iAffixes[0].asPower != prefix.power) {
                // LogErrorF("missed prefix %d vs %d (%d) seed%d", items[MAXITEMS]._iAffixes[0].asPower, prefix.power, preIdx, seed);
                goto restart;
            }
            if (prefix.power != IPL_INVALID) {
                LogErrorF("matched prefix rndval: %d (%d..%d) (%d) seed%d", affix_rnd[0], prefix.param1, prefix.param2, preIdx, seed);
                if (prefix.power == IPL_SKILLLVL && prefix.param2 == MAXSPLLEVEL + 1) {
                    const ItemAffixStruct *ia = items[MAXITEMS]._iNumAffixes == 0 ? NULL : &items[MAXITEMS]._iAffixes[0];
                    if (ia == NULL || ia->asPower != IPL_SKILLLVL || ia->asValue1 != prefix.param1) {
                        // LogErrorF("missed preval %d vs %d (%d) seed%d", ia == NULL ? -1 : ia->asPower, prefix.param1, preIdx, seed);
                        goto restart;
                    }
                } else if (affix_rnd[0] < prefix.param1 || affix_rnd[0] > prefix.param2) {
                    LogErrorF("missed preval %d vs [%d:%d]", affix_rnd[0], prefix.param1, prefix.param2);
                    goto restart;
                }
            }
            }
        }
        if (suffix.active) {
            if (items[MAXITEMS]._iNumAffixes == 0 || (items[MAXITEMS]._iNumAffixes == 1 && items[MAXITEMS]._iAffixes[0].asPower != suffix.power) || (items[MAXITEMS]._iNumAffixes > 1 && items[MAXITEMS]._iAffixes[1].asPower != suffix.power)) {
                // LogErrorF("missed suffix %d (%d,%d) vs %d (%d) seed%d", items[MAXITEMS]._iNumAffixes, items[MAXITEMS]._iAffixes[0].asPower, items[MAXITEMS]._iAffixes[1].asPower, suffix.power, sufIdx);
                goto restart;
            }
            if (suffix.power != IPL_INVALID) {
                if (suffix.power == IPL_SKILLLVL && suffix.param2 == MAXSPLLEVEL + 1) {
                    const ItemAffixStruct *ia = items[MAXITEMS]._iNumAffixes == 0 ? NULL : (items[MAXITEMS]._iNumAffixes == 1 ? &items[MAXITEMS]._iAffixes[0] : &items[MAXITEMS]._iAffixes[1]);
                    if (ia == NULL || ia->asPower != IPL_SKILLLVL || ia->asValue1 != suffix.param1) {
                        // LogErrorF("missed sufval %d vs %d (%d) seed%d", ia == NULL ? -1 : ia->asPower, suffix.param1, sufIdx, seed);
                        goto restart;
                    }
                } else if (affix_rnd[1] < suffix.param1 || affix_rnd[1] > suffix.param2) {
                    // LogErrorF("missed sufval %d vs [%d:%d]", affix_rnd[1], suffix.param1, suffix.param2);
                    goto restart;
                }
            }
        }
    }
    goto done;
restart:
    counter++;
    if (counter != 0) {
        seed++;// seed = NextRndSeed();
        goto start;
    }
    QMessageBox::critical(this, "Error", tr("Failed to create item with the required attributes."));
done:

    memcpy(this->is, &items[MAXITEMS], sizeof(ItemStruct));
    this->is->_iUnidentified = FALSE;
    if (counter != 0) {
        dProgress() << tr("Succeeded after %1 iterations.").arg(counter + 1);
    }
    IsMultiGame = gameMulti;
    IsHellfireGame = gameHellfire;
    return true;
}

void ItemsDialog::on_calculateButton_clicked()
{
    bool unique = this->ui->isUniqueCheckBox->isChecked();
    int lvl = this->is->_iCreateInfo & CF_LEVEL;
    int idx = this->ui->itemIdxComboBox->currentData().value<int>();
    int sn = SPL_NULL;
    if (this->ui->itemPrefixComboBox->currentData().value<int>() == AFFIX_SKILL && this->ui->itemPrefixLimitSlider->isEnabled()) {
#if 0
        int val = this->ui->itemPrefixLimitSlider->value();
        switch (AllItemList[wIdx].iMiscId) {
        case IMISC_SCROLL: sn = GetScrollSpell(lvl, val); break;
        case IMISC_RUNE:   sn = GetRuneSpell(lvl, val);   break;
        case IMISC_BOOK:
        default:           sn = GetBookSpell(lvl, val);   break;
        }
#else
        sn++;
#endif
    }

    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->ui->isHellfireCheckBox->isChecked();
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->ui->isMultiCheckBox->isChecked();

    float dropChance = this->ui->itemSourceComboBox->currentIndex() == 0 ? ItemDropChance(idx, sn, lvl, this->numPlayers, unique) : NAN;

    IsMultiGame = gameMulti;
    IsHellfireGame = gameHellfire;

    this->ui->itemChance->setText(tr("Chance: %1").arg(QString::number(dropChance, 'f')));
}

void ItemsDialog::on_generateButton_clicked()
{
    if (this->recreateItem()) {
        this->updateFields();
    }
}

void ItemsDialog::on_closeButton_clicked()
{
    this->close();
}
