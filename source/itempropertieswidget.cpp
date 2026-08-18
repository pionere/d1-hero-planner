#include "itempropertieswidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QString>

#include "ui_itempropertieswidget.h"

#include "dungeon/all.h"

ItemPropertiesWidget::ItemPropertiesWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ItemPropertiesWidget())
{
    ui->setupUi(this);
}

ItemPropertiesWidget::~ItemPropertiesWidget()
{
    delete ui;
}

void ItemPropertiesWidget::initialize(const ItemStruct *is)
{
    QString label;
    QString text;
    QString tooltip;
    int active;

    active = 0;
    if (is->_iClass == ICLASS_WEAPON) {
        label = tr("Damage:");
        if (is->_iMinDam == is->_iMaxDam)
            text = QString::number(is->_iMinDam);
        else
            text = QString("%1-%2").arg(is->_iMinDam).arg(is->_iMaxDam);
        switch (is->_iDamType) {
        case IDAM_NONE:     tooltip = tr("-");           break;
        case IDAM_SLASH:    tooltip = tr("Slash");       break;
        case IDAM_BLUNT:    tooltip = tr("Blunt");       break;
        case IDAM_SB_MIX:   tooltip = tr("Slash/Blunt"); break;
        case IDAM_PUNCTURE: tooltip = tr("Puncture");    break;
        }
        tooltip = tr("Type: %1. Base crit. chance: %2").arg(tooltip).arg(is->_iBaseCrit);
        active = 1;
    } else if (is->_iClass == ICLASS_ARMOR) {
        label = tr("Armor:");
        text = QString::number(is->_iAC);
        tooltip = "";
        active = 1;
    }
    this->ui->itemDAMLabel->setVisible(active != 0);
    this->ui->itemDAMLabel->setText(label);
    this->ui->itemDAMText->setVisible(active != 0);
    this->ui->itemDAMText->setText(text);
    this->ui->itemDAMText->setToolTip(tooltip);

    active = is->_iNumAffixes;
    for (int i = 0; i < active; i++) {
        PrintItemPower(i, is);
        switch (i) {
        case 0: this->ui->itemPower1Text->setText(tempstr); break;
        case 1: this->ui->itemPower2Text->setText(tempstr); break;
        case 2: this->ui->itemPower3Text->setText(tempstr); break;
        case 3: this->ui->itemPower4Text->setText(tempstr); break;
        case 4: this->ui->itemPower5Text->setText(tempstr); break;
        case 5: this->ui->itemPower6Text->setText(tempstr); break;
        }
    }
    this->ui->itemPower1Label->setVisible(active > 0);
    this->ui->itemPower1Text->setVisible(active > 0);
    this->ui->itemPower2Label->setVisible(active > 1);
    this->ui->itemPower2Text->setVisible(active > 1);
    this->ui->itemPower3Label->setVisible(active > 2);
    this->ui->itemPower3Text->setVisible(active > 2);
    this->ui->itemPower4Label->setVisible(active > 3);
    this->ui->itemPower4Text->setVisible(active > 3);
    this->ui->itemPower5Label->setVisible(active > 4);
    this->ui->itemPower5Text->setVisible(active > 4);
    this->ui->itemPower6Label->setVisible(active > 5);
    this->ui->itemPower6Text->setVisible(active > 5);

    active = 0;
    if ((is->_iReqStr | is->_iReqMag | is->_iReqDex) != 0) {
        text.clear();
        if (is->_iReqStr)
            text.append(tr("%1 Str  ").arg(is->_iReqStr));
        if (is->_iReqMag)
            text.append(tr("%1 Mag  ").arg(is->_iReqMag));
        if (is->_iReqDex)
            text.append(tr("%1 Dex  ").arg(is->_iReqDex));
        this->ui->itemRequirementsText->setText(text);
        active = 1;
    }
    this->ui->itemRequirementsLabel->setVisible(active != 0);
    this->ui->itemRequirementsText->setVisible(active != 0);

/*
	union {
		int _ix;
		int _iPHolder; // parent index of a placeholder entry in InvList
	};
	int _iy;
	int _iCurs;   // item_cursor_graphic
	int _iMiscId; // item_misc_id
	int _iSpell;  // spell_id
	BYTE _iMagical;	// item_quality
	BYTE _iSelFlag;
	BOOLEAN _iStatFlag;
	BOOLEAN _iUnidentified;
	int _ivalue;
	int _iIvalue;
	int _iCharges;
	int _iMaxCharges;
	int _iDurability;
	int _iMaxDur;
	int _iPLDam;
	int _iPLToHit;
	int _iPLStr;
	int _iPLMag;
	int _iPLDex;
	int _iPLVit;

	int _iUid; // unique_item_indexes

*/

}
