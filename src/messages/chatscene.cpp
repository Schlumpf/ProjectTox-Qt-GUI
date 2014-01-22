#include "chatscene.hpp"
#include "message.hpp"
#include <QDrag>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QKeySequence>
#include "chatitem.hpp"
#include "chatline.hpp"
#include "markerlineitem.hpp"
#include "columnhandleitem.hpp"
#include <QApplication>
#include <QMimeData>
#include <QGraphicsSceneContextMenuEvent>
#include "chatview.hpp"

const qreal minContentsWidth = 100;

ChatScene::ChatScene(QAbstractItemModel *model, qreal width, ChatView *parent) :
    QGraphicsScene(0, 0, width, 0, (QObject *)parent),
    _chatView(parent),
    _model(model),
    _sceneRect(0, 0, width, 0),
    _firstLineRow(-1),
    _viewportHeight(0),
    _markerLine(new MarkerLineItem(width)),
    _markerLineVisible(false),
    _markerLineValid(false),
    _markerLineJumpPending(false),
    _cutoffMode(CutoffRight),
    _selectingItem(0),
    _selectionStart(-1),
    _isSelecting(false),
    _clickMode(NoClick),
    _clickHandled(true),
    _leftButtonPressed(false)
{
    addItem(_markerLine);
    connect(this, SIGNAL(sceneRectChanged(const QRectF &)), _markerLine, SLOT(sceneRectChanged(const QRectF &)));

    int defaultFirstColHandlePos = 50;
    int defaultSecondColHandlePos = 250;

    _firstColHandlePos = defaultFirstColHandlePos;
    _secondColHandlePos = defaultSecondColHandlePos;

    _firstColHandle = new ColumnHandleItem(6);
    addItem(_firstColHandle);
    _firstColHandle->setXPos(_firstColHandlePos);
    connect(_firstColHandle, SIGNAL(positionChanged(qreal)), this, SLOT(firstHandlePositionChanged(qreal)));
    connect(this, SIGNAL(sceneRectChanged(const QRectF &)), _firstColHandle, SLOT(sceneRectChanged(const QRectF &)));

    _secondColHandle = new ColumnHandleItem(6);
    addItem(_secondColHandle);
    _secondColHandle->setXPos(_secondColHandlePos);
    connect(_secondColHandle, SIGNAL(positionChanged(qreal)), this, SLOT(secondHandlePositionChanged(qreal)));

    connect(this, SIGNAL(sceneRectChanged(const QRectF &)), _secondColHandle, SLOT(sceneRectChanged(const QRectF &)));

    setHandleXLimits();

    if (model->rowCount() > 0)
        rowsInserted(QModelIndex(), 0, model->rowCount() - 1);

    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
        this, SLOT(rowsInserted(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
        this, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
        this, SLOT(rowsRemoved()));
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(dataChanged(QModelIndex, QModelIndex)));

    _clickTimer.setInterval(QApplication::doubleClickInterval());
    _clickTimer.setSingleShot(true);
    connect(&_clickTimer, SIGNAL(timeout()), SLOT(clickTimeout()));

    setItemIndexMethod(QGraphicsScene::NoIndex);
}

int ChatScene::rowByScenePos(qreal y) const
{
    QList<QGraphicsItem *> itemList = items(QPointF(0, y));

    // ChatLine should be at the bottom of the list
    for (int i = itemList.count()-1; i >= 0; i--) {
        ChatLine *line = qgraphicsitem_cast<ChatLine *>(itemList.at(i));
        if (line)
            return line->row();
    }
    return -1;
}

MessageModel::ColumnType ChatScene::columnByScenePos(qreal x) const
{
    if (x < _firstColHandle->x())
        return MessageModel::TimestampColumn;
    if (x < _secondColHandle->x())
        return MessageModel::SenderColumn;

    return MessageModel::ContentsColumn;
}

ChatItem *ChatScene::chatItemAt(const QPointF &scenePos) const
{
    foreach(QGraphicsItem *item, items(scenePos, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder)) {
        ChatLine *line = qgraphicsitem_cast<ChatLine *>(item);
        if (line)
            return line->itemAt(line->mapFromScene(scenePos));
    }
    return 0;
}

//! Find the ChatLine belonging to a MsgId
/** Searches for the ChatLine belonging to a MsgId. If there are more than one ChatLine with the same msgId,
 *  the first one is returned.
 *  Note that this method performs a binary search, hence it has as complexity of O(log n).
 *  If matchExact is false, and we don't have an exact match for the given msgId, we return the visible line right
 *  above the requested one.
 *  \param msgId      The message ID to look for
 *  \param matchExact Whether we find only exact matches
 *  \param ignoreDayChange Whether we ignore day change messages
 *  \return The ChatLine corresponding to the given MsgId */
ChatLine *ChatScene::chatLine(MsgId msgId, bool matchExact, bool ignoreDayChange) const
{
    if (!_lines.count())
            return 0;

        QList<ChatLine *>::ConstIterator start = _lines.begin();
        QList<ChatLine *>::ConstIterator end = _lines.end();
        QList<ChatLine *>::ConstIterator middle;

        int n = int(end - start);
        int half;

        while (n > 0) {
            half = n >> 1;
            middle = start + half;
            if ((*middle)->msgId() < msgId) {
                start = middle + 1;
                n -= half + 1;
            }
            else {
                n = half;
            }
        }

        if (start != end && (*start)->msgId() == msgId && (ignoreDayChange ? (*start)->msgType() != Message::DayChange : true))
            return *start;

        if (matchExact)
            return 0;

        if (start == _lines.begin()) // not (yet?) in our scene
            return 0;

        // if we didn't find the exact msgId, take the next-lower one (this makes sense for lastSeen)

        if (start == end) { // higher than last element
            if (!ignoreDayChange)
                return _lines.last();

            for (int i = _lines.count() -1; i >= 0; i--) {
                if (_lines.at(i)->msgType() != Message::DayChange)
                    return _lines.at(i);
            }
            return 0;
        }

        // return the next-lower line
        if (!ignoreDayChange)
            return *(--start);

        do {
            if ((*(--start))->msgType() != Message::DayChange)
                return *start;
        }
        while (start != _lines.begin());
        return 0;
}

//!\brief Convert current selection to human-readable string.
QString ChatScene::selection() const
{
    //TODO Make selection format configurable!
    if (hasGlobalSelection()) {
        int start = qMin(_selectionStart, _selectionEnd);
        int end = qMax(_selectionStart, _selectionEnd);
        if (start < 0 || end >= _lines.count()) {
            qDebug() << "Invalid selection range:" << start << end;
            return QString();
        }
        QString result;
        for (int l = start; l <= end; l++) {
            if (_selectionMinCol == MessageModel::TimestampColumn)
                result += _lines[l]->item(MessageModel::TimestampColumn)->data(MessageModel::DisplayRole).toString() + " ";
            if (_selectionMinCol <= MessageModel::SenderColumn)
                result += _lines[l]->item(MessageModel::SenderColumn)->data(MessageModel::DisplayRole).toString() + " ";
            result += _lines[l]->item(MessageModel::ContentsColumn)->data(MessageModel::DisplayRole).toString() + "\n";
        }
        return result;
    }
    else if (selectingItem())
        return selectingItem()->selection();
    return QString();
}

bool ChatScene::hasSelection() const
{
    return hasGlobalSelection() || (selectingItem() && selectingItem()->hasSelection());
}

bool ChatScene::isPosOverSelection(const QPointF &pos) const
{
    ChatItem *chatItem = chatItemAt(pos);
    if (!chatItem)
        return false;
    if (hasGlobalSelection()) {
        int row = chatItem->row();
        if (row >= qMin(_selectionStart, _selectionEnd) && row <= qMax(_selectionStart, _selectionEnd))
            return columnByScenePos(pos) >= _selectionMinCol;
    }
    else {
        return chatItem->isPosOverSelection(chatItem->mapFromScene(pos));
    }
    return false;
}

void ChatScene::initiateDrag(QWidget *source)
{
    QDrag *drag = new QDrag(source);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(selection());
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);
}

bool ChatScene::isScrollingAllowed() const
{
    if (_isSelecting)
            return false;
        // TODO: Handle clicks and single-item selections too
    return true;
}

void ChatScene::updateForViewport(qreal width, qreal height)
{
    _viewportHeight = height;
    setWidth(width);
}

void ChatScene::setWidth(qreal width)
{
    if (width == _sceneRect.width())
        return;
    layout(0, _lines.count()-1, width);
}

void ChatScene::layout(int start, int end, qreal width)
{
    if (end >= 0) {
        int row = end;
        qreal linePos = _lines.at(row)->scenePos().y() + _lines.at(row)->height();
        qreal thirdWidth = width - secondColumnHandle()->sceneRight();
        while (row >= start) {
            _lines.at(row--)->setGeometryByWidth(width, thirdWidth, linePos);
        }

        if (row >= 0) {
            // remaining items don't need geometry changes, but maybe repositioning?
            ChatLine *line = _lines.at(row);
            qreal offset = linePos - (line->scenePos().y() + line->height());
            if (offset != 0) {
                while (row >= 0) {
                    line = _lines.at(row--);
                    line->setPos(0, line->scenePos().y() + offset);
                }
            }
        }
    }

    //setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    updateSceneRect(width);
    setHandleXLimits();
    setMarkerLine();
    emit layoutChanged();
}

void ChatScene::setMarkerLineVisible(bool visible)
{
    _markerLineVisible = visible;
    if (visible && _markerLineValid)
        markerLine()->setVisible(true);
    else
        markerLine()->setVisible(false);
}

void ChatScene::setMarkerLine(MsgId msgId)
{
    // TODO MKO
    /*
    if (!msgId.isValid())
        msgId = Client::markerLine(singleBufferId());
    */

    //if (msgId.isValid()) {
        ChatLine *line = chatLine(msgId, false, true);
        if (line) {
            markerLine()->setChatLine(line);
            // if this was the last line, we won't see it because it's outside the sceneRect
            // .. which is exactly what we want :)
            markerLine()->setPos(line->pos() + QPointF(0, line->height()));

            // DayChange messages might have been hidden outside the scene rect, don't make the markerline visible then!
            if (markerLine()->pos().y() >= sceneRect().y()) {
                _markerLineValid = true;
                if (_markerLineVisible)
                    markerLine()->setVisible(true);
                if (_markerLineJumpPending) {
                    _markerLineJumpPending = false;
                    if (markerLine()->isVisible()) {
                        markerLine()->ensureVisible(QRectF(), 50, 50);
                    }
                }
                return;
            }
        }
    //}
    _markerLineValid = false;
    markerLine()->setVisible(false);
}

void ChatScene::jumpToMarkerLine()
{
    if (markerLine()->isVisible()) {
        markerLine()->ensureVisible(QRectF(), 50, 50);
        return;
    }
}

void ChatScene::setSelectingItem(ChatItem *item)
{
    if (_selectingItem) _selectingItem->clearSelection();
    _selectingItem = item;
}

void ChatScene::startGlobalSelection(ChatItem *item, const QPointF &itemPos)
{
    _selectionStart = _selectionEnd = _firstSelectionRow = item->row();
    _selectionStartCol = _selectionMinCol = item->column();
    _isSelecting = true;
    _lines[_selectionStart]->setSelected(true, (MessageModel::ColumnType)_selectionMinCol);
    updateSelection(item->mapToScene(itemPos));
}

void ChatScene::clearGlobalSelection()
{
    if (hasGlobalSelection()) {
        for (int l = qMin(_selectionStart, _selectionEnd); l <= qMax(_selectionStart, _selectionEnd); l++)
            _lines[l]->setSelected(false);
        _isSelecting = false;
        _selectionStart = -1;
    }
}

void ChatScene::clearSelection()
{
    clearGlobalSelection();
    if (selectingItem())
        selectingItem()->clearSelection();
}

void ChatScene::selectionToClipboard(QClipboard::Mode mode)
{
    if (!hasSelection())
            return;

    stringToClipboard(selection(), mode);
}

void ChatScene::stringToClipboard(const QString &str_, QClipboard::Mode mode)
{
    QString str = str_;
    // remove trailing linefeeds
    if (str.endsWith('\n'))
        str.chop(1);

    switch (mode) {
    case QClipboard::Clipboard:
        QApplication::clipboard()->setText(str);
        break;
    case QClipboard::Selection:
        if (QApplication::clipboard()->supportsSelection())
            QApplication::clipboard()->setText(str, QClipboard::Selection);
        break;
    default:
        break;
    };
}

void ChatScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QPointF pos = event->scenePos();
    QMenu menu;

    if (isPosOverSelection(pos))
        menu.addAction(tr("Copy Selection"),
                       this, SLOT(selectionToClipboard()),
                       QKeySequence::Copy);

    // item-specific options (select link etc)
    ChatItem *item = chatItemAt(pos);
    if (item)
        item->addActionsToMenu(&menu, item->mapFromScene(pos));

    menu.exec(event->screenPos());
}

void ChatScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        if (!_clickHandled && (event->scenePos() - _clickPos).toPoint().manhattanLength() >= QApplication::startDragDistance()) {
            if (_clickTimer.isActive())
                _clickTimer.stop();
            if (_clickMode == SingleClick && isPosOverSelection(_clickPos))
                initiateDrag(event->widget());
            else {
                _clickMode = DragStartClick;
                handleClick(Qt::LeftButton, _clickPos);
            }
            _clickMode = NoClick;
        }
        if (_isSelecting) {
            updateSelection(event->scenePos());
            emit mouseMoveWhileSelecting(event->scenePos());
            event->accept();
        }
        else if (_clickHandled && _clickMode < DoubleClick)
            QGraphicsScene::mouseMoveEvent(event);
    }
    else
        QGraphicsScene::mouseMoveEvent(event);
}

void ChatScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        _leftButtonPressed = true;
        _clickHandled = false;
        if (!isPosOverSelection(event->scenePos())) {
            // immediately clear selection if clicked outside; otherwise, wait for potential drag
            clearSelection();
        }
        if (_clickMode != NoClick && _clickTimer.isActive()) {
            switch (_clickMode) {
            case NoClick:
                _clickMode = SingleClick; break;
            case SingleClick:
                _clickMode = DoubleClick; break;
            case DoubleClick:
                _clickMode = TripleClick; break;
            case TripleClick:
                _clickMode = DoubleClick; break;
            case DragStartClick:
                break;
            }
            handleClick(Qt::LeftButton, _clickPos);
        }
        else {
            _clickMode = SingleClick;
            _clickPos = event->scenePos();
        }
        _clickTimer.start();
    }
    if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        QGraphicsScene::mouseDoubleClickEvent(event);
    else
        QGraphicsScene::mousePressEvent(event);
}

void ChatScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && _leftButtonPressed) {
        _leftButtonPressed = false;
        if (_clickMode != NoClick) {
            if (_clickMode == SingleClick)
                clearSelection();
            event->accept();
            if (!_clickTimer.isActive())
                handleClick(Qt::LeftButton, _clickPos);
        }
        else {
            // no click -> drag or selection move
            if (isGloballySelecting()) {
                selectionToClipboard(QClipboard::Selection);
                _isSelecting = false;
                event->accept();
                return;
            }
        }
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void ChatScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // we check for doubleclick ourselves, so just call press handler
    mousePressEvent(event);
}

void ChatScene::handleClick(Qt::MouseButton button, const QPointF &scenePos)
{
    if (button == Qt::LeftButton) {
        clearSelection();

        // Now send click down to items
        ChatItem *chatItem = chatItemAt(scenePos);
        if (chatItem) {
            chatItem->handleClick(chatItem->mapFromScene(scenePos), _clickMode);
        }
        _clickHandled = true;
    }
}

void ChatScene::rowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    qreal h = 0;
    qreal y = 0;
    qreal width = _sceneRect.width();
    bool atBottom = (start == _lines.count());
    bool atTop = !atBottom && (start == 0);

    if (start < _lines.count()) {
        y = _lines.value(start)->y();
    }
    else if (atBottom && !_lines.isEmpty()) {
        y = _lines.last()->y() + _lines.last()->height();
    }

    qreal contentsWidth = width - secondColumnHandle()->sceneRight();
    qreal senderWidth = secondColumnHandle()->sceneLeft() - firstColumnHandle()->sceneRight();
    qreal timestampWidth = firstColumnHandle()->sceneLeft();
    QPointF contentsPos(secondColumnHandle()->sceneRight(), 0);
    QPointF senderPos(firstColumnHandle()->sceneRight(), 0);

    if (atTop) {
        for (int i = end; i >= start; i--) {
            ChatLine *line = new ChatLine(i, model(),
                                          width,
                                          timestampWidth, senderWidth, contentsWidth,
                                          senderPos, contentsPos);
            h += line->height();
            line->setPos(0, y-h);
            _lines.insert(start, line);
            addItem(line);
        }
    }
    else {
        for (int i = start; i <= end; i++) {
            ChatLine *line = new ChatLine(i, model(),
                                          width,
                                          timestampWidth, senderWidth, contentsWidth,
                                          senderPos, contentsPos);
            line->setPos(0, y+h);
            h += line->height();
            _lines.insert(i, line);
            addItem(line);
        }
    }

    // update existing items
    for (int i = end+1; i < _lines.count(); i++) {
        _lines[i]->setRow(i);
    }

    // update selection
    if (_selectionStart >= 0) {
        int offset = end - start + 1;
        int oldStart = _selectionStart;
        if (_selectionStart >= start)
            _selectionStart += offset;
        if (_selectionEnd >= start) {
            _selectionEnd += offset;
            if (_selectionStart == oldStart)
                for (int i = start; i < start + offset; i++)
                    _lines[i]->setSelected(true);
        }
        if (_firstSelectionRow >= start)
            _firstSelectionRow += offset;
    }

    // neither pre- or append means we have to do dirty work: move items...
    if (!(atTop || atBottom)) {
        ChatLine *line = 0;
        for (int i = 0; i <= end; i++) {
            line = _lines.at(i);
            line->setPos(0, line->pos().y() - h);
            if (line == markerLine()->chatLine())
                markerLine()->setPos(line->pos() + QPointF(0, line->height()));
        }
    }

    // check if all went right
    Q_ASSERT(start == 0 || _lines.at(start - 1)->pos().y() + _lines.at(start - 1)->height() == _lines.at(start)->pos().y());
    Q_ASSERT(end + 1 == _lines.count() || _lines.at(end)->pos().y() + _lines.at(end)->height() == _lines.at(end + 1)->pos().y());

    if (!atBottom) {
        if (start < _firstLineRow) {
            int prevFirstLineRow = _firstLineRow + (end - start + 1);
            for (int i = end + 1; i < prevFirstLineRow; i++) {
                _lines.at(i)->show();
            }
        }
        // force new search for first proper line
        _firstLineRow = -1;
    }
    updateSceneRect();
    if (atBottom) {
        emit lastLineChanged(_lines.last(), h);
    }

    // now move the marker line if necessary. we don't need to do anything if we appended lines though...
    if (!_markerLineValid)
        setMarkerLine();
}

void ChatScene::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);

    qreal h = 0; // total height of removed items;

    bool atTop = (start == 0);
    bool atBottom = (end == _lines.count() - 1);

    // clear selection
    if (_selectingItem) {
        int row = _selectingItem->row();
        if (row >= start && row <= end)
            setSelectingItem(0);
    }

    // remove items from scene
    QList<ChatLine *>::iterator lineIter = _lines.begin() + start;
    int lineCount = start;
    while (lineIter != _lines.end() && lineCount <= end) {
        if ((*lineIter) == markerLine()->chatLine())
            markerLine()->setChatLine(0);
        h += (*lineIter)->height();
        delete *lineIter;
        lineIter = _lines.erase(lineIter);
        lineCount++;
    }

    // update rows of remaining chatlines
    for (int i = start; i < _lines.count(); i++) {
        _lines.at(i)->setRow(i);
    }

    // update selection
    if (_selectionStart >= 0) {
        int offset = end - start + 1;
        if (_selectionStart >= start)
            _selectionStart = qMax(_selectionStart - offset, start);
        if (_selectionEnd >= start)
            _selectionEnd -= offset;
        if (_firstSelectionRow >= start)
            _firstSelectionRow -= offset;

        if (_selectionEnd < _selectionStart) {
            _isSelecting = false;
            _selectionStart = -1;
        }
    }

    // neither removing at bottom or top means we have to move items...
    if (!(atTop || atBottom)) {
        qreal offset = h;
        int moveStart = 0;
        int moveEnd = _lines.count() - 1;
        if (start < _lines.count() - start) {
            // move top part
            moveEnd = start - 1;
        }
        else {
            // move bottom part
            moveStart = start;
            offset = -offset;
        }
        ChatLine *line = 0;
        for (int i = moveStart; i <= moveEnd; i++) {
            line = _lines.at(i);
            line->setPos(0, line->pos().y() + offset);
        }
    }

    Q_ASSERT(start == 0 || start >= _lines.count() || _lines.at(start - 1)->pos().y() + _lines.at(start - 1)->height() == _lines.at(start)->pos().y());

    // update sceneRect
    // when searching for the first non-date-line we have to take into account that our
    // model still contains the just removed lines so we cannot simply call updateSceneRect()
    int numRows = model()->rowCount();
    QModelIndex firstLineIdx;
    _firstLineRow = -1;
    bool needOffset = false;
    do {
        _firstLineRow++;
        if (_firstLineRow >= start && _firstLineRow <= end) {
            _firstLineRow = end + 1;
            needOffset = true;
        }
        firstLineIdx = model()->index(_firstLineRow, 0);
    }
    while ((Message::Type)(model()->data(firstLineIdx, MessageModel::TypeRole).toInt()) == Message::DayChange && _firstLineRow < numRows);

    if (needOffset)
        _firstLineRow -= end - start + 1;
    updateSceneRect();
}

void ChatScene::dataChanged(const QModelIndex &tl, const QModelIndex &br)
{
    layout(tl.row(), br.row(), _sceneRect.width());
}

void ChatScene::firstHandlePositionChanged(qreal xpos)
{
    if (_firstColHandlePos == xpos)
            return;

        _firstColHandlePos = xpos >= 0 ? xpos : 0;
        // clock_t startT = clock();

        // disabling the index while doing this complex updates is about
        // 2 to 10 times faster!
        //setItemIndexMethod(QGraphicsScene::NoIndex);

        QList<ChatLine *>::iterator lineIter = _lines.end();
        QList<ChatLine *>::iterator lineIterBegin = _lines.begin();
        qreal linePos = _sceneRect.y() + _sceneRect.height();
        qreal firstColumnWidth = firstColumnHandle()->sceneLeft();
        qreal secondColumnWidth = secondColumnHandle()->sceneLeft() - firstColumnHandle()->sceneRight();
        QPointF secondColumnPos(firstColumnHandle()->sceneRight(), 0);

        while (lineIter != lineIterBegin) {
            lineIter--;
            (*lineIter)->setFirstColumn(firstColumnWidth, secondColumnWidth, secondColumnPos, linePos);
        }
        //setItemIndexMethod(QGraphicsScene::BspTreeIndex);

        setHandleXLimits();
}

void ChatScene::secondHandlePositionChanged(qreal xpos)
{
    if (_secondColHandlePos == xpos)
        return;

    _secondColHandlePos = xpos;
    // clock_t startT = clock();

    // disabling the index while doing this complex updates is about
    // 2 to 10 times faster!
    //setItemIndexMethod(QGraphicsScene::NoIndex);

    QList<ChatLine *>::iterator lineIter = _lines.end();
    QList<ChatLine *>::iterator lineIterBegin = _lines.begin();
    qreal linePos = _sceneRect.y() + _sceneRect.height();
    qreal secondColumnWidth = secondColumnHandle()->sceneLeft() - firstColumnHandle()->sceneRight();
    qreal thirdColumnWidth = _sceneRect.width() - secondColumnHandle()->sceneRight();
    QPointF thirdColumnPos(secondColumnHandle()->sceneRight(), 0);
    while (lineIter != lineIterBegin) {
        lineIter--;
        (*lineIter)->setSecondColumn(secondColumnWidth, thirdColumnWidth, thirdColumnPos, linePos);
    }
    //setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    updateSceneRect();
    setHandleXLimits();
    emit layoutChanged();
}

void ChatScene::rowsRemoved()
{
    // move the marker line if necessary
    setMarkerLine();
}

void ChatScene::clickTimeout()
{
    if (!_leftButtonPressed && _clickMode == SingleClick)
        handleClick(Qt::LeftButton, _clickPos);
}

void ChatScene::setHandleXLimits()
{
    _firstColHandle->setXLimits(0, _secondColHandle->sceneLeft() - minContentsWidth);
    _secondColHandle->setXLimits(_firstColHandle->sceneRight() + minContentsWidth, width());
    update();
}

void ChatScene::updateSelection(const QPointF &pos)
{
    int curRow = rowByScenePos(pos);
        if (curRow < 0) return;
        int curColumn = (int)columnByScenePos(pos);
        MessageModel::ColumnType minColumn = (MessageModel::ColumnType)qMin(curColumn, _selectionStartCol);
        if (minColumn != _selectionMinCol) {
            _selectionMinCol = minColumn;
            for (int l = qMin(_selectionStart, _selectionEnd); l <= qMax(_selectionStart, _selectionEnd); l++) {
                _lines[l]->setSelected(true, minColumn);
            }
        }
        int newstart = qMin(curRow, _firstSelectionRow);
        int newend = qMax(curRow, _firstSelectionRow);
        if (newstart < _selectionStart) {
            for (int l = newstart; l < _selectionStart; l++)
                _lines[l]->setSelected(true, minColumn);
        }
        if (newstart > _selectionStart) {
            for (int l = _selectionStart; l < newstart; l++)
                _lines[l]->setSelected(false);
        }
        if (newend > _selectionEnd) {
            for (int l = _selectionEnd+1; l <= newend; l++)
                _lines[l]->setSelected(true, minColumn);
        }
        if (newend < _selectionEnd) {
            for (int l = newend+1; l <= _selectionEnd; l++)
                _lines[l]->setSelected(false);
        }

        _selectionStart = newstart;
        _selectionEnd = newend;

        if (newstart == newend && minColumn == MessageModel::ContentsColumn) {
            if (!_selectingItem) {
                // _selectingItem has been removed already
                return;
            }
            _lines[curRow]->setSelected(false);
            _isSelecting = false;
            _selectionStart = -1;
            _selectingItem->continueSelecting(_selectingItem->mapFromScene(pos));
        }
}

void ChatScene::updateSceneRect(qreal width)
{
    if (_lines.isEmpty()) {
        updateSceneRect(QRectF(0, 0, width, 0));
        return;
    }

    // we hide day change messages at the top by making the scene rect smaller
    // and by calling QGraphicsItem::hide() on all leading day change messages
    // the first one is needed to ensure proper scrollbar ranges
    // the second for cases where the viewport is larger then the set scenerect
    //  (in this case the items are shown anyways)
    if (_firstLineRow == -1) {
        int numRows = model()->rowCount();
        _firstLineRow = 0;
        QModelIndex firstLineIdx;
        while (_firstLineRow < numRows) {
            firstLineIdx = model()->index(_firstLineRow, 0);
            if ((Message::Type)(model()->data(firstLineIdx, MessageModel::TypeRole).toInt()) != Message::DayChange)
                break;
            _lines.at(_firstLineRow)->hide();
            _firstLineRow++;
        }
    }

    // the following call should be safe. If it crashes something went wrong during insert/remove
    if (_firstLineRow < _lines.count()) {
        ChatLine *firstLine = _lines.at(_firstLineRow);
        ChatLine *lastLine = _lines.last();
        updateSceneRect(QRectF(0, firstLine->pos().y(), width, lastLine->pos().y() + lastLine->height() - firstLine->pos().y()));
    }
    else {
        // empty scene rect
        updateSceneRect(QRectF(0, 0, width, 0));
    }
}

void ChatScene::updateSceneRect(const QRectF &rect)
{
    _sceneRect = rect;
    setSceneRect(rect);
    update();
}