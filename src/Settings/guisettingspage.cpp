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

#include "guisettingspage.hpp"

#include "settings.hpp"
#include "appinfo.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSettings>
#include <QRegularExpression>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QDateTime>

#include "smileypack.hpp"

GuiSettingsPage::GuiSettingsPage(QWidget *parent) :
    AbstractSettingsPage(parent)
{
}

void GuiSettingsPage::buildGui()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(buildAnimationGroup());
    layout->addWidget(buildChatviewGroup());
    layout->addWidget(buildOthersGroup());
    layout->addStretch(0);
}

void GuiSettingsPage::setGui()
{
    const Settings& settings = Settings::getInstance();
    enableAnimationCheckbox->setChecked(settings.isAnimationEnabled());
    minimizeToTrayCheckbox->setChecked(settings.isMinimizeOnCloseEnabled());
    timestampLineedit->setText(settings.getTimestampFormat());
}

void GuiSettingsPage::applyChanges()
{
    Settings& settings = Settings::getInstance();
    settings.setAnimationEnabled(enableAnimationCheckbox->isChecked());

    settings.setTimestampFormat(timestampLineedit->text());
    settings.setMinimizeOnClose(minimizeToTrayCheckbox->isChecked());
}

QGroupBox *GuiSettingsPage::buildAnimationGroup()
{
    QGroupBox *group = new QGroupBox(tr("Smooth animation"), this);
    QVBoxLayout* layout = new QVBoxLayout(group);
    enableAnimationCheckbox = new QCheckBox(tr("Enable animation"), group);

    layout->addWidget(enableAnimationCheckbox);
    return group;
}

QGroupBox *GuiSettingsPage::buildChatviewGroup()
{
    QGroupBox *group = new QGroupBox(tr("Chat view"), this);
    QFormLayout *layout = new QFormLayout(group);
    QVBoxLayout *horizontal = new QVBoxLayout();

    timestampLineedit = new QLineEdit(group);
    timestampPreview = new QLabel(group);

    horizontal->addWidget(timestampLineedit);
    horizontal->addWidget(timestampPreview);

    layout->addRow(tr("Timestamp format:"), horizontal);

    connect(timestampLineedit, &QLineEdit::textChanged, this, &GuiSettingsPage::updateTimestampPreview);

    return group;
}

void GuiSettingsPage::updateTimestampPreview(QString format)
{
    timestampPreview->setText(tr("Preview: %1").arg(QDateTime::currentDateTime().toString(format)));
}

QGroupBox* GuiSettingsPage::buildOthersGroup()
{
    QGroupBox *group = new QGroupBox(tr("Others"), this);
    QVBoxLayout* layout = new QVBoxLayout(group);
    minimizeToTrayCheckbox = new QCheckBox(tr("Minimize to tray on close"), group);

    layout->addWidget(minimizeToTrayCheckbox);
    return group;
}
