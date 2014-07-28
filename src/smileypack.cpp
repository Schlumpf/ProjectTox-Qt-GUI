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
#include <QTextCursor>

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
        text.replace(pair.first, textSmilies.first());
    }
    return text;
}

/*! This function changes doc */
void Smileypack::resizeEmoji(QTextDocument *doc)
{
    const Settings &settings = Settings::getInstance();

    // Simplyfied list from https://gist.github.com/mattt/8185075
    QRegExp rx("(\\x00A9|\
\\x00AE|\
\\x203C|\
\\x2049|\
\\x2122|\
\\x2139|\
[\\x2194-\\x2199]|\
[\\x21A9-\\x21AA]|\
[\\x231A-\\x231B]|\
[\\x23E9-\\x23EC]|\
\\x23F0|\
\\x23F3|\
\\x24C2|\
[\\x25AA-\\x25AB]|\
\\x25B6|\
\\x25C0|\
[\\x25FB-\\x2601]|\
\\x260E|\
\\x2611|\
[\\x2614-\\x2615]|\
\\x261D|\
\\x263A|\
[\\x2648-\\x2653]|\
[\\x2660-\\x2668]|\
\\x267B|\
\\x267F|\
\\x2693|\
[\\x26A0-\\x26A1]|\
[\\x26AA-\\x26AB]|\
[\\x26BD-\\x26BE]|\
[\\x26C4-\\x26C5]|\
\\x26CE|\
\\x26D4|\
\\x26EA|\
[\\x26F2-\\x26F5]|\
\\x26FA|\
\\x26FD|\
\\x2702|\
\\x2705|\
[\\x2708-\\x2716]|\
\\x2728|\
[\\x2733-\\x2734]|\
\\x2744|\
\\x2747|\
\\x274C|\
\\x274E|\
[\\x2753-\\x2757]|\
\\x2764|\
[\\x2795-\\x2797]|\
\\x27A1|\
\\x27B0|\
\\x27BF|\
[\\x2934-\\x2935]|\
[\\x2B05-\\x2B07]|\
[\\x2B1B-\\x2B1C]|\
\\x2B50|\
\\x2B55|\
\\x3030|\
\\x303D|\
[\\x3297-\\x3299]|\
\\xD83C\\xDC04|\
\\xD83C\\xDCCF|\
\\xD83C[\\xDD70-\\xDD71]|\
\\xD83C[\\xDD7E-\\xDD7F]|\
\\xD83C\\xDD8E|\
\\xD83C[\\xDD91-\\xDD9A]|\
\\xD83C[\\xDE01-\\xDE02]|\
\\xD83C\\xDE1A|\
\\xD83C\\xDE2F|\
\\xD83C[\\xDE32-\\xDE3A]|\
\\xD83C[\\xDE50-\\xDE51]|\
\\xD83C[\\xDF00-\\xDFF0]|\
\\xD83D[\\xDC00-\\xDD6F]|\
\\xD83D[\\xDDFB-\\xDE4F]|\
\\xD83D[\\xDE80-\\xDEC5])");
    QTextCursor c = doc->find(rx,0);

    while (!c.isNull()) {
        QTextCharFormat format;
        format.setFont(settings.getEmojiFont());
        c.mergeCharFormat(format);

        // Find next
        c = doc->find(rx,c);
    }
}

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
        {"🎵", {"(music)"}},
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
