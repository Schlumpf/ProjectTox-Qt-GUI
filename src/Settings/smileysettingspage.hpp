/*
    Copyright (C) 2014 by Martin Kröll <technikschlumpf@web.de>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef SMILEYSETTINGSPAGE_HPP
#define SMILEYSETTINGSPAGE_HPP

#include "abstractsettingspage.hpp"
#include "messages/smiley.hpp"
#include "smileypack.hpp"

class QGroupBox;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QLabel;
class EmojiFontComboBox;
class QSpinBox;

class SmileySettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    SmileySettingsPage(QWidget *parent);

    void buildGui();
    void setGui();
    void applyChanges();

private slots:
    void updatesmileyPackDesc(int index);
    void updateEmojiPreview();

private:
    QGroupBox *buildSmileyGroup();
    QGroupBox *buildEmojiFontGroup();

    void searchSmileyPacks();

    QGroupBox *mSmileyGroup;
    QComboBox *mSmileyType;

    QComboBox *mSmileypackCombobox;
    QLabel    *mSmileypackDescLabel;
    QLabel    *mSmileypackPreview;

    QList<Smileypack*> mSmileyPacks;

    QCheckBox *mSendPlaintextCheckbox;

    // Emoji font settings
    QGroupBox         *mEmojiFontGroup;
    EmojiFontComboBox *mEmojiFontComboBox;
    QSpinBox          *mEmojiFontSizeSpinBox;
    QLabel            *mEmojiFontPreview;
};


#endif // SMILEYSETTINGSPAGE_HPP
