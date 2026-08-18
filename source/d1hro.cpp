#include "d1hro.h"

#include <QApplication>
#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QImage>
#include <QMessageBox>

#include "dungeon/all.h"

#include "config.h"
#include "d1cel.h"
#include "d1gfx.h"
#include "progressdialog.h"

typedef struct HeroSaveStruct {
    PkPlayerStruct pps;
    quint8 isHellfire;
    quint8 isMulti;
    char itemNames[NUM_INVELEM][256];
} HeroSaveStruct;

D1Hero* D1Hero::instance()
{
    for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
        if (!plr._pActive) {
            plr._pActive = TRUE;
            D1Hero* hero = new D1Hero();
            hero->pnum = pnum;
            return hero;
        }
    }
    return nullptr;
}

bool D1Hero::isStandardClass(int hc)
{
    return hc == PC_WARRIOR || hc == PC_ROGUE || hc == PC_SORCERER;
}

D1Hero::~D1Hero()
{
    plr._pActive = FALSE;
}

bool D1Hero::load(const QString &filePath, const OpenAsParam &params)
{
    // Opening CEL file and load it in RAM
    QFile file = QFile(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QByteArray fileData = file.readAll();

    if (fileData.size() < sizeof(PkPlayerStruct))
        return false;

    this->hellfire = false;
    this->multi = false;
    const HeroSaveStruct *hss = (const HeroSaveStruct*)fileData.constData();
    if (fileData.size() == sizeof(HeroSaveStruct)) {
        this->hellfire = hss->isHellfire != 0 || !D1Hero::isStandardClass(hss->pps.pClass);
        this->multi = hss->isMulti != 0;
    } else {
        this->hellfire = !D1Hero::isStandardClass(hss->pps.pClass);
    }

    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->hellfire;
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->multi;
// LogErrorF("D1Hero::load 0");
    UnPackPlayer(&hss->pps, this->pnum);
    // plr._pDunLevel = DLV_CATHEDRAL1;
// LogErrorF("D1Hero::load 1");
    if (fileData.size() == sizeof(HeroSaveStruct)) {
        // static_assert(lengthof(hss->itemNames) >= lengthof(plr._pInvBody) + lengthof(plr._pInvList) + lengthof(plr._pSpdList), "item-name copy in D1Hero::load must be adjusted.");
        static_assert(sizeof(hss->itemNames[0]) == sizeof(plr._pInvBody[0]._iName), "memcopy in D1Hero::load must be adjusted.");
        int cursor = 0;
        for (int i = 0; i < lengthof(plr._pInvBody); i++, cursor++) {
            memcpy(plr._pInvBody[i]._iName, &hss->itemNames[cursor], sizeof(plr._pInvBody[i]._iName));
        }
        for (int i = 0; i < lengthof(plr._pInvList); i++, cursor++) {
            memcpy(plr._pInvList[i]._iName, &hss->itemNames[cursor], sizeof(plr._pInvList[i]._iName));
        }
        for (int i = 0; i < lengthof(plr._pSpdList); i++, cursor++) {
            memcpy(plr._pSpdList[i]._iName, &hss->itemNames[cursor], sizeof(plr._pSpdList[i]._iName));
        }
    }
// LogErrorF("D1Hero::load 2");
    IsHellfireGame = gameHellfire;
    IsMultiGame = gameMulti;

    this->calcInv();
// LogErrorF("D1Hero::load 3");
    this->filePath = filePath;
    this->modified = false;

    int hc = plr._pClass;
    switch (params.heroClass) {
    case OPEN_HERO_CLASS::AUTODETECT:                   break;
    case OPEN_HERO_CLASS::WARRIOR:   hc = PC_WARRIOR;   break;
    case OPEN_HERO_CLASS::ROGUE:     hc = PC_ROGUE;     break;
    case OPEN_HERO_CLASS::SORCERER:  hc = PC_SORCERER;  break;
    case OPEN_HERO_CLASS::MONK:      hc = PC_MONK;      break;
    }
    if (hc != plr._pClass) {
        this->setClass(hc);
    }
// LogErrorF("D1Hero::load 4");
    if (params.heroType != OPEN_HERO_TYPE::AUTODETECT) {
        this->setHellfire(params.heroType == OPEN_HERO_TYPE::HELLFIRE_HERO);
    }

    return true;
}

void D1Hero::create(unsigned index)
{
    _uiheroinfo selhero_heroInfo;

    //SelheroClassSelectorFocus(unsigned index)

    //selhero_heroInfo.hiIdx = MAX_CHARACTERS;
    selhero_heroInfo.hiLevel = 1;
    selhero_heroInfo.hiClass = index;
    //selhero_heroInfo.hiRank = 0;
    selhero_heroInfo.hiStrength = StrengthTbl[index];   //defaults.dsStrength;
    selhero_heroInfo.hiMagic = MagicTbl[index];         //defaults.dsMagic;
    selhero_heroInfo.hiDexterity = DexterityTbl[index]; //defaults.dsDexterity;
    selhero_heroInfo.hiVitality = VitalityTbl[index];   //defaults.dsVitality;

    switch (index) {
    case PC_WARRIOR:   selhero_heroInfo.hiBuild = {  5,  2,  3,  4 }; break;
    case PC_ROGUE:     selhero_heroInfo.hiBuild = {  2,  4,  6,  2 }; break;
    case PC_SORCERER:  selhero_heroInfo.hiBuild = {  2,  6,  3,  3 }; break;
    case PC_MONK:      selhero_heroInfo.hiBuild = {  4,  3,  4,  3 }; break;
    }

    selhero_heroInfo.hiName[0] = '\0';

    this->hellfire = !D1Hero::isStandardClass(index);
    this->multi = false;

    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->hellfire;
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->multi;

    CreatePlayer(this->pnum, selhero_heroInfo);
    InitPlayer(this->pnum);
    // plr._pDunLevel = DLV_CATHEDRAL1;

    IsMultiGame = gameMulti;
    IsHellfireGame = gameHellfire;

    this->calcInv();

    this->filePath.clear();
    this->modified = true;
}

void D1Hero::compareTo(const D1Hero *hero, QString header) const
{

}

D1Pal *D1Hero::getPalette() const
{
    return this->palette;
}

void D1Hero::setPalette(D1Pal *pal)
{
    this->palette = pal;
}

static void RecreateHeroItems(ItemStruct *is, int numItems)
{
    for (int i = 0; i < numItems; i++, is++) {
        if (is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER && is->_itype != ITYPE_GOLD && is->_iMiscId != IMISC_EAR) {
            ItemStruct tmpItem;
            memcpy(&tmpItem, is, sizeof(ItemStruct));

            RecreateItem(is->_iSeed, is->_iIdx, is->_iCreateInfo);

            items[MAXITEMS]._iUnidentified = tmpItem._iUnidentified;
            if (tmpItem._itype == items[MAXITEMS]._itype && tmpItem._iMiscId == items[MAXITEMS]._iMiscId && tmpItem._iClass == items[MAXITEMS]._iClass) {
                // preserve the name
                bool match = tmpItem._iUid == items[MAXITEMS]._iUid && tmpItem._iNumAffixes == items[MAXITEMS]._iNumAffixes;
                for (int n = 0; match && n < tmpItem._iNumAffixes; n++) {
                    match &= tmpItem._iAffixes[n].asPower == items[MAXITEMS]._iAffixes[n].asPower;
                }
                if (match)
                    copy_cstr(items[MAXITEMS]._iName, tmpItem._iName);
                // TODO: preserve stats?
                /*if (tmpItem._iMaxDur < items[MAXITEMS]._iMaxDur)
                    items[MAXITEMS]._iMaxDur = tmpItem._iMaxDur;
                if (tmpItem._iDurability < items[MAXITEMS]._iDurability)
                    items[MAXITEMS]._iDurability = tmpItem._iDurability;
                if (tmpItem._iMaxCharges < items[MAXITEMS]._iMaxCharges)
                    items[MAXITEMS]._iMaxCharges = tmpItem._iMaxCharges;
                if (tmpItem._iCharges < items[MAXITEMS]._iCharges)
                    items[MAXITEMS]._iCharges = tmpItem._iCharges;*/
            }
            memcpy(is, &items[MAXITEMS], sizeof(ItemStruct));
        }
    }
}

void D1Hero::update()
{
    // update hero-items
    auto gameHellfire = IsHellfireGame;
    IsHellfireGame = this->hellfire;
    auto gameMulti = IsMultiGame;
    IsMultiGame = this->multi;

    RecreateHeroItems(&plr._pInvBody[0], NUM_INVLOC);
    RecreateHeroItems(&plr._pSpdList[0], MAXBELTITEMS);
    RecreateHeroItems(&plr._pInvList[0], NUM_INV_GRID_ELEM);

    IsMultiGame = gameMulti;
    IsHellfireGame = gameHellfire;
    // - enforce TWOHAND/ONEHAND rules
    this->rebalance();

    // update hero-stats
    this->calcInv();
}

static QPainter *InvPainter = nullptr;
static D1Pal *InvPal = nullptr;
static BYTE light_trn_index = 0;
static bool gbCelTransparencyActive;

static void InvDrawSlotBack(int X, int Y, int W, int H)
{
    // LogErrorF("InvDrawSlotBack %d:%d %dx%d", X, Y, W, H);
    QImage *destImage = (QImage *)InvPainter->device();
    for (int y = Y - H + 1; y <= Y; y++) {
        for (int x = X; x < X + W; x++) {
            QColor color = destImage->pixelColor(x, y);
            for (int i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
                if (InvPal->getColor(i) == color) {
                    color = InvPal->getColor(i - (PAL16_GRAY - PAL16_BEIGE));
                    destImage->setPixelColor(x, y, color);
                    break;
                }
            }
            //QColor color = QColor(0, 0, 0);
            //destImage->setPixelColor(x, y, color);
        }
    }
    // LogErrorF("InvDrawSlotBack done");
}

static void CelClippedDrawOutline(BYTE col, int sx, int sy, const D1Gfx *pCelBuff, int nCel, int nWidth)
{
    // LogErrorF("CelClippedDrawOutline %d:%d idx:%d w:%d trn%d", sx, sy, nCel, nWidth, col);
    QImage *destImage = (QImage *)InvPainter->device();

    QColor color = InvPal->getColor(col);
    D1GfxFrame *item = pCelBuff->getFrame(nCel - 1);
    for (int y = sy - item->getHeight() + 1; y <= sy; y++) {
        for (int x = sx; x < sx + item->getWidth(); x++) {
            D1GfxPixel pixel = item->getPixel(x - sx, y - (sy - item->getHeight() + 1));
            if (!pixel.isTransparent()) {
                destImage->setPixelColor(x + 1, y, color);
                destImage->setPixelColor(x - 1, y, color);
                destImage->setPixelColor(x, y + 1, color);
                destImage->setPixelColor(x, y - 1, color);
            }
        }
    }
    // LogErrorF("CelClippedDrawOutline done");
}

static void CelClippedDrawLightTbl(int sx, int sy, const D1Gfx *pCelBuff, int nCel, int nWidth, BYTE trans)
{
    // LogErrorF("CelClippedDrawLightTbl %d:%d idx:%d w:%d trn%d", sx, sy, nCel, nWidth, trans);
    QImage *destImage = (QImage *)InvPainter->device();

    D1GfxFrame *item = pCelBuff->getFrame(nCel - 1);
    for (int y = sy - item->getHeight() + 1; y <= sy; y++) {
        for (int x = sx; x < sx + item->getWidth(); x++) {
            D1GfxPixel pixel = item->getPixel(x - sx, y - (sy - item->getHeight() + 1));
            if (!pixel.isTransparent()) {
                quint8 col = pixel.getPaletteIndex();
                QColor color = InvPal->getColor(ColorTrns[trans][col]);
                destImage->setPixelColor(x, y, color);
            }
        }
    }
    // LogErrorF("CelClippedDrawLightTbl done");
}

static void CelClippedDrawLightTrans(int sx, int sy, const D1Gfx *pCelBuff, int nCel, int nWidth)
{
    if (pCelBuff == nullptr || pCelBuff->getFrameCount() <= nCel) return;
    BYTE trans = light_trn_index;
    // LogErrorF("CelClippedDrawLightTrans %d:%d idx:%d w:%d trn%d", sx, sy, nCel, nWidth, trans);
    QImage *destImage = (QImage *)InvPainter->device();

    D1GfxFrame *item = pCelBuff->getFrame(nCel - 1);
    for (int y = sy - item->getHeight() + 1; y <= sy; y++) {
        for (int x = sx; x < sx + item->getWidth(); x++) {
            if ((x & 1) == (y & 1))
                continue;
            // LogErrorF("CelClippedDrawLightTrans pixel %d:%d of %dx%d to %d:%d", x - sx, y - (sy - item->getHeight() + 1), item->getWidth(), item->getHeight(), x, y);
            D1GfxPixel pixel = item->getPixel(x - sx, y - (sy - item->getHeight() + 1));
            if (!pixel.isTransparent()) {
                quint8 col = pixel.getPaletteIndex();
                QColor color = InvPal->getColor(ColorTrns[trans][col]);
                destImage->setPixelColor(x, y, color);
            }
        }
    }
    // LogErrorF("CelClippedDrawLightTrans done");
}

static void scrollrt_draw_item(const ItemStruct* is, bool outline, int sx, int sy, const D1Gfx *pCelBuff, int nCel, int nWidth)
{
	BYTE col, trans;
    // dProgressErr() << QString("scrollrt_draw_item %1:%2 idx:%3 w:%4 outline%5").arg(sx).arg(sy).arg(nCel).arg(nWidth).arg(outline);
    // LogErrorF("scrollrt_draw_item %d:%d cel%d, w%d o%d", sx, sy, nCel, nWidth, outline);
	col = ICOL_YELLOW;
	if (is->_iMagical != ITEM_QUALITY_NORMAL) {
		col = ICOL_BLUE;
	}
	if (!is->_iStatFlag) {
		col = ICOL_RED;
	}

    if (pCelBuff != nullptr && pCelBuff->getFrameCount() > nCel) {
        if (outline) {
            CelClippedDrawOutline(col, sx, sy, pCelBuff, nCel, nWidth);
        }
        trans = col != ICOL_RED ? 0 : COLOR_TRN_RED;
        CelClippedDrawLightTbl(sx, sy, pCelBuff, nCel, nWidth, trans);
    } else {
        QString text = is->_iName;
        QFontMetrics fm(InvPainter->font());
        int textWidth = fm.horizontalAdvance(text);
        if (outline) {
            col = PAL16_ORANGE + 2;
        }
        InvPainter->setPen(InvPal->getColor(col));
        InvPainter->drawText(sx + (nWidth - textWidth) / 2, sy - fm.height(), text);
        // dProgressErr() << QString("scrollrt_draw_item text %1 to %2:%3 (w:%4 col%5)").arg(text).arg(sx + (nWidth - textWidth) / 2).arg(sy - fm.height()).arg(nWidth).arg(col);
    }
}

static void draw_item_placeholder(const char* name, bool outline, int sx, int sy, const D1Gfx *pCelBuff, int nCel, int nWidth)
{
    /*BYTE col = ICOL_YELLOW;
    // LogErrorF("draw_item_placeholder %s %d:%d idx:%d w:%d", name, sx, sy, nCel, nWidth);
    if (pCelBuff != nullptr && pCelBuff->getFrameCount() > nCel) {
        if (outline) {
            CelClippedDrawOutline(col, sx, sy, pCelBuff, nCel, nWidth);
        }
        CelClippedDrawLightTbl(sx, sy, pCelBuff, nCel, nWidth, COLOR_TRN_GRAY);
    } else {
        QString text = name;
        QFontMetrics fm(InvPainter->font());
        int textWidth = fm.horizontalAdvance(text);
        InvPainter->setPen(InvPal->getColor(PAL16_GRAY));
        InvPainter->drawText(sx + (nWidth - textWidth) / 2, sy - fm.height(), text);
        // LogErrorF("draw_item_placeholder text %s to %d:%d (w:%d)", name, sx + (nWidth - textWidth) / 2, sy - fm.height(), nWidth);
    }*/
}

QImage D1Hero::getEquipmentImage(int pcursinvitem) const
{
    constexpr int INV_WIDTH = SPANEL_WIDTH;
    constexpr int INV_HEIGHT = 178;
    constexpr int INV_ROWS = SPANEL_HEIGHT - INV_HEIGHT;
    QImage result = QImage(INV_WIDTH, INV_HEIGHT, QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    /*QString folder = Config::getAssetsFolder() + "\\";
    // draw the inventory
    QString invFilePath = folder + "Data\\Inv\\Inv.CEL";
    if (QFile::exists(invFilePath)) {
        // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("File '%1' exists").arg(invFilePath));
        D1Gfx gfx;
        gfx.setPalette(this->palette);
        OpenAsParam params = OpenAsParam();
        if (D1Cel::load(gfx, invFilePath, params) && gfx.getFrameCount() != 0) {
            D1GfxFrame *inv = gfx.getFrame(0);
            // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("File '%1' loaded %2x%3").arg(invFilePath).arg(inv->getWidth()).arg(inv->getHeight()));
            int ox = (INV_WIDTH - inv->getWidth()) / 2;
            int oy = (INV_HEIGHT - (inv->getHeight() - INV_ROWS)) / 2;

            for (int y = 0; y < inv->getHeight(); y++) {
                int yy = y - oy;
                if (yy >= 0 && yy < INV_HEIGHT) {
                    for (int x = 0; x < inv->getWidth(); x++) {
                        int xx = x - ox;
                        if (xx >= 0 && xx < INV_WIDTH) {
                            D1GfxPixel pixel = inv->getPixel(x, y);
                            if (!pixel.isTransparent()) {
                                QColor color = this->palette->getColor(pixel.getPaletteIndex());
                                result.setPixelColor(xx, yy, color);
                            }
                        }
                    }
                }
            }
            // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("Inv copied %1:%2").arg(ox).arg(oy));
        } else {
            // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("Failed to load CEL file: %1").arg(invFilePath));
        }
    } else {
        // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("Failed to load inv %1").arg(invFilePath));
    }*/
    // LogErrorF("D1Hero::getEquipmentImage 0 %d", pInvCels != nullptr);
    // draw the inventory
    D1Gfx *iCels = pInvCels;
    if (iCels != nullptr) {
        // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("File '%1' exists").arg(invFilePath));
            D1GfxFrame *inv = iCels->getFrame(0);
            // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("File '%1' loaded %2x%3").arg(invFilePath).arg(inv->getWidth()).arg(inv->getHeight()));
            int ox = (INV_WIDTH - inv->getWidth()) / 2;
            int oy = (INV_HEIGHT - (inv->getHeight() - INV_ROWS)) / 2;

            for (int y = 0; y < inv->getHeight(); y++) {
                int yy = y - oy;
                if (yy >= 0 && yy < INV_HEIGHT) {
                    for (int x = 0; x < inv->getWidth(); x++) {
                        int xx = x - ox;
                        if (xx >= 0 && xx < INV_WIDTH) {
                            D1GfxPixel pixel = inv->getPixel(x, y);
                            if (!pixel.isTransparent()) {
                                QColor color = this->palette->getColor(pixel.getPaletteIndex());
                                result.setPixelColor(xx, yy, color);
                            }
                        }
                    }
                }
            }
    }
    // LogErrorF("D1Hero::getEquipmentImage 1");
    // draw the items
    // QString objFilePath = folder + "Data\\Inv\\Objcurs.CEL";
    QPainter invPainter(&result);
    invPainter.setPen(QColor(Config::getPaletteUndefinedColor()));

    InvPainter = &invPainter;
    InvPal = this->palette;

    D1Gfx *cCels = pCursCels;
    /*if (QFile::exists(objFilePath)) {
        // dProgressErr() << QString("File '%1' exists").arg(objFilePath);
        cCels = new D1Gfx();
        cCels->setPalette(this->palette);
        OpenAsParam params = OpenAsParam();
        if (!D1Cel::load(*cCels, objFilePath, params)) {
            // dProgressErr() << QString("Failed to load CEL file: %1").arg(objFilePath);
            delete cCels;
            cCels = nullptr;
        } else {
            // dProgressErr() << QString("File '%1' loaded.").arg(objFilePath);
        }
    } else {
        // QMessageBox::critical(nullptr, QApplication::tr("Error"), QApplication::tr("Failed to load obj %1").arg(objFilePath));
    }*/
    // DrawInv
	ItemStruct *is, *pi;
	int frame, frame_width, screen_x, screen_y, dx, dy;

	screen_x = 0;
	screen_y = 0;

    pi = PlrItem(pnum, pcursinvitem); // FIXME /*pcursinvitem == ITEM_NONE ? NULL :*/ PlrItem(pnum, pcursinvitem);
    // LogErrorF("D1Hero::getEquipmentImage 2 %d", INVLOC_HEAD);
	is = &plr._pInvBody[INVLOC_HEAD];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HEAD_FIRST].X, screen_y + InvRect[SLOTXY_HEAD_LAST].Y, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HEAD_FIRST].X, screen_y + InvRect[SLOTXY_HEAD_LAST].Y, cCels, frame, frame_width);
	} else {
		frame = ICURS_HELM + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

        draw_item_placeholder("helm", pi == is, screen_x + InvRect[SLOTXY_HEAD_FIRST].X, screen_y + InvRect[SLOTXY_HEAD_LAST].Y, cCels, frame, frame_width);
    }
    // LogErrorF("D1Hero::getEquipmentImage 3 %d", INVLOC_RING_LEFT);
	is = &plr._pInvBody[INVLOC_RING_LEFT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_RING_LEFT].X, screen_y + InvRect[SLOTXY_RING_LEFT].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_RING_LEFT].X, screen_y + InvRect[SLOTXY_RING_LEFT].Y, cCels, frame, frame_width);
	} else {
		frame = ICURS_RING + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

        draw_item_placeholder("left ring", pi == is, screen_x + InvRect[SLOTXY_RING_LEFT].X, screen_y + InvRect[SLOTXY_RING_LEFT].Y, cCels, frame, frame_width);
	}
    // LogErrorF("D1Hero::getEquipmentImage 4 %d", INVLOC_RING_RIGHT);
	is = &plr._pInvBody[INVLOC_RING_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_RING_RIGHT].X, screen_y + InvRect[SLOTXY_RING_RIGHT].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_RING_RIGHT].X, screen_y + InvRect[SLOTXY_RING_RIGHT].Y, cCels, frame, frame_width);
	} else {
		frame = ICURS_RING + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

        draw_item_placeholder("right ring", pi == is, screen_x + InvRect[SLOTXY_RING_RIGHT].X, screen_y + InvRect[SLOTXY_RING_RIGHT].Y, cCels, frame, frame_width);
	}
    // LogErrorF("D1Hero::getEquipmentImage 5 %d", INVLOC_AMULET);
	is = &plr._pInvBody[INVLOC_AMULET];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_AMULET].X, screen_y + InvRect[SLOTXY_AMULET].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_AMULET].X, screen_y + InvRect[SLOTXY_AMULET].Y, cCels, frame, frame_width);
	} else {
		frame = ICURS_AMULET + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

        draw_item_placeholder("amulet", pi == is, screen_x + InvRect[SLOTXY_AMULET].X, screen_y + InvRect[SLOTXY_AMULET].Y, cCels, frame, frame_width);
	}
    // LogErrorF("D1Hero::getEquipmentImage 6 %d", INVLOC_HAND_LEFT);
	is = &plr._pInvBody[INVLOC_HAND_LEFT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_LEFT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_LEFT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		dx = 0;
		dy = 0;
		if (frame_width == INV_SLOT_SIZE_PX)
			dx += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != (3 * INV_SLOT_SIZE_PX))
			dy -= INV_SLOT_SIZE_PX / 2;

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HAND_LEFT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_LEFT_LAST].Y + dy, cCels, frame, frame_width);

		if (TWOHAND_WIELD(&plr, is)) {
				InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
				light_trn_index = 0;
				gbCelTransparencyActive = true;

				dx = 0;
				dy = 0;
				if (frame_width == INV_SLOT_SIZE_PX)
					dx += INV_SLOT_SIZE_PX / 2;
				if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
					dy -= INV_SLOT_SIZE_PX / 2;
				CelClippedDrawLightTrans(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y + dy, cCels, frame, frame_width);
		}
	} else {
        frame = 0;
        switch (plr._pClass) {
        case PC_WARRIOR:   frame = ICURS_SHORT_SWORD; break;
        case PC_ROGUE:     frame = ICURS_SHORT_BOW;   break;
        case PC_SORCERER:  frame = ICURS_SHORT_STAFF; break;
        case PC_MONK:      frame = ICURS_SHORT_STAFF; break;
        }
		frame += CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		dx = 0;
		dy = 0;
		if (frame_width == INV_SLOT_SIZE_PX)
			dx += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != (3 * INV_SLOT_SIZE_PX))
			dy -= INV_SLOT_SIZE_PX / 2;

        draw_item_placeholder("weapon", pi == is, screen_x + InvRect[SLOTXY_HAND_LEFT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_LEFT_LAST].Y + dy, cCels, frame, frame_width);
	}
    // LogErrorF("D1Hero::getEquipmentImage 7 %d", INVLOC_HAND_RIGHT);
	is = &plr._pInvBody[INVLOC_HAND_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		dx = 0;
		dy = 0;
		if (frame_width == INV_SLOT_SIZE_PX)
			dx += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
			dy -= INV_SLOT_SIZE_PX / 2;

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y + dy, cCels, frame, frame_width);
	} else {
        is = &plr._pInvBody[INVLOC_HAND_LEFT];
        if ((is->_itype == ITYPE_NONE && (plr._pClass == PC_WARRIOR)) || !TWOHAND_WIELD(&plr, is)) {
            frame = ICURS_SMALL_SHIELD + CURSOR_FIRSTITEM;
            frame_width = InvItemWidth[frame];

            draw_item_placeholder("shield", pi == is, screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y, cCels, frame, frame_width);
        }
	}
    // LogErrorF("D1Hero::getEquipmentImage 8 %d", INVLOC_CHEST);
	is = &plr._pInvBody[INVLOC_CHEST];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_CHEST_FIRST].X, screen_y + InvRect[SLOTXY_CHEST_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_CHEST_FIRST].X, screen_y + InvRect[SLOTXY_CHEST_LAST].Y, cCels, frame, frame_width);
	} else {
		frame = ICURS_QUILTED_ARMOR + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

        draw_item_placeholder("armor", pi == is, screen_x + InvRect[SLOTXY_CHEST_FIRST].X, screen_y + InvRect[SLOTXY_CHEST_LAST].Y, cCels, frame, frame_width);
	}

    invPainter.end();

    return result;
}

const ItemStruct *D1Hero::item(int ii) const
{
    return PlrItem(this->pnum, ii);
}

bool D1Hero::addInvItem(int dst_ii, ItemStruct *is)
{
    // LogErrorF("D1Hero::addInvItem %d type %d", dst_ii, is->_itype);
    if (is->_itype == ITYPE_NONE) {
        return this->swapInvItem(dst_ii, INVITEM_NONE);
    }
    ItemStruct *pi;
    if (dst_ii < NUM_INVLOC) {
        pi = PlrItem(this->pnum, dst_ii);
        if (memcmp(pi, is, sizeof(ItemStruct)) == 0)
            return false;
        if (is->_itype != ITYPE_NONE) {
            if (TWOHAND_WIELD(&plr, is)) {
                if (dst_ii == INVLOC_HAND_LEFT) {
                    ItemStruct *ri = PlrItem(this->pnum, INVLOC_HAND_RIGHT);
                    if (ri->_itype != ITYPE_NONE) {
                        // non-empty right + new two handed weapon -> move the right item to the inventory
                        this->swapInvItem(INVLOC_HAND_RIGHT, INVITEM_NONE);
                    }
                } else /*if (dst_ii == INVLOC_HAND_RIGHT) */{
                    if (pi->_itype != ITYPE_NONE) {
                        // non-empty right + new two handed weapon -> move the right item to the inventory
                        this->swapInvItem(INVLOC_HAND_RIGHT, INVITEM_NONE);
                    }
                    // new two handed weapon to the right -> force to the left hand
                    pi = PlrItem(this->pnum, INVLOC_HAND_LEFT);
                }
            } else {
                if (dst_ii == INVLOC_HAND_LEFT) {
                    ItemStruct *ri = PlrItem(this->pnum, INVLOC_HAND_RIGHT);
                    if (is->_iClass != ICLASS_WEAPON) {
                        // new shield -> force to the right hand
                        pi = ri;
                    }
                } else if (dst_ii == INVLOC_HAND_RIGHT) {
                    ItemStruct *li = PlrItem(this->pnum, INVLOC_HAND_LEFT);
                    if (li->_itype != ITYPE_NONE && TWOHAND_WIELD(&plr, li)) {
                        // two handed + new weapon -> move the two handed to the inventory
                        this->swapInvItem(INVLOC_HAND_LEFT, INVITEM_NONE);
                    } else if (li->_itype == ITYPE_NONE && pi->_iClass == ICLASS_WEAPON) {
                        // empty left + new weapon -> force to the left hand
                        pi = li;
                    }
                }
            }
        }
    }  else {
        for (int ii = INVITEM_INV_FIRST; ii < INVITEM_INV_LAST; ii++) {
            pi = &plr._pInvList[ii];
            if (memcmp(pi, is, sizeof(ItemStruct)) == 0) {
                return false;
            }
        }
        for (int ii = INVITEM_INV_FIRST; ii < INVITEM_INV_LAST; ii++) {
            pi = &plr._pInvList[ii];
            if (pi->_itype == ITYPE_NONE) {
                break;
            }
        }
    }
    if (pi->_itype != ITYPE_NONE) {
        for (int ii = INVITEM_INV_FIRST; ii < INVITEM_INV_LAST; ii++) {
            ItemStruct *ci = &plr._pInvList[ii];
            if (ci->_itype == ITYPE_NONE) {
                memcpy(ci, pi, sizeof(ItemStruct));
                break;
            }
        }
    }
    memcpy(pi, is, sizeof(ItemStruct));
    this->calcInv();
    this->modified = true;
    // LogErrorF("D1Hero::addInvItem done");
    return true;
}

bool D1Hero::swapInvItem(int dst_ii, int src_ii)
{
    bool result = SwapPlrItem(this->pnum, dst_ii, src_ii);
    if (result) {
        this->calcInv();
        this->modified = true;
    }
    return result;
}

void D1Hero::renameItem(int ii, QString &name)
{
    ItemStruct *pi = PlrItem(this->pnum, ii);

    int len = name.length();
    if (len > lengthof(pi->_iName) - 1)
        len = lengthof(pi->_iName) - 1;
    // LogErrorF("D1Hero::renameItem '%s' -> '%s' (%d)", pi->_iName, text.toLatin1().constData(), len);
    memcpy(pi->_iName, name.toLatin1().constData(), len);

    pi->_iName[len] = '\0';

    this->modified = true;
}

QString D1Hero::getFilePath() const
{
    return this->filePath;
}

void D1Hero::setFilePath(const QString &filePath)
{
    this->filePath = filePath;
    this->modified = true;
}

bool D1Hero::isModified() const
{
    return this->modified;
}

void D1Hero::setModified(bool modified)
{
    this->modified = modified;
}

bool D1Hero::isHellfire() const
{
    return this->hellfire;
}

void D1Hero::setHellfire(bool hf)
{
    if (this->hellfire != hf) {
        this->hellfire = hf;
        this->modified = true;

        this->update();
    }
}

bool D1Hero::isMulti() const
{
    return this->multi;
}

void D1Hero::setMulti(bool ml)
{
    if (this->multi != ml) {
        this->multi = ml;
        this->modified = true;

        this->update();
    }
}

const char* D1Hero::getName() const
{
    return players[this->pnum]._pName;
}

void D1Hero::setName(const QString &name)
{
    QString currName = QString(players[this->pnum]._pName);
    if (currName == name)
        return;

    int len = name.length();
    if (len > lengthof(players[this->pnum]._pName) - 1)
        len = lengthof(players[this->pnum]._pName) - 1;

    memcpy(players[this->pnum]._pName, name.toLatin1().constData(), len);

    players[this->pnum]._pName[len] = '\0';

    this->modified = true;
}

int D1Hero::getClass() const
{
    return players[this->pnum]._pClass;
}

void D1Hero::setClass(int cls)
{
    if (players[this->pnum]._pClass == cls)
        return;
    players[this->pnum]._pClass = cls;
    if (!D1Hero::isStandardClass(cls)) {
        this->setHellfire(true);
    }

    this->rebalance();

    this->calcInv();

    this->modified = true;
}

int D1Hero::getLevel() const
{
    return players[this->pnum]._pLevel;
}

int D1Hero::getStatPoints() const
{
    return players[this->pnum]._pStatPts;
}

void D1Hero::setLevel(int level)
{
    int dlvl;
    if (level < 1)
        level = 1;
    if (level > MAXCHARLEVEL)
        level = MAXCHARLEVEL;
    dlvl = level - players[this->pnum]._pLevel;
    if (dlvl == 0)
        return;
    players[this->pnum]._pLevel = level;
    if (dlvl > 0) {
        players[this->pnum]._pStatPts += 4 * dlvl;
    } else {
        this->rebalance();
    }
    players[this->pnum]._pExperience = PlrExpLvlsTbl[level - 1];

    this->calcInv(); // update item-stats, damage, etc...

    this->modified = true;
}

int D1Hero::getRank() const
{
    return players[this->pnum]._pRank;
}

void D1Hero::setRank(int rank)
{
    if (players[this->pnum]._pRank == rank)
        return;
    players[this->pnum]._pRank = rank;
    this->modified = true;
}

int D1Hero::getBtStr() const
{
    return players[this->pnum]._pBuildType._pbStr;
}

void D1Hero::setBtStr(int v)
{
    if (players[this->pnum]._pBuildType._pbStr == v)
        return;
    players[this->pnum]._pBuildType._pbStr = v;

    this->rebalance();

    this->calcInv();

    this->modified = true;
}

int D1Hero::getBtMag() const
{
    return players[this->pnum]._pBuildType._pbMag;
}

void D1Hero::setBtMag(int v)
{
    if (players[this->pnum]._pBuildType._pbMag == v)
        return;
    players[this->pnum]._pBuildType._pbMag = v;

    this->rebalance();

    this->calcInv();

    this->modified = true;
}

int D1Hero::getBtDex() const
{
    return players[this->pnum]._pBuildType._pbDex;
}

void D1Hero::setBtDex(int v)
{
    if (players[this->pnum]._pBuildType._pbDex == v)
        return;
    players[this->pnum]._pBuildType._pbDex = v;

    this->rebalance();

    this->calcInv();

    this->modified = true;
}
int D1Hero::getBtVit() const
{
    return players[this->pnum]._pBuildType._pbVit;
}

void D1Hero::setBtVit(int v)
{
    if (players[this->pnum]._pBuildType._pbVit == v)
        return;
    players[this->pnum]._pBuildType._pbVit = v;

    this->rebalance();

    this->calcInv();

    this->modified = true;
}

int D1Hero::getStrength() const
{
    return players[this->pnum]._pStrength;
}

void D1Hero::setStrength(int value)
{
    value += players[this->pnum]._pBaseStr - players[this->pnum]._pStrength;

    if (value < StrengthTbl[players[this->pnum]._pClass])
        value = StrengthTbl[players[this->pnum]._pClass];

    while (value > players[this->pnum]._pBaseStr && players[this->pnum]._pStatPts > 0) {
        this->addStrength();
    }
    while (value < players[this->pnum]._pBaseStr) {
        this->subStrength();
    }
    this->calcInv(); // update item-stats
}

int D1Hero::getBaseStrength() const
{
    return players[this->pnum]._pBaseStr;
}

void D1Hero::addStrength()
{
    IncreasePlrStr(this->pnum);
    this->modified = true;
}

void D1Hero::subStrength()
{
    DecreasePlrStr(this->pnum);
    this->modified = true;
}

int D1Hero::getDexterity() const
{
    return players[this->pnum]._pDexterity;
}

void D1Hero::setDexterity(int value)
{
    value += players[this->pnum]._pBaseDex - players[this->pnum]._pDexterity;

    if (value < DexterityTbl[players[this->pnum]._pClass])
        value = DexterityTbl[players[this->pnum]._pClass];

    while (value > players[this->pnum]._pBaseDex && players[this->pnum]._pStatPts > 0) {
        this->addDexterity();
    }
    while (value < players[this->pnum]._pBaseDex) {
        this->subDexterity();
    }
    this->calcInv(); // update item-stats
}

int D1Hero::getBaseDexterity() const
{
    return players[this->pnum]._pBaseDex;
}

void D1Hero::addDexterity()
{
    IncreasePlrDex(this->pnum);
    this->modified = true;
}

void D1Hero::subDexterity()
{
    DecreasePlrDex(this->pnum);
    this->modified = true;
}

int D1Hero::getMagic() const
{
    return players[this->pnum]._pMagic;
}

void D1Hero::setMagic(int value)
{
    value += players[this->pnum]._pBaseMag - players[this->pnum]._pMagic;

    if (value < MagicTbl[players[this->pnum]._pClass])
        value = MagicTbl[players[this->pnum]._pClass];

    while (value > players[this->pnum]._pBaseMag && players[this->pnum]._pStatPts > 0) {
        this->addMagic();
    }
    while (value < players[this->pnum]._pBaseMag) {
        this->subMagic();
    }
    this->calcInv(); // update item-stats
}

int D1Hero::getBaseMagic() const
{
    return players[this->pnum]._pBaseMag;
}

void D1Hero::addMagic()
{
    IncreasePlrMag(this->pnum);
    this->modified = true;
}

void D1Hero::subMagic()
{
    DecreasePlrMag(this->pnum);
    this->modified = true;
}

int D1Hero::getVitality() const
{
    return players[this->pnum]._pVitality;
}

void D1Hero::setVitality(int value)
{
    value += players[this->pnum]._pBaseVit - players[this->pnum]._pVitality;

    if (value < VitalityTbl[players[this->pnum]._pClass])
        value = VitalityTbl[players[this->pnum]._pClass];

    while (value > players[this->pnum]._pBaseVit && players[this->pnum]._pStatPts > 0) {
        this->addVitality();
    }
    while (value < players[this->pnum]._pBaseVit) {
        this->subVitality();
    }
    this->calcInv(); // update damage
}

int D1Hero::getBaseVitality() const
{
    return players[this->pnum]._pBaseVit;
}

void D1Hero::addVitality()
{
    IncreasePlrVit(this->pnum);
    this->modified = true;
}

void D1Hero::subVitality()
{
    DecreasePlrVit(this->pnum);
    this->modified = true;
}

int D1Hero::getLife() const
{
    return players[this->pnum]._pMaxHP >> 6;
}

int D1Hero::getBaseLife() const
{
    return players[this->pnum]._pMaxHPBase >> 6;
}

void D1Hero::decLife()
{
    DecreasePlrMaxHp(this->pnum);
}

void D1Hero::restoreLife()
{
    RestorePlrHpVit(this->pnum);
}

int D1Hero::getMana() const
{
    return players[this->pnum]._pMaxMana >> 6;
}

int D1Hero::getBaseMana() const
{
    return players[this->pnum]._pMaxManaBase >> 6;
}

int D1Hero::getMagicResist() const
{
    return players[this->pnum]._pMagResist;
}

int D1Hero::getFireResist() const
{
    return players[this->pnum]._pFireResist;
}

int D1Hero::getLightningResist() const
{
    return players[this->pnum]._pLghtResist;
}

int D1Hero::getAcidResist() const
{
    return players[this->pnum]._pAcidResist;
}

int D1Hero::getSkillLvl(int sn) const
{
    return players[this->pnum]._pSkillLvl[sn];
}

int D1Hero::getSkillLvlBase(int sn) const
{
    return players[this->pnum]._pSkillLvlBase[sn];
}

void D1Hero::setSkillLvlBase(int sn, int level)
{
    if (players[this->pnum]._pSkillLvlBase[sn] == level)
        return;

    players[this->pnum]._pSkillLvlBase[sn] = level;
    this->calcInv(); // update _pSkillLvl
    this->modified = true;
}

uint64_t D1Hero::getFixedSkills() const
{
    return SPELL_MASK(plrAbility) | plr._pISpells;
}

uint64_t D1Hero::getSkills() const
{
    return getFixedSkills() | plr._pMemSkills;
}

int D1Hero::getSkillSources(int sn) const
{
    int result = 0;
    if (SPELL_MASK(plrAbility) & SPELL_MASK(sn))
        result |= 1 << RSPLTYPE_ABILITY;
    if (plr._pMemSkills & SPELL_MASK(sn))
        result |= 1 << RSPLTYPE_SPELL;
    if (plr._pISpells & SPELL_MASK(sn))
        result |= 1 << RSPLTYPE_CHARGES;
    return result;
}

int D1Hero::getWalkSpeed() const
{
    return players[this->pnum]._pIWalkSpeed;
}

int D1Hero::getWalkSpeedInTicks() const
{
    return GetWalkSpeedInTicks(this->pnum);
}

int D1Hero::getChargeSpeed() const
{
    return GetChargeSpeed(this->pnum);
}

int D1Hero::getBaseAttackSpeed() const
{
    return players[this->pnum]._pIBaseAttackSpeed;
}

int D1Hero::getAttackSpeedInTicks(int sn) const
{
    return GetAttackSpeedInTicks(this->pnum, sn);
}

int D1Hero::getBaseCastSpeed() const
{
    return players[this->pnum]._pIBaseCastSpeed;
}

int D1Hero::getCastSpeedInTicks(int sn) const
{
    return GetCastSpeedInTicks(this->pnum, sn);
}

int D1Hero::getRecoverySpeed() const
{
    return players[this->pnum]._pIRecoverySpeed;
}

int D1Hero::getRecoverySpeedInTicks() const
{
    return GetRecoverySpeedInTicks(this->pnum);
}

int D1Hero::getLightRad() const
{
    return players[this->pnum]._pLightRad;
}

int D1Hero::getEvasion() const
{
    return players[this->pnum]._pIEvasion;
}

int D1Hero::getAC() const
{
    return players[this->pnum]._pIAC;
}

int D1Hero::getPower() const
{
    return players[this->pnum]._pIPower;
}

int D1Hero::getBlockChance() const
{
    return players[this->pnum]._pIBlockChance;
}

int D1Hero::getAbsAnyHit() const
{
    return players[this->pnum]._pIAbsAnyHit >> 6;
}

int D1Hero::getAbsPhyHit() const
{
    return players[this->pnum]._pIAbsPhyHit >> 6;
}

int D1Hero::getLifeSteal() const
{
    return players[this->pnum]._pILifeSteal;
}

int D1Hero::getManaSteal() const
{
    return players[this->pnum]._pIManaSteal;
}

int D1Hero::getHitChance() const
{
    return players[this->pnum]._pIHitChance; // _pIBaseHitBonus
}

int D1Hero::getCritChance() const
{
    return players[this->pnum]._pICritChance;
}

int D1Hero::getSlMinDam() const
{
    return players[this->pnum]._pISlMinDam >> (6 + 1);
}

int D1Hero::getSlMaxDam() const
{
    return players[this->pnum]._pISlMaxDam >> (6 + 1);
}

int D1Hero::getBlMinDam() const
{
    return players[this->pnum]._pIBlMinDam >> (6 + 1);
}

int D1Hero::getBlMaxDam() const
{
    return players[this->pnum]._pIBlMaxDam >> (6 + 1);
}

int D1Hero::getPcMinDam() const
{
    return players[this->pnum]._pIPcMinDam >> (6 + 1);
}

int D1Hero::getPcMaxDam() const
{
    return players[this->pnum]._pIPcMaxDam >> (6 + 1);
}

int D1Hero::getChMinDam() const
{
    return players[this->pnum]._pIChMinDam >> 6;
}

int D1Hero::getChMaxDam() const
{
    return players[this->pnum]._pIChMaxDam >> 6;
}

int D1Hero::getFMinDam() const
{
    return players[this->pnum]._pIFMinDam >> 6;
}

int D1Hero::getFMaxDam() const
{
    return players[this->pnum]._pIFMaxDam >> 6;
}

int D1Hero::getLMinDam() const
{
    return players[this->pnum]._pILMinDam >> 6;
}

int D1Hero::getLMaxDam() const
{
    return players[this->pnum]._pILMaxDam >> 6;
}

int D1Hero::getMMinDam() const
{
    return players[this->pnum]._pIMMinDam >> 6;
}

int D1Hero::getMMaxDam() const
{
    return players[this->pnum]._pIMMaxDam >> 6;
}

int D1Hero::getAMinDam() const
{
    return players[this->pnum]._pIAMinDam >> 6;
}

int D1Hero::getAMaxDam() const
{
    return players[this->pnum]._pIAMaxDam >> 6;
}

int D1Hero::getTotalMinDam() const
{
    int mindam = 0;
    PlayerStruct *p = &players[this->pnum];
	mindam += (p->_pIFMinDam + p->_pILMinDam + p->_pIMMinDam + p->_pIAMinDam) >> 6;
	mindam += (p->_pISlMinDam + p->_pIBlMinDam + p->_pIPcMinDam) >> (6 + 1); // +1 is a temporary(?) adjustment for backwards compatibility
    return mindam;
}

int D1Hero::getTotalMaxDam() const
{
    int maxdam = 0;
    PlayerStruct *p = &players[this->pnum];
	maxdam += (p->_pIFMaxDam + p->_pILMaxDam + p->_pIMMaxDam + p->_pIAMaxDam) >> 6;
	maxdam += (p->_pISlMaxDam + p->_pIBlMaxDam + p->_pIPcMaxDam) >> (6 + 1);
    return maxdam;
}

void D1Hero::getMonDamage(int sn, int sl, const MonsterStruct *mon, int *mindam, int *maxdam) const
{
    GetMonByPlrDamage(this->pnum, sn, sl, mon, mindam, maxdam);
}

void D1Hero::getPlrDamage(int sn, int sl, const D1Hero *target, int *mindam, int *maxdam) const
{
    GetPlrByPlrDamage(this->pnum, sn, sl, target->pnum, mindam, maxdam);
}

void D1Hero::getMonSkillDamage(int sn, int sl, int dist, const MonsterStruct *mon, int *mindam, int *maxdam) const
{
    SkillMonByPlrDamage(sn, sl, dist, this->pnum, mon, mindam, maxdam);
}

void D1Hero::getPlrSkillDamage(int sn, int sl, int dist, const D1Hero *target, int *mindam, int *maxdam) const
{
    SkillPlrByPlrDamage(sn, sl, dist, this->pnum, target->pnum, mindam, maxdam);
}

int D1Hero::calcPlrDam(unsigned mRes, unsigned damage) const
{
    return CalcPlrDam(this->pnum, mRes, damage);
}

int D1Hero::getSkillFlags() const
{
    return players[this->pnum]._pSkillFlags;
}

int D1Hero::getItemFlags() const
{
    return players[this->pnum]._pIFlags;
}

void D1Hero::rebalance()
{
    // validate the player ?
    if (plr._pClass >= NUM_CLASSES)
        plr._pClass = PC_WARRIOR;
    if (plr._pLevel < 1)
        plr._pLevel = 1;
    if (plr._pLevel > MAXCHARLEVEL)
        plr._pLevel = MAXCHARLEVEL;
    // if (plr._pStatPts < 0)
    //    plr._pStatPts = 0;

    int pc, dv, v;
    pc = plr._pClass;
    // validate the strength
    dv = plr._pBaseStr - StrengthTbl[pc];
    if (dv < 0)
        dv = 0;
    dv *= 2;
    v = plr._pBuildType._pbStr;
    if (v > 6) {
        v = plr._pBuildType._pbStr = 6;
    }
    if (v == 0)
        dv = 0;
    else if (dv % v != 0) {
        dv++; // value was rounded down last time -> increment it to round up now
        dv -= dv % v;
    }
    v = StrengthTbl[pc] + dv / 2u;
    plr._pBaseStr = v;

    // validate the magic
    dv = plr._pBaseMag - MagicTbl[pc];
    if (dv < 0)
        dv = 0;
    dv *= 2;
    v = plr._pBuildType._pbMag;
    if (v > 6) {
        v = plr._pBuildType._pbMag = 6;
    }
    if (v == 0)
        dv = 0;
    else if (dv % v != 0) {
        dv++; // value was rounded down last time -> increment it to round up now
        dv -= dv % v;
    }
    v = MagicTbl[pc] + dv / 2u;
    plr._pBaseMag = v;

    // validate the dexterity
    dv = plr._pBaseDex - DexterityTbl[pc];
    if (dv < 0)
        dv = 0;
    dv *= 2;
    v = plr._pBuildType._pbDex;
    if (v > 6) {
        v = plr._pBuildType._pbDex = 6;
    }
    if (v == 0)
        dv = 0;
    else if (dv % v != 0) {
        dv++; // value was rounded down last time -> increment it to round up now
        dv -= dv % v;
    }
    v = DexterityTbl[pc] + dv / 2u;
    plr._pBaseDex = v;

    // validate the vitality
    dv = plr._pBaseVit - VitalityTbl[pc];
    if (dv < 0)
        dv = 0;
    dv *= 2;
    v = plr._pBuildType._pbVit;
    if (v > 6) {
        v = plr._pBuildType._pbVit = 6;
    }
    if (v == 0)
        dv = 0;
    else if (dv % v != 0) {
        dv++; // value was rounded down last time -> increment it to round up now
        dv -= dv % v;
    }
    v = VitalityTbl[pc] + dv / 2u;
    plr._pBaseVit = v;

    // if (change) {
        // RestorePlrHpVit
        plr._pMaxHPBase = plr._pBaseVit << (6 + 1);
        // RestorePlrHpMana
        plr._pMaxManaBase = plr._pBaseMag << (6 + 1);
    // }

    while (true) {
        int remStatPts = (plr._pLevel - 1) * 4;

        if (plr._pStatPts > remStatPts) {
            plr._pStatPts = remStatPts;
        }
        remStatPts -= plr._pStatPts;

        // LogErrorF("PlrStats: [str%d mag%d dex%d vit%d] vs base[str%d mag%d dex%d vit%d] (bonus%d)", plr._pBaseStr, plr._pBaseMag, plr._pBaseDex, plr._pBaseVit,
        //     StrengthTbl[plr._pClass], MagicTbl[plr._pClass], DexterityTbl[plr._pClass], VitalityTbl[plr._pClass], plr._pStatPts);

        int usedStatPts[4];
        // calculate the points used for strength
        dv = plr._pBaseStr - StrengthTbl[pc];
        dv *= 2;
        v = plr._pBuildType._pbStr;
        if (v == 0)
            dv = 0;
        else {
            if (dv % v != 0)
                dv++; // value was rounded down last time -> increment it to round up now
            dv = dv / v;
        }
        usedStatPts[0] = dv;
        // calculate the points used for magic
        dv = plr._pBaseMag - MagicTbl[pc];
        dv *= 2;
        v = plr._pBuildType._pbMag;
        if (v == 0)
            dv = 0;
        else {
            if (dv % v != 0)
                dv++; // value was rounded down last time -> increment it to round up now
            dv = dv / v;
        }
        usedStatPts[1] = dv;
        // calculate the points used for dexterity
        dv = plr._pBaseDex - DexterityTbl[pc];
        dv *= 2;
        v = plr._pBuildType._pbDex;
        if (v == 0)
            dv = 0;
        else {
            if (dv % v != 0)
                dv++; // value was rounded down last time -> increment it to round up now
            dv = dv / v;
        }
        usedStatPts[2] = dv;
        // calculate the points used for vitality
        dv = plr._pBaseVit - VitalityTbl[pc];
        dv *= 2;
        v = plr._pBuildType._pbVit;
        if (v == 0)
            dv = 0;
        else {
            if (dv % v != 0)
                dv++; // value was rounded down last time -> increment it to round up now
            dv = dv / v;
        }
        usedStatPts[3] = dv;

        int totalUsedStatPts = usedStatPts[0] + usedStatPts[1] + usedStatPts[2] + usedStatPts[3];
        // LogErrorF("UsedStats: %d [str%d mag%d dex%d vit%d] vs %d (bonus%d)", totalUsedStatPts, usedStatPts[0], usedStatPts[1], usedStatPts[2], usedStatPts[3], remStatPts, plr._pStatPts);
        if (totalUsedStatPts <= remStatPts) {
            plr._pStatPts += remStatPts - totalUsedStatPts;
            break;
        }

        int mostUsedIdx = 0;
        int mostUsedVal = usedStatPts[0];
        for (int i = 1; i < 4; i++) {
            if (usedStatPts[i] > mostUsedVal) {
                mostUsedIdx = i;
                mostUsedVal = usedStatPts[i];
            }
        }
        switch (mostUsedIdx) {
        case 0: DecreasePlrStr(pnum); break;
        case 1: DecreasePlrMag(pnum); break;
        case 2: DecreasePlrDex(pnum); break;
        case 3: DecreasePlrVit(pnum); break;
        }
        plr._pStatPts--;
        // LogErrorF("mostUsedIdx: %d -> (bonus%d)", mostUsedIdx, plr._pStatPts);
    }
    // enforce TWOHAND/ONEHAND rules
    ItemStruct *ri = PlrItem(this->pnum, INVLOC_HAND_RIGHT);
    ItemStruct *li = PlrItem(this->pnum, INVLOC_HAND_LEFT);
    if (ri->_itype != ITYPE_NONE) {
        if (TWOHAND_WIELD(&plr, ri)) {
            this->swapInvItem(INVLOC_HAND_RIGHT, INVITEM_NONE);
        } else {
            if (li->_itype != ITYPE_NONE) {
                if (TWOHAND_WIELD(&plr, li)) {
                    // twohand left + non-empty right -> move right to the inventory
                    this->swapInvItem(INVLOC_HAND_RIGHT, INVITEM_NONE);
                } else if (li->_iClass != ICLASS_WEAPON) {
                    if (ri->_iClass != ICLASS_WEAPON) {
                        // shield left + shield right -> move left to the inventory
                        this->swapInvItem(INVLOC_HAND_LEFT, INVITEM_NONE);
                    } else {
                        // shield left + weapon right -> swap hands
                        this->swapInvItem(INVLOC_HAND_RIGHT, INVLOC_HAND_LEFT);
                    }
                }
            } else {
                if (ri->_iClass == ICLASS_WEAPON) {
                    // empty left + weapon right -> move the weapon to the left
                    this->swapInvItem(INVLOC_HAND_RIGHT, INVLOC_HAND_LEFT);
                }
            }
        }
    } else {
        if (li->_itype != ITYPE_NONE && li->_iClass != ICLASS_WEAPON) {
            // shield left + empty right -> move shield to the right hand
            this->swapInvItem(INVLOC_HAND_RIGHT, INVLOC_HAND_LEFT);
        }
    }

    // this->calcInv(); ?
}

void D1Hero::calcInv()
{
    CalcPlrInv(this->pnum, false);
}

bool D1Hero::save(const SaveAsParam &params)
{
    QString filePath = this->filePath;
    if (!params.filePath.isEmpty()) {
        filePath = params.filePath;
        if (!params.autoOverwrite && QFile::exists(filePath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, QApplication::tr("Confirmation"), QApplication::tr("Are you sure you want to overwrite %1?").arg(QDir::toNativeSeparators(filePath)), QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return false;
            }
        }
    } else if (!this->isModified()) {
        return false;
    }

    if (filePath.isEmpty()) {
        return false;
    }

    QDir().mkpath(QFileInfo(filePath).absolutePath());
    QFile outFile = QFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        dProgressFail() << QApplication::tr("Failed to open file: %1.").arg(QDir::toNativeSeparators(filePath));
        return false;
    }

    HeroSaveStruct hss;
    PackPlayer(&hss.pps, this->pnum);
    hss.isHellfire = IsHellfireGame;
    hss.isMulti = IsMultiGame;
    // static_assert(lengthof(hss.itemNames) >= lengthof(plr._pInvBody) + lengthof(plr._pInvList) + lengthof(plr._pSpdList), "item-name copy in D1Hero::save must be adjusted.");
    static_assert(sizeof(hss.itemNames[0]) == sizeof(plr._pInvBody[0]._iName), "memcopy in D1Hero::save must be adjusted.");
    int cursor = 0;
    for (int i = 0; i < lengthof(plr._pInvBody); i++, cursor++) {
        memcpy(&hss.itemNames[cursor], plr._pInvBody[i]._iName, sizeof(plr._pInvBody[i]._iName));
    }
    for (int i = 0; i < lengthof(plr._pInvList); i++, cursor++) {
        memcpy(&hss.itemNames[cursor], plr._pInvList[i]._iName, sizeof(plr._pInvList[i]._iName));
    }
    for (int i = 0; i < lengthof(plr._pSpdList); i++, cursor++) {
        memcpy(&hss.itemNames[cursor], plr._pSpdList[i]._iName, sizeof(plr._pSpdList[i]._iName));
    }

    // write to file
    QDataStream out(&outFile);
    bool result = out.writeRawData((char *)&hss, sizeof(HeroSaveStruct)) == sizeof(HeroSaveStruct);
    if (result) {

        this->filePath = filePath; //  D1Hero::load(gfx, filePath);
        this->modified = false;
    }

    return result;
}
