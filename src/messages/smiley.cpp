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

#include "smiley.hpp"
#include <smileypack.hpp>
#include "Settings/settings.hpp"
#include <QMapIterator>
#include <QDebug>
#include <QApplication>

#include "clickable.hpp"

Smiley::Smiley(const QString &text, const QString &graphics, int start, int smileyfiedStart, Type type)
{
    mType = type;
    mStart = start;
    mSmiletfiedStart = smileyfiedStart;
    mText = text;
    mTextLength = text.length();
    mGraphics = graphics;
}

// Find smileys with original positions
SmileyList SmileyList::fromText(QString text)
{
    // Get current smileypack
    const Settings &settings = Settings::getInstance();

    // Reconvert emoji
    text = Smileypack::deemojify(text);

    SmileyList result;
    ClickableList clickables = ClickableList::fromString(text);

    // Whlie smileys found to replace
    bool found;
    int searchStart = 0;
    int offset = 0;
    do {
        found = false;

        // Fill a map with positions of possible smileys
        QMap<int, QStringList> possibleTexts;

        Smileypack::SmileypackList smileyList;
        if (settings.getSmileyType() == Smiley::Emoji)
            smileyList = Smileypack::emojiList();
        else
            smileyList = Smileypack::currentPack().getList();

        for (const auto& pair : smileyList) {
            for (const QString& smileytext : pair.second) {
                int pos = text.indexOf(smileytext, searchStart);
                if (pos > -1) {
                    possibleTexts.insertMulti(pos, {smileytext, pair.first});
                    found = true;
                }
            }
        }

        // check the first smiley alternative representations
        QMapIterator<int, QStringList> first(possibleTexts);
        if (first.hasNext()) {
            first.next();

            int length = first.value().first().count();
            QString repSrt = first.value().first();
            int     repPos = first.key();
            QString repRep = first.value().at(1);

            QMapIterator<int, QStringList> i(possibleTexts);
            i.next();

            // Search for a longer smileyrepresentation at same position
            while (i.hasNext() && i.key() < first.key() + length) {
                i.next();

                // If there is a longer smileyrepresentation, use it
                if (i.value().count() > length) {
                    repPos = i.key();
                    repSrt = i.value().first();
                    repRep = i.value().at(1);
                }
            }

            // Check if there is a clickable
            if(!clickables.atCursorPos(repPos).isValid()) {

                // Add found smiley to List
                result.append(Smiley(repSrt, repRep, repPos, repPos - offset, (Smiley::Type) settings.getSmileyType()));

                // calculate offset for next smiley
                offset += repSrt.count() - ((settings.getSmileyType() == Smiley::Emoji) ? repRep.count() : 1);
            }
            searchStart = repPos + repSrt.count();
        }
    } while (found);

    return result;
}


QDebug operator<<(QDebug dbg, const Smiley &smiley)
{
    dbg.nospace() << qPrintable(QString("%1 [%2-%3][%4-%5] %6").arg(smiley.text(), 3)
                                                        .arg(smiley.start(), 3)
                                                        .arg(smiley.start()+smiley.textLength()-1, 3)
                                                        .arg(smiley.smileyfiedStart(), 3)
                                                        .arg(smiley.smileyfiedStart()+((smiley.type() == Smiley::Emoji) ? smiley.graphics().count() : 1 )-1, 3)
                                                        .arg(smiley.graphics()));
    return dbg;
}
