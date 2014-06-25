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

#include "smileypack.hpp"

#include <QTextDocument>
#include <QRegularExpression>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDataStream>
#include "appinfo.hpp"
#include "Settings/settings.hpp"
#include <QDebug>

Smileypack::Smileypack(QObject *parent) :
    QObject(parent)
{
}

void Smileypack::operator =(const Smileypack &other)
{
    list = other.getList();
    name = other.name;
    author = other.author;
    description = other.description;
    version = other.version;
    website = other.website;
    icon = other.icon;
}

QString Smileypack::desmilify(QString htmlText)
{
    const Settings &settings = Settings::getInstance();

    // De-emoji if emoji is enabled
    if (settings.getSmileyType() == Smiley::Emoji) {
        QTextDocument doc;
        doc.setHtml(htmlText);
        QString plain = doc.toPlainText();

        if(settings.isEmojiSendPlaintext())
            return Smileypack::deemojify(plain);
        else
            return plain;
    }

    // Replace smileys by their textual representation
    int i = 0;
    QRegularExpression re(R"_(<img[\s]+[^>]*?((alt*?[\s]?=[\s\"\']+(.*?)[\"\']+.*?)|(src*?[\s]?=[\s\"\']+(.*?)[\"\']+.*?))((src*?[\s]?=[\s\"\']+(.*?)[\"\']+.*?>)|(alt*?[\s]?=[\s\"\']+(.*?)[\"\']+.*?>)|>))_");
    QRegularExpressionMatch match = re.match(htmlText, i);
    while (match.hasMatch()) {
        // Replace smiley and match next
        for (const auto& pair : Smileypack::currentPack().getList()) {
            if (pair.first == match.captured(5)) {
                const QStringList& textSmilies = pair.second;
                if (textSmilies.isEmpty()) {
                    htmlText.remove(match.captured(0));
                } else {
                    htmlText.replace(match.captured(0), textSmilies.first());
                }
                break;
            }
        }
        match = re.match(htmlText, ++i);
    }

    // convert to plain text
    QTextDocument doc;
    doc.setHtml(htmlText);
    return doc.toPlainText();
}

/*! Replace Emoji by text strings */
QString Smileypack::deemojify(QString text)
{
    for (const auto& pair : Smileypack::emojiList()) {
        const QStringList& textSmilies = pair.second;
        text.replace(pair.first, textSmilies.first().toHtmlEscaped());
    }
    return text;
}

/*
QString Smileypack::resizeEmoji(QString text)
{
    Settings &settings = Settings::getInstance();

    // All Unicode 6.2 emoji "Emoticons" and a some of "Miscellaneous Symbols and Pictographs"
    // TODO find a regular expression for that
    // nurupo: that will do `text.replace(QRegularExpression("([\\x{1F600}-\\x{1F64F}])"), QString("<span style=\"font-family: '%1'; font-size: %2pt;\">\\1</span>").arg(settings.getEmojiFont(), QString::number(settings.getEmojiSize())));`
    //         except that you used symbols outside the Emoticons range (1F600-1F64F), so we need count for the too
    QStringList foundEmojis({"😀","😁","😂","😃","😄","😅","😆","😇","😈","😉","😊","😋","😌","😍","😎","😏","😐","😑","😒","😓","😔","😕","😖","😗","😘","😙","😚","😛","😜","😝","😞","😟","😠","😡","😢","😣","😤","😥","😦","😧","😨","😩","😪","😫","😬","😭","😮","😯","😰","😱","😲","😳","😴","😵","😶","😷","😸","😹","😺","😻","😼","😽","😾","😿","🙀","🙅","🙆","🙇","🙈","🙉","🙊","🙋","🙌","🙍","🙎","🙏","☺","☹","⚇","🐱","♥","☔","☀","♫","☕","★"});

    for (QString emo : foundEmojis) {
        text.replace(emo, QString("<span style=\"font-family: '%1'; font-size: %2pt;\">%3</span>").arg(settings.getEmojiFontFamily(), QString::number(settings.getEmojiFontPointSize()), emo));
    }

    return text;
}
*/

const Smileypack::SmileypackList Smileypack::emojiList()
{
    static const SmileypackList tmpList =
    {
        // Smileys
        {"☺", {":)",  ":-)"}},
        {"😞", {":(",  ":-("}},
        {"😄", {":D",  ":-D"}},
        {"😎", {"8)",  "8-)",  "B)",  "B-)"}},
        {"😲", {":O",  ":-O",  ":o",  ":-o"}},
        {"😉", {";)",  ";-)"}},
        {"😢", {";(",  ";-("}},
        {"😓", {"(:|",  "(:-|"}},
        {"😐", {":|",  ":-|"}},
        {"😚", {":*",  ":-*"}},
        {"😜", {":P",  ":-P",  ":p",  ":-p"}},
        // Blushing         :$
        {"😒", {":^)",  ":^-)"}},
        {"😪", {"|)",   "|-)"}},
        // Dull             |-(
        {"😍", {"(inlove)"}},
        {"😈", {"]:)", "]:-)", "(devil)"}},
        // Fingers crossed  (yn)
        // Yawn             (yawn)
        // Puking           (puke)
        // Doh!             (doh)
        {"😠", {">:(", ">:-(", "(angry)"}},
        // It wasn’t me!    (wasntme)
        // Party            (party)
        {"😰", {"(worry)"}},
        {"😏", {"(mm)"}},
        // Nerdy            (nerd)
        {"😷", {":x",  ":-x",  ":X",  ":-X"}},
        // Hi               (wave)
        // Facepalm         (facepalm)
        {"😇", {"O:)", "O:-)", "o:)", "o:-)", "(angel)"}},
        // ...
        {"♥", {"<3",  "(h)"}},
        // ...
        {"☔", {"(rain)"}},
        {"☀", {"(sun)"}},
        // Tumbleweed       (tumbleweed)
        {"♫", {"(music)"}},
        // ...
        {"☕", {"(coffee)"}},
        // ...
        {"★", {"(*)"}},
        // Additional smileys
        {"🐱", {":3"}}
    };
    return tmpList;
}

Smileypack &Smileypack::currentPack()
{
    static Smileypack pack;

    // no pack selected
    if (pack.getName().isEmpty() && !Settings::getInstance().getSmileyPackPath().isEmpty()) {
        if(pack.parseFile(Settings::getInstance().getSmileyPackPath()))
            // TODO MKO correct to connect here?
            connect(&Settings::getInstance(), &Settings::smileyPackChanged, &Smileypack::currentPack(), &Smileypack::updatePack);
    }

    return pack;
}


void Smileypack::updatePack()
{
    const Settings &settings = Settings::getInstance();
    parseFile(settings.getSmileyPackPath());
}

bool Smileypack::parseFile(const QString &filePath)
{
    // Open file
    QFile file(filePath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    themeFile = filePath;

    // Clear old data
    list.clear();
    name.clear();
    author.clear();
    description.clear();
    version.clear();
    website.clear();
    icon.clear();

    // Get pack folder path
    QFileInfo info(file);
    QString path = info.absolutePath() + '/';

    // Read lines
    ParserStates state = StateHead;
    while (!file.atEnd()) {
        processLine(file.readLine(), path, state);
    }

    // End parsing
    file.close();
    return true;
}


void Smileypack::processLine(const QString &xLine, const QString &xPath, ParserStates &xState)
{
    // Trim spaces and exclue comment lines
    QString line = xLine.trimmed();
    if (line.startsWith('#') || line.isEmpty()) {
        return;
    }

    if (xState == StateHead) {

        // Switch on [theme]
        QRegularExpression rx("\\[(theme|smileys)\\]");
        QRegularExpressionMatch match = rx.match(line);
        if (match.hasMatch()) {
            xState = StateSmileys;
            return;
        }

        QString key   = line.section(QRegularExpression("\\s*\\=\\s*"), 0,0);
        QString value = line.section(QRegularExpression("\\s*\\=\\s*"), 1);

        if      (key == "Name")        name        = value;
        else if (key == "Author")      author      = value;
        else if (key == "Description") description = value;
        else if (key == "Version")     version     = value;
        else if (key == "Website")     website     = value;
        else if (key == "Icon")        icon        = value;
    }
    else if (xState == StateSmileys) {
        QString key   = line.section(QRegularExpression("\\s+"), 0,0);
        QString value = line.section(QRegularExpression("\\s+"), 1);

        list.append({xPath+key, value.split(QRegularExpression("\\s+"))});
    }
}
