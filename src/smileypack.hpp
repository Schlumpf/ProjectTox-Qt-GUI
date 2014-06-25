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

#ifndef SMILEYPACK_HPP
#define SMILEYPACK_HPP

#include <QObject>
#include <QPair>
#include <QStringList>
#include "messages/smiley.hpp"


class Smileypack : public QObject
{
    Q_OBJECT
public:
    typedef QList<QPair<QString, QStringList>> SmileypackList;

    explicit Smileypack(QObject *parent = 0);
    void operator=(const Smileypack &other);

    bool parseFile(const QString &filePath);

    inline const QString &getThemeFile() const   { return themeFile;   }
    inline void setThemeFile(const QString &x)   { themeFile = x;      }
    inline const QString &getName() const        { return name;        }
    inline void setName(const QString &x)        { name = x;           }
    inline const QString &getAuthor() const      { return author;      }
    inline void setAuthor(const QString &x)      { author = x;         }
    inline const QString &getDescription() const { return description; }
    inline void setDescription(const QString &x) { description = x;    }
    inline const QString &getVersion() const     { return version;     }
    inline void setVersion(const QString &x)     { version = x;        }
    inline const QString &getWebsite() const     { return website;     }
    inline void setWebsite(const QString &x)     { website = x;        }
    inline const QString &getIcon() const        { return icon;        }
    inline void setIcon(const QString &x)        { icon = x;           }
    inline const SmileypackList &getList() const { return list;        }
    inline void setList(const SmileypackList &x) { list = x;           }

    static QString desmilify(QString htmlText); // and deemojifiy
    static QString deemojify(QString text);
    //static QString resizeEmoji(QString text);
    static const SmileypackList emojiList();

    static Smileypack &currentPack();

public slots:
    void updatePack();

private:
    QString themeFile;
    QString name;
    QString author;
    QString description;
    QString version;
    QString website;
    QString icon;
    SmileypackList list;

    // Parser functions
    enum ParserStates {
        StateHead,
        StateSmileys
    };
    void processLine(const QString &xLine, const QString &xPath, ParserStates &xState);
};


#endif // SMILEYPACK_HPP
