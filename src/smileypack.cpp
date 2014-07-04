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

    // Complete emoji list: http://www.unicode.org/Public/UNIDATA/EmojiSources.txt
    // last update: EmojiSources-7.0.0.txt
    QRegExp rx("(\\x0023|\
[\\x0030-\\x0039]|\
\\x00A9|\
\\x00AE|\
[\\x2002-\\x2005]|\
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
[\\x25FB-\\x25FE]|\
[\\x2600-\\x2601]|\
\\x260E|\
\\x2611|\
[\\x2614-\\x2615]|\
\\x261D|\
\\x263A|\
[\\x2648-\\x2653]|\
\\x2660|\
\\x2663|\
[\\x2665-\\x2666]|\
\\x2668|\
\\x267B|\
\\x267F|\
\\x2693|\
[\\x26A0-\\x26A1]|\
[\\x26AA-\\x26AB]|\
\\x26BD|\
\\x26BE|\
[\\x26C4-\\x26C5]|\
\\x26CE|\
\\x26D4|\
\\x26EA|\
[\\x26F2-\\x26F3]|\
\\x26F5|\
\\x26FA|\
\\x26FD|\
\\x2702|\
\\x2705|\
[\\x2708-\\x270C]|\
\\x270F|\
\\x2712|\
\\x2714|\
\\x2716|\
\\x2728|\
[\\x2733-\\x2734]|\
\\x2744|\
\\x2747|\
\\x274C|\
\\x274E|\
[\\x2753-\\x2755]|\
\\x2757|\
\\x2764|\
[\\x2795-\\x2797]|\
\\x27A1|\
\\x27B0|\
[\\x2934-\\x2935]|\
[\\x2B05-\\x2B07]|\
[\\x2B1B-\\x2B1C]|\
\\x2B50|\
\\x2B55|\
\\x3030|\
\\x303D|\
\\x3297|\
\\x3299|\
\\xD83C\\xDC04|\
\\xD83C\\xDCCF|\
\\xD83C[\\xDD70-\\xDD71]|\
\\xD83C[\\xDD7E-\\xDD7F]|\
\\xD83C\\xDC8E|\
\\xD83C[\\xDD91-\\xDD91]|\
\\xD83C[\\xDE01-\\xDE02]|\
\\xD83C\\xDE1A|\
\\xD83C\\xDE2F|\
\\xD83C[\\xDE32-\\xDE3A]|\
\\xD83C[\\xDE50-\\xDE51]|\
\\xD83C[\\xDF00-\\xDF0C]|\
\\xD83C\\xDF0F|\
\\xD83C\\xDF11|\
\\xD83C[\\xDF13-\\xDF15]|\
\\xD83C\\xDF19|\
\\xD83C\\xDF1B|\
\\xD83C\\xDF1F|\
\\xD83C\\xDF20|\
\\xD83C[\\xDF30-\\xDF31]|\
\\xD83C[\\xDF34-\\xDF35]|\
\\xD83C[\\xDF37-\\xDF4A]|\
\\xD83C[\\xDF4C-\\xDF4F]|\
\\xD83C[\\xDF51-\\xDF7B]|\
\\xD83C[\\xDF80-\\xDF93]|\
\\xD83C[\\xDFA0-\\xDFC4]|\
\\xD83C\\xDFC6|\
\\xD83C\\xDFC8|\
\\xD83C\\xDFCA|\
\\xD83C[\\xDFE0-\\xDFE3]|\
\\xD83C[\\xDFE5-\\xDFF0]|\
\\xD83D[\\xDC0C-\\xDC0E]|\
\\xD83D[\\xDC11-\\xDC12]|\
\\xD83D\\xDC14|\
\\xD83D[\\xDC17-\\xDC29]|\
\\xD83D[\\xDC2B-\\xDC3E]|\
\\xD83D\\xDC40|\
\\xD83D[\\xDC42-\\xDC64]|\
\\xD83D[\\xDC66-\\xDC6B]|\
\\xD83D[\\xDC6E-\\xDCAC]|\
\\xD83D[\\xDCAE-\\xDCB5]|\
\\xD83D[\\xDCB8-\\xDCEB]|\
\\xD83D\\xDCEE|\
\\xD83D[\\xDCF0-\\xDCF4]|\
\\xD83D[\\xDCF6-\\xDCF7]|\
\\xD83D[\\xDCF9-\\xDCFC]|\
\\xD83D\\xDD03|\
\\xD83D[\\xDD0A-\\xDD14]|\
\\xD83D[\\xDD16-\\xDD2B]|\
\\xD83D[\\xDD2E-\\xDD3D]|\
\\xD83D[\\xDD50-\\xDD5B]|\
\\xD83D[\\xDDFB-\\xDDFF]|\
\\xD83D[\\xDE01-\\xDE06]|\
\\xD83D[\\xDE09-\\xDE0D]|\
\\xD83D[\\xDE0F|\
\\xD83D[\\xDE12-\\xDE14]|\
\\xD83D[\\xDE16|\
\\xD83D[\\xDE18|\
\\xD83D[\\xDE1A|\
\\xD83D[\\xDE1C-\\xDE1E]|\
\\xD83D[\\xDE20-\\xDE25]|\
\\xD83D[\\xDE28-\\xDE2B]|\
\\xD83D[\\xDE2D|\
\\xD83D[\\xDE30-\\xDE33]|\
\\xD83D[\\xDE35|\
\\xD83D[\\xDE37-\\xDE40]|\
\\xD83D[\\xDE45-\\xDE4F]|\
\\xD83D[\\xDE80|\
\\xD83D[\\xDE83-\\xDE85]|\
\\xD83D[\\xDE87|\
\\xD83D[\\xDE89|\
\\xD83D[\\xDE8C|\
\\xD83D[\\xDE8F|\
\\xD83D[\\xDE91-\\xDE93]|\
\\xD83D[\\xDE95|\
\\xD83D[\\xDE97|\
\\xD83D[\\xDE99-\\xDE9A]|\
\\xD83D[\\xDEA2|\
\\xD83D[\\xDEA4-\\xDEA5]|\
\\xD83D[\\xDEA7-\\xDEAD]|\
\\xD83D\\xDEB2|\
\\xD83D\\xDEB6|\
\\xD83D[\\xDEB9-\\xDEBE]|\
\\xD83D\\xDEC0)");

    QTextCursor c = doc->find(rx,0);

    while (!c.isNull()) {
        QTextCharFormat format;
        format.setFont(settings.getEmojiFont());
        c.mergeCharFormat(format);

        // Find next
        c = doc->find(rx,c);
    }

    // For later use of QRegularExpression
    // Better maintainable, because you can set the unicode ID instead of UTF16 characters
    /*QRegularExpression rx("([\\x{0023}          |\
                          \\x{0030}-\\x{0039}|\
                          \\x{00A9}          |\
                          \\x{00AE}          |\
                          \\x{2002}-\\x{2005}|\
                          \\x{203C}          |\
                          \\x{2049}          |\
                          \\x{2122}          |\
                          \\x{2139}          |\
                          \\x{2194}-\\x{2199}|\
                          \\x{21A9}-\\x{21AA}|\
                          \\x{231A}-\\x{231B}|\
                          \\x{23E9}-\\x{23EC}|\
                          \\x{23F0}          |\
                          \\x{23F3}          |\
                          \\x{24C2}          |\
                          \\x{25AA}-\\x{25AB}|\
                          \\x{25B6}          |\
                          \\x{25C0}          |\
                          \\x{25FB}-\\x{25FE}|\
                          \\x{2600}-\\x{2601}|\
                          \\x{260E}          |\
                          \\x{2611}          |\
                          \\x{2614}-\\x{2615}|\
                          \\x{261D}          |\
                          \\x{263A}          |\
                          \\x{2648}-\\x{2653}|\
                          \\x{2660}          |\
                          \\x{2663}          |\
                          \\x{2665}-\\x{2666}|\
                          \\x{2668}          |\
                          \\x{267B}          |\
                          \\x{267F}          |\
                          \\x{2693}          |\
                          \\x{26A0}-\\x{26A1}|\
                          \\x{26AA}-\\x{26AB}|\
                          \\x{26BD}          |\
                          \\x{26BE}          |\
                          \\x{26C4}-\\x{26C5}|\
                          \\x{26CE}          |\
                          \\x{26D4}          |\
                          \\x{26EA}          |\
                          \\x{26F2}-\\x{26F3}|\
                          \\x{26F5}          |\
                          \\x{26FA}          |\
                          \\x{26FD}          |\
                          \\x{2702}          |\
                          \\x{2705}          |\
                          \\x{2708}-\\x{270C}|\
                          \\x{270F}          |\
                          \\x{2712}          |\
                          \\x{2714}          |\
                          \\x{2716}          |\
                          \\x{2728}          |\
                          \\x{2733}-\\x{2734}|\
                          \\x{2744}          |\
                          \\x{2747}          |\
                          \\x{274C}          |\
                          \\x{274E}          |\
                          \\x{2753}-\\x{2755}|\
                          \\x{2757}          |\
                          \\x{2764}          |\
                          \\x{2795}-\\x{2797}|\
                          \\x{27A1}          |\
                          \\x{27B0}          |\
                          \\x{2934}-\\x{2935}|\
                          \\x{2B05}-\\x{2B07}|\
                          \\x{2B1B}-\\x{2B1C}|\
                          \\x{2B50}          |\
                          \\x{2B55}          |\
                          \\x{3030}          |\
                          \\x{303D}          |\
                          \\x{3297}          |\
                          \\x{3299}          |\
                          \\x{1F004}           |\
                          \\x{1F0CF}           |\
                          \\x{1F170}-\\x{1F171}|\
                          \\x{1F17E}-\\x{1F17F}|\
                          \\x{1F18E}           |\
                          \\x{1F191}-\\x{1F19A}|\
                          \\x{1F201}-\\x{1F202}|\
                          \\x{1F21A}           |\
                          \\x{1F22F}           |\
                          \\x{1F232}-\\x{1F23A}|\
                          \\x{1F250}-\\x{1F251}|\
                          \\x{1F300}-\\x{1F30C}|\
                          \\x{1F30F}           |\
                          \\x{1F311}           |\
                          \\x{1F313}-\\x{1F315}|\
                          \\x{1F319}           |\
                          \\x{1F31B}           |\
                          \\x{1F31F}           |\
                          \\x{1F320}           |\
                          \\x{1F330}-\\x{1F331}|\
                          \\x{1F334}-\\x{1F335}|\
                          \\x{1F337}-\\x{1F34A}|\
                          \\x{1F34C}-\\x{1F34F}|\
                          \\x{1F351}-\\x{1F37B}|\
                          \\x{1F380}-\\x{1F393}|\
                          \\x{1F3A0}-\\x{1F3C4}|\
                          \\x{1F3C6}           |\
                          \\x{1F3C8}           |\
                          \\x{1F3CA}           |\
                          \\x{1F3E0}-\\x{1F3E3}|\
                          \\x{1F3E5}-\\x{1F3F0}|\
                          \\x{1F40C}-\\x{1F40E}|\
                          \\x{1F411}-\\x{1F412}|\
                          \\x{1F414}           |\
                          \\x{1F417}-\\x{1F429}|\
                          \\x{1F42B}-\\x{1F43E}|\
                          \\x{1F440}           |\
                          \\x{1F442}-\\x{1F464}|\
                          \\x{1F466}-\\x{1F46B}|\
                          \\x{1F46E}-\\x{1F4AC}|\
                          \\x{1F4AE}-\\x{1F4B5}|\
                          \\x{1F4B8}-\\x{1F4EB}|\
                          \\x{1F4EE}           |\
                          \\x{1F4F0}-\\x{1F4F4}|\
                          \\x{1F4F6}-\\x{1F4F7}|\
                          \\x{1F4F9}-\\x{1F4FC}|\
                          \\x{1F503}           |\
                          \\x{1F50A}-\\x{1F514}|\
                          \\x{1F516}-\\x{1F52B}|\
                          \\x{1F52E}-\\x{1F53D}|\
                          \\x{1F550}-\\x{1F55B}|\
                          \\x{1F5FB}-\\x{1F5FF}|\
                          \\x{1F601}-\\x{1F606}|\
                          \\x{1F609}-\\x{1F60D}|\
                          \\x{1F60F}           |\
                          \\x{1F612}-\\x{1F614}|\
                          \\x{1F616}           |\
                          \\x{1F618}           |\
                          \\x{1F61A}           |\
                          \\x{1F61C}-\\x{1F61E}|\
                          \\x{1F620}-\\x{1F625}|\
                          \\x{1F628}-\\x{1F62B}|\
                          \\x{1F62D}           |\
                          \\x{1F630}-\\x{1F633}|\
                          \\x{1F635}           |\
                          \\x{1F637}-\\x{1F640}|\
                          \\x{1F645}-\\x{1F64F}|\
                          \\x{1F680}           |\
                          \\x{1F683}-\\x{1F685}|\
                          \\x{1F687}           |\
                          \\x{1F689}           |\
                          \\x{1F68C}           |\
                          \\x{1F68F}           |\
                          \\x{1F691}-\\x{1F693}|\
                          \\x{1F695}           |\
                          \\x{1F697}           |\
                          \\x{1F699}-\\x{1F69A}|\
                          \\x{1F6A2}           |\
                          \\x{1F6A4}-\\x{1F6A5}|\
                          \\x{1F6A7}-\\x{1F6AD}|\
                          \\x{1F6B2}           |\
                          \\x{1F6B6}           |\
                          \\x{1F6B9}-\\x{1F6BE}|\
                          \\x{1F6C0}])");*/
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
