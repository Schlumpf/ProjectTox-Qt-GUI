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

#include "smileysettingspage.hpp"
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QButtonGroup>
#include "settings.hpp"
#include "emojifontcombobox.hpp"
#include "QApplication"
#include "smileypack.hpp"
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QStackedWidget>

SmileySettingsPage::SmileySettingsPage(QWidget *parent) :
    AbstractSettingsPage(parent)
{
}

void SmileySettingsPage::buildGui()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(buildSmileyGroup());
    layout->addWidget(buildEmojiFontGroup());
    layout->addStretch(0);
}

void SmileySettingsPage::setGui()
{
    const Settings& settings = Settings::getInstance();

    // Emoji font settings
    mSmileyGroup->setChecked(settings.isSmileyReplacementEnabled());
    mSmileyType->setCurrentIndex(mSmileyType->findData((Smiley::Type)settings.getSmileyType()));
    mEmojiFontGroup->setChecked(settings.isCurstomEmojiFont());
    mEmojiFontComboBox->setCurrentIndex(mEmojiFontComboBox->findText(settings.getEmojiFont().family()));
    mEmojiFontSizeSpinBox->setValue(settings.getEmojiFont().pointSize());
    mSendPlaintextCheckbox->setChecked(settings.isEmojiSendPlaintext());

    searchSmileyPacks();

    int index = 0;
    for (int i=0; i<mSmileyPacks.count(); i++) {
        if(mSmileyPacks.at(i)->getThemeFile() == settings.getSmileyPackPath())
            index = i;
    }
    mSmileypackCombobox->setCurrentIndex(index);
}

void SmileySettingsPage::applyChanges()
{
    Settings& settings = Settings::getInstance();
    settings.setSmileySettings(mSmileyGroup->isChecked(),
                               mSmileyType->currentData().toInt(),
                               mSmileyPacks.at(mSmileypackCombobox->currentIndex())->getThemeFile(),
                               mEmojiFontGroup->isChecked(),
                               mEmojiFontComboBox->itemText(mEmojiFontComboBox->currentIndex()),
                               mEmojiFontSizeSpinBox->value(),
                               mSendPlaintextCheckbox->isChecked());
}

QGroupBox *SmileySettingsPage::buildSmileyGroup()
{
    mSmileyGroup = new QGroupBox(tr("Enable smiley replacement"), this);
    mSmileyGroup->setCheckable(true);

    mSmileyType = new QComboBox(mSmileyGroup);
    mSmileyType->addItem(tr("Use a smileypack"), Smiley::Pixmap);
    mSmileyType->addItem(tr("Use emoji"), Smiley::Emoji);

    QStackedWidget *typeStack = new QStackedWidget(mSmileyGroup);
    connect(mSmileyType, SIGNAL(currentIndexChanged(int)), typeStack, SLOT(setCurrentIndex(int)));

    // Smileypack settings
    QWidget *packPage = new QWidget(typeStack);
    packPage->setContentsMargins(0,0,0,0);
    typeStack->addWidget(packPage);
    QVBoxLayout *packLayout = new QVBoxLayout(packPage);
    packLayout->setContentsMargins(0,0,0,0);
    mSmileypackCombobox = new QComboBox(packPage);
    mSmileypackDescLabel = new QLabel(packPage);
    mSmileypackDescLabel->setWordWrap(true);
    mSmileypackDescLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mSmileypackDescLabel->setOpenExternalLinks(true);
    mSmileypackPreview = new QLabel(packPage);
    mSmileypackPreview->setAlignment(Qt::AlignCenter);
    mSmileypackPreview->setMinimumSize(300, 30);
    mSmileypackPreview->setFrameShape(QFrame::StyledPanel);
    packLayout->addWidget(mSmileypackCombobox);
    packLayout->addWidget(mSmileypackDescLabel);
    packLayout->addWidget(mSmileypackPreview);
    packLayout->addStretch(0);
    connect(mSmileypackCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updatesmileyPackDesc(int)));

    // Emoji settings
    QWidget *emojiPage = new QWidget(typeStack);
    emojiPage->setContentsMargins(0,0,0,0);
    typeStack->addWidget(emojiPage);
    QVBoxLayout *emojiLayout = new QVBoxLayout(emojiPage);
    emojiLayout->setContentsMargins(0,0,0,0);
    mSendPlaintextCheckbox = new QCheckBox(tr("Send plaintext (reconvert emoji characters)"), emojiPage);
    emojiLayout->addWidget(mSendPlaintextCheckbox);

    QVBoxLayout *layout = new QVBoxLayout(mSmileyGroup);
    layout->addWidget(mSmileyType);
    layout->addWidget(typeStack);

    return mSmileyGroup;
}

QGroupBox *SmileySettingsPage::buildEmojiFontGroup()
{
    mEmojiFontGroup = new QGroupBox(tr("Enable emoji font override"), this);
    mEmojiFontGroup->setCheckable(true);

    mEmojiFontComboBox = new EmojiFontComboBox(mEmojiFontGroup);
    mEmojiFontSizeSpinBox = new QSpinBox(mEmojiFontGroup);
    mEmojiFontSizeSpinBox->setRange(4, 72);
    mEmojiFontSizeSpinBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    QToolButton *defaultButton = new QToolButton(mEmojiFontGroup);
    defaultButton->setIcon(QIcon("://icons/arrow_undo.png"));
    defaultButton->setToolTip(tr("Reset"));
    defaultButton->setAutoRaise(true);

    connect(defaultButton, &QToolButton::clicked, [=]() {
        mEmojiFontComboBox->setCurrentIndex(mEmojiFontComboBox->findText(QStringLiteral("DejaVu Sans")));
        mEmojiFontSizeSpinBox->setValue(QApplication::font().pointSize());
    });

    mEmojiFontPreview = new QLabel(mEmojiFontGroup);
    mEmojiFontPreview->setAlignment(Qt::AlignCenter);
    mEmojiFontPreview->setMinimumSize(300, 30);
    mEmojiFontPreview->setFrameShape(QFrame::StyledPanel);
    mEmojiFontPreview->setText("☺ 😞 😄 😲 😉 😜 😇 🐱");

    connect(mEmojiFontComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEmojiPreview()));
    connect(mEmojiFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateEmojiPreview()));

    QHBoxLayout *selectLayout = new QHBoxLayout();
    selectLayout->addWidget(mEmojiFontComboBox);
    selectLayout->addWidget(mEmojiFontSizeSpinBox);
    selectLayout->addWidget(defaultButton);


    QVBoxLayout *layout = new QVBoxLayout(mEmojiFontGroup);
    layout->addLayout(selectLayout);
    layout->addWidget(mEmojiFontPreview);

    return mEmojiFontGroup;
}

void SmileySettingsPage::searchSmileyPacks()
{
    // Go to smiley pack folder
    QDir dir(Settings::getSettingsDirPath()+"/smileys");
    if (!dir.mkpath(Settings::getSettingsDirPath()+"/smileys"))
        return;

    // Go through all packs
    dir.setFilter(QDir::Dirs|QDir::NoDot|QDir::NoDotDot);
    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();

        // Check theme file
        QFileInfo f(it.filePath() + "/theme");
        if (!f.exists()) {
            continue;
        }

        // Parse theme file
        Smileypack *newPack = new Smileypack(this);
        if (!newPack->parseFile(f.absoluteFilePath())) {
            continue;
        }

        // Add new pack
        mSmileypackCombobox->addItem(newPack->getName());
        mSmileyPacks.append(newPack);
    }
}

void SmileySettingsPage::updatesmileyPackDesc(int index)
{
    if(index >= mSmileyPacks.count())
        return;

    const Smileypack *pack = mSmileyPacks.at(index);

    // Description
    QString version = pack->getVersion();
    if (!version.isEmpty()) {
        version.prepend(" v");
    }
    QString website = pack->getWebsite();
    if (!website.isEmpty()) {
        website = QString("<br><a href=\"%1\">%1</a>").arg(website);
    }
    QString desc = tr("<b>%1</b>%2 by %3<br>\"<i>%4</i>\"%5").arg(pack->getName(), version, pack->getAuthor(), pack->getDescription(), website);
    mSmileypackDescLabel->setText(desc);

    // Preview
    QString text;
    int count = pack->getList().count();
    if(count > 10)
        count = 10;
    for (int i=0; i<count; i++)
        text.append(QString("<img src=\"%1\"> ").arg(pack->getList().at(i).first));

    mSmileypackPreview->setText(text);
}

void SmileySettingsPage::updateEmojiPreview()
{
    QFont font;
    font.setFamily(mEmojiFontComboBox->itemText(mEmojiFontComboBox->currentIndex()));
    font.setPointSize(mEmojiFontSizeSpinBox->value());
    mEmojiFontPreview->setFont(font);
}
