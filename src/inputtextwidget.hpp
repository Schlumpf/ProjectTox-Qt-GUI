/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
                  2013-2014 by Martin Kröll <technikschlumpf@web.de>
    
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

#ifndef INPUTTEXTWIDGET_HPP
#define INPUTTEXTWIDGET_HPP

#include <QTextEdit>
#include <QTimer>
#include <messages/smiley.hpp>

class InputTextWidget : public QTextEdit
{
    Q_OBJECT
public:
    InputTextWidget(QWidget* parent);
    QSize sizeHint() const;

public slots:
    void insertSmiley(const Smiley &smiley);

protected:
    void keyPressEvent(QKeyEvent* event);
    void focusOutEvent(QFocusEvent * event);

signals:
    void sendMessage(const QString& message);
    void sendAction(const QString& action);
    void sendTyping(bool typing);

private slots:
    void showContextMenu(const QPoint &pos);
    void copyPlainText();
    void pastePlainText();
    void cutPlainText();
    void endTyping();
    void startTyping();
    void resetFormat();

private:
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;

    bool   mTyping;
    QTimer mTypingTimer;
};

#endif // INPUTTEXTWIDGET_HPP
