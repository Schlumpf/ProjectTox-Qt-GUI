/*
    Copyright (C) 2013 by Martin Kröll <technikschlumpf@web.de>

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

#include "emoticonmenu.hpp"

#include <QGridLayout>
#include <QToolButton>
#include <QWidgetAction>

#include "smileypack.hpp"
#include "Settings/settings.hpp"

EmoticonMenu::EmoticonMenu(QWidget *parent) :
    QMenu(parent)
{
    action              = new QWidgetAction(this);
    actionDefaultWidget = new QWidget(this);
    layout              = new QGridLayout(actionDefaultWidget);
    updateEmoticons();

    connect(&Settings::getInstance(), &Settings::smileySettingsChanged, this, &EmoticonMenu::updateEmoticons);
}

void EmoticonMenu::updateEmoticons()
{
    // Delete old menu
    action->deleteLater();
    actionDefaultWidget->deleteLater();
    layout->deleteLater();

    // Create new menu
    action = new QWidgetAction(this);
    actionDefaultWidget = new QWidget(this);
    layout = new QGridLayout(actionDefaultWidget);
    layout->setMargin(1);
    layout->setSpacing(0);
    action->setDefaultWidget(actionDefaultWidget);
    addAction(action);

    // Add new pack
    const Settings &settings = Settings::getInstance();
    if (settings.getSmileyType() == Smiley::Emoji) {
        for (const auto& pair : Smileypack::emojiList()) {
            addEmoticon(pair.first, pair.second, true);
        }
    }
    else {
        Smileypack::currentPack().getList();
        for (const auto& pair : Smileypack::currentPack().getList()) {
            addEmoticon(pair.first, pair.second, false);
        }
    }
}

void EmoticonMenu::addEmoticon(const QString &imgPath, const QStringList &texts, bool isEmoji)
{
    const Settings &settings = Settings::getInstance();

    QToolButton *button = new QToolButton(this);
    if (isEmoji) {
        button->setText(imgPath);
        if (settings.isCurstomEmojiFont())
            button->setFont(settings.getEmojiFont());
    }
    else {
        button->setIcon(QIcon(imgPath));
    }
    button->setProperty("smiley", imgPath);
    button->setProperty("rawText", texts.first());
    button->setAutoRaise(true);
    button->setToolTip(texts.first());

    connect(button, &QToolButton::clicked, this, &EmoticonMenu::onEmoticonTriggered);
    connect(button, &QToolButton::clicked, this, &EmoticonMenu::close);

    layout->addWidget(button, layout->count() / EMOTICONS_IN_A_ROW, layout->count() % EMOTICONS_IN_A_ROW);
}

/*! Signal sends the (first) textual form of the clicked smiley. */
void EmoticonMenu::onEmoticonTriggered()
{
    emit insertEmoticon(QObject::sender()->property("smiley").toString());
    emit insertSmiley(Smiley(QObject::sender()->property("rawText").toString(), QObject::sender()->property("smiley").toString(), 0, 0, (Smiley::Type) Settings::getInstance().getSmileyType()));
}
