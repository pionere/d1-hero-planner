#pragma once

#include "sliderwidget.h"

class AffixSliderWidget : public SliderWidget {
    Q_OBJECT

public:
    AffixSliderWidget(QWidget *parent = nullptr);
    ~AffixSliderWidget() = default;

    void changeValue(int value);
    void setItemLevel(int level);
    void setItemMiscId(int miscId);
    void setLimitMode(int mode);

private slots:
    void on_valueChanged(int value);

private:
    void updateToolTip();

    int itemLevel = 0;
    int miscId = 0;
    int limitMode = 0;
};
