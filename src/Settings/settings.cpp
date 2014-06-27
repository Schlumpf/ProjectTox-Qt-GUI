/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    
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

#include "settings.hpp"
#include "settingsdialog.hpp"
#include "smileypack.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

const QString Settings::FILENAME = "settings.ini";

Settings::Settings() :
    loaded(false)
{
    load();
}

Settings::~Settings()
{
    save();
}

Settings& Settings::getInstance()
{
    static Settings settings;
    return settings;
}

void Settings::load()
{
    if (loaded) {
        return;
    }

    QString filePath = getSettingsDirPath() + '/' + FILENAME;

    //if no settings file exist -- use the default one
    QFile file(filePath);
    if (!file.exists()) {
        filePath = ":/texts/" + FILENAME;
    }

    QSettings s(filePath, QSettings::IniFormat);
    s.beginGroup("DHT Server");
        int serverListSize = s.beginReadArray("dhtServerList");
        for (int i = 0; i < serverListSize; i ++) {
            s.setArrayIndex(i);
            DhtServer server;
            server.name = s.value("name").toString();
            server.userId = s.value("userId").toString();
            server.address = s.value("address").toString();
            server.port = s.value("port").toInt();
            dhtServerList << server;
        }
        s.endArray();
    s.endGroup();

    //NOTE: uncomment when logging will be implemented
/*
    s.beginGroup("Logging");
       enableLogging = s.value("enableLogging", false).toBool();
       encryptLogs = s.value("encryptLogs", true).toBool();
    s.endGroup();
*/

    s.beginGroup("General");
        username = s.value("username", "My name").toString();
        statusMessage = s.value("statusMessage", "My status").toString();
    s.endGroup();

    s.beginGroup("Widgets");
        QList<QString> objectNames = s.childKeys();
        for (const QString& name : objectNames) {
            widgetSettings[name] = s.value(name).toByteArray();
        }
    s.endGroup();

    s.beginGroup("GUI");
        enableSmoothAnimation = s.value("smoothAnimation", true).toBool();
        firstColumnHandlePos = s.value("firstColumnHandlePos", 50).toInt();
        secondColumnHandlePosFromRight = s.value("secondColumnHandlePosFromRight", 50).toInt();
        timestampFormat = s.value("timestampFormat", "hh:mm").toString();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
    s.endGroup();

    s.beginGroup("Smileys");
        smileyReplacementEnabled = s.value("smileyReplacementEnabled", true).toBool();
        smileyType = s.value("smileyType", 1).toInt(); // 1 = pixmap, see smiley.hpp
        smileyPackPath = s.value("smileyPackPath").toString();
        emojiFontOverride = s.value("customEmojiFont", true).toBool();
        emojiFontFamily = s.value("emojiFontFamily", "DejaVu Sans").toString();
        emojiFontPointSize = s.value("emojiFontPointSize", QApplication::font().pointSize()).toInt();
        emojiSendPlaintext = s.value("emojiSendPlaintext", true).toBool();
    s.endGroup();

    s.beginGroup("Privacy");
        typingNotification = s.value("typingNotification", false).toBool();
    s.endGroup();

    loaded = true;
}

void Settings::save()
{
    QString filePath = getSettingsDirPath() + '/' + FILENAME;

    QSettings s(filePath, QSettings::IniFormat);

    s.clear();

    s.beginGroup("DHT Server");
        s.beginWriteArray("dhtServerList", dhtServerList.size());
        for (int i = 0; i < dhtServerList.size(); i ++) {
            s.setArrayIndex(i);
            s.setValue("name", dhtServerList[i].name);
            s.setValue("userId", dhtServerList[i].userId);
            s.setValue("address", dhtServerList[i].address);
            s.setValue("port", dhtServerList[i].port);
        }
        s.endArray();
    s.endGroup();

    //NOTE: uncomment when logging will be implemented
/*
    s.beginGroup("Logging");
        s.setValue("storeLogs", enableLogging);
        s.setValue("encryptLogs", encryptLogs);
    s.endGroup();
*/

    s.beginGroup("General");
        s.setValue("username", username);
        s.setValue("statusMessage", statusMessage);
    s.endGroup();

    s.beginGroup("Widgets");
    const QList<QString> widgetNames = widgetSettings.keys();
    for (const QString& name : widgetNames) {
        s.setValue(name, widgetSettings.value(name));
    }
    s.endGroup();

    s.beginGroup("GUI");
        s.setValue("smoothAnimation", enableSmoothAnimation);
        s.setValue("firstColumnHandlePos", firstColumnHandlePos);
        s.setValue("secondColumnHandlePosFromRight", secondColumnHandlePosFromRight);
        s.setValue("timestampFormat", timestampFormat);
        s.setValue("minimizeOnClose", minimizeOnClose);
    s.endGroup();

    s.beginGroup("Smileys");
        s.setValue("smileyReplacementEnabled", smileyReplacementEnabled);
        s.setValue("smileyType", smileyType);
        s.setValue("smileyPackPath", smileyPackPath);
        s.setValue("customEmojiFont", emojiFontOverride);
        s.setValue("emojiFontFamily", emojiFontFamily);
        s.setValue("emojiFontPointSize", emojiFontPointSize);
        s.setValue("emojiSendPlaintext", emojiSendPlaintext);
    s.endGroup();

    s.beginGroup("Privacy");
        s.setValue("typingNotification", typingNotification);
    s.endGroup();
}

QString Settings::getSettingsDirPath()
{
    // workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + '/' + qApp->organizationName() + '/' + qApp->applicationName();
#endif
}

void Settings::executeSettingsDialog(QWidget* parent)
{
    if (SettingsDialog::showDialog(parent) == QDialog::Accepted) {
        save();
        //emit dataChanged();
    }
}

const QList<Settings::DhtServer>& Settings::getDhtServerList() const
{
    return dhtServerList;
}

void Settings::setDhtServerList(const QList<DhtServer>& newDhtServerList)
{
    dhtServerList = newDhtServerList;
    emit dhtServerListChanged();
}

QString Settings::getUsername() const
{
    return username;
}

void Settings::setUsername(const QString& newUsername)
{
    username = newUsername;
}

QString Settings::getStatusMessage() const
{
    return statusMessage;
}

void Settings::setStatusMessage(const QString& newMessage)
{
    statusMessage = newMessage;
}

bool Settings::getEnableLogging() const
{
    return enableLogging;
}

void Settings::setEnableLogging(bool newValue)
{
    enableLogging = newValue;
}

bool Settings::getEncryptLogs() const
{
    return encryptLogs;
}

void Settings::setEncryptLogs(bool newValue)
{
    encryptLogs = newValue;
}

void Settings::setWidgetData(const QString& uniqueName, const QByteArray& data)
{
    widgetSettings[uniqueName] = data;
}

QByteArray Settings::getWidgetData(const QString& uniqueName) const
{
    return widgetSettings.value(uniqueName);
}

bool Settings::isAnimationEnabled() const
{
    return enableSmoothAnimation;
}

void Settings::setAnimationEnabled(bool newValue)
{
    enableSmoothAnimation = newValue;
}

int Settings::getFirstColumnHandlePos() const
{
    return firstColumnHandlePos;
}

void Settings::setFirstColumnHandlePos(const int pos)
{
    firstColumnHandlePos = pos;
}

int Settings::getSecondColumnHandlePosFromRight() const
{
    return secondColumnHandlePosFromRight;
}

void Settings::setSecondColumnHandlePosFromRight(const int pos)
{
    secondColumnHandlePosFromRight = pos;
}

const QString &Settings::getTimestampFormat() const
{
    return timestampFormat;
}

void Settings::setTimestampFormat(const QString &format)
{
    if (timestampFormat != format) {
        timestampFormat = format;
        emit timestampFormatChanged();
    }
}

bool Settings::isMinimizeOnCloseEnabled() const
{
    return minimizeOnClose;
}

void Settings::setMinimizeOnClose(bool newValue)
{
    minimizeOnClose = newValue;
}

// Smileys
void Settings::setSmileySettings(bool replacementEnabled, int type, const QString &packPath, bool eFontOverride, const QString &eFontFamily, int eFontPointSize, bool eSendPlaintext)
{
    bool changed = false;
    if (smileyReplacementEnabled != replacementEnabled) {
        smileyReplacementEnabled = replacementEnabled;
        changed = true;
    }
    if (smileyType != type) {
        smileyType = type;
        changed = true;
    }
    if (smileyPackPath != packPath) {
        smileyPackPath = packPath;
        emit smileyPackChanged();
        changed = true;
    }
    if (emojiFontOverride != eFontOverride) {
        emojiFontOverride = eFontOverride;
        changed = true;
    }
    if (emojiFontFamily != eFontFamily) {
        emojiFontFamily = eFontFamily;
        changed = true;
    }
    if (emojiFontPointSize != eFontPointSize) {
        emojiFontPointSize = eFontPointSize;
        changed = true;
    }
    if (emojiSendPlaintext != eSendPlaintext) {
        emojiSendPlaintext = eSendPlaintext;
        changed = true;
    }

    if(changed)
        emit smileySettingsChanged();
}

bool Settings::isSmileyReplacementEnabled() const
{
    return smileyReplacementEnabled;
}

int Settings::getSmileyType() const
{
    return smileyType;
}

QString Settings::getSmileyPackPath() const
{
    return smileyPackPath;
}

bool Settings::isCurstomEmojiFont() const
{
    return emojiFontOverride;
}

QFont Settings::getEmojiFont() const
{
    return QFont(emojiFontFamily, emojiFontPointSize);
}

bool Settings::isEmojiSendPlaintext() const
{
    return emojiSendPlaintext;
}

// Privacy
bool Settings::isTypingNotificationEnabled() const
{
    return typingNotification;
}

void Settings::setTypingNotification(bool enabled)
{
    typingNotification = enabled;
}
