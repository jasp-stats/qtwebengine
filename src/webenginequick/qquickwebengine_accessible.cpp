// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickwebengine_accessible_p.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QAccessible>

#include "api/qquickwebengineview_p.h"
#include "api/qquickwebengineview_p_p.h"
#include "web_contents_adapter.h"
#include "browser_accessibility_qt.h"

QT_BEGIN_NAMESPACE

QQuickWebEngineViewAccessible::QQuickWebEngineViewAccessible(QQuickWebEngineView *o)
    : QAccessibleObject(o)
{}

bool QQuickWebEngineViewAccessible::isValid() const
{
    if (!QAccessibleObject::isValid())
        return false;

    if (!engineView() || !engineView()->d_func())
        return false;

    return true;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::parent() const
{
    QQuickItem *parent = engineView()->parentItem();
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(parent);
    if (!iface)
        return QAccessible::queryAccessibleInterface(engineView()->window());
    return iface;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::focusChild() const
{
    if (child(0) && child(0)->focusChild())
        return child(0)->focusChild();
    return const_cast<QQuickWebEngineViewAccessible *>(this);
}

int QQuickWebEngineViewAccessible::childCount() const
{
    return child(0) ? 1 : 0;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::child(int index) const
{
    if (index == 0 && isValid())
        return browserAccessible();
    return nullptr;
}

int QQuickWebEngineViewAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (child(0) && c == child(0))
        return 0;
    return -1;
}

QString QQuickWebEngineViewAccessible::text(QAccessible::Text) const
{
    return QString();
}

QAccessible::Role QQuickWebEngineViewAccessible::role() const
{
    return QAccessible::WebDocument;
}

QAccessible::State QQuickWebEngineViewAccessible::state() const
{
    QAccessible::State s;
    if (engineView() && engineView()->hasFocus())
        s.focused = true;
    return s;
}

void *QQuickWebEngineViewAccessible::interface_cast(QAccessible::InterfaceType t)
{
    // Only Text and TableCell interfaces are exposed at the view level;
    // all implementation is delegated to Chromium's accessibility tree.
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface *>(this);
    if (t == QAccessible::TableCellInterface)
        return static_cast<QAccessibleTableCellInterface *>(this);
    return nullptr;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::browserAccessible() const
{
    // Returns the root accessible interface from Chromium's accessibility tree
    // via WebContentsAdapter. This is the bridge from Qt Quick accessibility
    // to the browser's internal accessibility representation.
    if (!isValid())
        return nullptr;
    if (auto *adapter = engineView()->d_func()->adapter.data()) {
        return adapter->browserAccessible();
    }
    return nullptr;
}

QAccessibleTextInterface *QQuickWebEngineViewAccessible::textInterface() const
{
    if (auto browserAcc = browserAccessible())
        return static_cast<QAccessibleTextInterface *>(browserAcc->interface_cast(QAccessible::TextInterface));
    return nullptr;
}

QAccessibleTableCellInterface *QQuickWebEngineViewAccessible::cellInterface() const
{
    if (auto browserAcc = browserAccessible())
        return static_cast<QAccessibleTableCellInterface *>(browserAcc->interface_cast(QAccessible::TableCellInterface));
    return nullptr;
}

void QQuickWebEngineViewAccessible::scrollToSubstring(int startIndex, int endIndex)
{
    if (auto text = textInterface())
        text->scrollToSubstring(startIndex, endIndex);
}

void QQuickWebEngineViewAccessible::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (auto text = textInterface())
        text->setSelection(selectionIndex, startOffset, endOffset);
}

void QQuickWebEngineViewAccessible::addSelection(int startOffset, int endOffset)
{
    if (auto text = textInterface())
        text->addSelection(startOffset, endOffset);
}

void QQuickWebEngineViewAccessible::removeSelection(int selectionIndex)
{
    if (auto text = textInterface())
        text->removeSelection(selectionIndex);
}

void QQuickWebEngineViewAccessible::setCursorPosition(int position)
{
    if (auto text = textInterface())
        text->setCursorPosition(position);
}

QString QQuickWebEngineViewAccessible::attributes(int offset, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->attributes(offset, startOffset, endOffset);
    return QString();
}

QString QQuickWebEngineViewAccessible::textAtOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textAtOffset(offset, type, startOffset, endOffset);
    return QString();
}

QString QQuickWebEngineViewAccessible::textAfterOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textAfterOffset(offset, type, startOffset, endOffset);
    return QString();
}

QString QQuickWebEngineViewAccessible::textBeforeOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textBeforeOffset(offset, type, startOffset, endOffset);
    return QString();
}

int QQuickWebEngineViewAccessible::offsetAtPoint(const QPoint &point) const
{
    if (auto text = textInterface())
        return text->offsetAtPoint(point);
    return 0;
}

QRect QQuickWebEngineViewAccessible::characterRect(int offset) const
{
    if (auto text = textInterface())
        return text->characterRect(offset);
    return QRect();
}

int QQuickWebEngineViewAccessible::selectionCount() const
{
    if (auto text = textInterface())
        return text->selectionCount();
    return 0;
}

void QQuickWebEngineViewAccessible::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        text->selection(selectionIndex, startOffset, endOffset);
}

int QQuickWebEngineViewAccessible::cursorPosition() const
{
    if (auto text = textInterface())
        return text->cursorPosition();
    return 0;
}

int QQuickWebEngineViewAccessible::characterCount() const
{
    if (auto text = textInterface())
        return text->characterCount();
    return 0;
}

bool QQuickWebEngineViewAccessible::isSelected() const
{
    if (auto cell = cellInterface())
        return cell->isSelected();
    return false;
}

QList<QAccessibleInterface*> QQuickWebEngineViewAccessible::columnHeaderCells() const
{
    if (auto cell = cellInterface())
        return cell->columnHeaderCells();
    return QList<QAccessibleInterface*>();
}

QList<QAccessibleInterface*> QQuickWebEngineViewAccessible::rowHeaderCells() const
{
    if (auto cell = cellInterface())
        return cell->rowHeaderCells();
    return QList<QAccessibleInterface*>();
}

QString QQuickWebEngineViewAccessible::text(int startOffset, int endOffset) const
{
    if (auto text = textInterface())
        return text->text(startOffset, endOffset);
    return QString();
}

int QQuickWebEngineViewAccessible::columnIndex() const
{
    if (auto cell = cellInterface())
        return cell->columnIndex();
    return 0;
}

int QQuickWebEngineViewAccessible::rowIndex() const
{
    if (auto cell = cellInterface())
        return cell->rowIndex();
    return 0;
}

int QQuickWebEngineViewAccessible::columnExtent() const
{
    if (auto cell = cellInterface())
        return cell->columnExtent();
    return 0;
}

int QQuickWebEngineViewAccessible::rowExtent() const
{
    if (auto cell = cellInterface())
        return cell->rowExtent();
    return 0;
}

QAccessibleInterface *QQuickWebEngineViewAccessible::table() const
{
    if (auto cell = cellInterface())
        return cell->table();
    return nullptr;
}

QQuickWebEngineView *QQuickWebEngineViewAccessible::engineView() const
{
    return static_cast<QQuickWebEngineView*>(object());
}

QT_END_NAMESPACE

namespace QtWebEngineCore {

RenderWidgetHostViewQtDelegateQuickAccessible::RenderWidgetHostViewQtDelegateQuickAccessible(QObject *o, QQuickWebEngineView *view)
    : QAccessibleObject(o)
    , m_view(view)
{
}

bool RenderWidgetHostViewQtDelegateQuickAccessible::isValid() const
{
    if (!viewAccessible() || !browserAccessible() || !browserAccessible()->isValid())
        return false;

    return QAccessibleObject::isValid();
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::parent() const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->parent();
    return nullptr;
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::text(QAccessible::Text) const
{
    return QString();
}

QAccessible::Role RenderWidgetHostViewQtDelegateQuickAccessible::role() const
{
    return QAccessible::WebDocument;
}

QAccessible::State RenderWidgetHostViewQtDelegateQuickAccessible::state() const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->state();
    return QAccessible::State();
}

void *RenderWidgetHostViewQtDelegateQuickAccessible::interface_cast(QAccessible::InterfaceType t)
{
    // Only Text interface is exposed at the delegate level
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface *>(this);
    return nullptr;
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::focusChild() const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->focusChild();
    return nullptr;
}

int RenderWidgetHostViewQtDelegateQuickAccessible::childCount() const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->childCount();
    return 0;
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::child(int index) const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->child(index);
    return nullptr;
}

int RenderWidgetHostViewQtDelegateQuickAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (auto browserAcc = browserAccessible())
        return browserAcc->indexOfChild(c);
    return -1;
}

QQuickWebEngineViewAccessible *RenderWidgetHostViewQtDelegateQuickAccessible::viewAccessible() const
{
    if (!m_view)
        return nullptr;
    return static_cast<QQuickWebEngineViewAccessible *>(QAccessible::queryAccessibleInterface(m_view));
}

QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::browserAccessible() const
{
    // Bridges to the browser's accessibility tree via the parent view accessible
    if (auto viewAcc = viewAccessible())
        return viewAcc->browserAccessible();
    return nullptr;
}

QAccessibleTextInterface *RenderWidgetHostViewQtDelegateQuickAccessible::textInterface() const
{
    if (auto browserAcc = browserAccessible())
        return static_cast<QAccessibleTextInterface *>(browserAcc->interface_cast(QAccessible::TextInterface));
    return nullptr;
}

void RenderWidgetHostViewQtDelegateQuickAccessible::scrollToSubstring(int startIndex, int endIndex)
{
    if (auto text = textInterface())
        text->scrollToSubstring(startIndex, endIndex);
}

void RenderWidgetHostViewQtDelegateQuickAccessible::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (auto text = textInterface())
        text->setSelection(selectionIndex, startOffset, endOffset);
}

void RenderWidgetHostViewQtDelegateQuickAccessible::addSelection(int startOffset, int endOffset)
{
    if (auto text = textInterface())
        text->addSelection(startOffset, endOffset);
}

void RenderWidgetHostViewQtDelegateQuickAccessible::removeSelection(int selectionIndex)
{
    if (auto text = textInterface())
        text->removeSelection(selectionIndex);
}

void RenderWidgetHostViewQtDelegateQuickAccessible::setCursorPosition(int position)
{
    if (auto text = textInterface())
        text->setCursorPosition(position);
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::attributes(int offset, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->attributes(offset, startOffset, endOffset);
    return QString();
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::textAtOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textAtOffset(offset, type, startOffset, endOffset);
    return QString();
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::textAfterOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textAfterOffset(offset, type, startOffset, endOffset);
    return QString();
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::textBeforeOffset(int offset, QAccessible::TextBoundaryType type, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        return text->textBeforeOffset(offset, type, startOffset, endOffset);
    return QString();
}

int RenderWidgetHostViewQtDelegateQuickAccessible::offsetAtPoint(const QPoint &point) const
{
    if (auto text = textInterface())
        return text->offsetAtPoint(point);
    return 0;
}

QRect RenderWidgetHostViewQtDelegateQuickAccessible::characterRect(int offset) const
{
    if (auto text = textInterface())
        return text->characterRect(offset);
    return QRect();
}

int RenderWidgetHostViewQtDelegateQuickAccessible::selectionCount() const
{
    if (auto text = textInterface())
        return text->selectionCount();
    return 0;
}

void RenderWidgetHostViewQtDelegateQuickAccessible::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    if (auto text = textInterface())
        text->selection(selectionIndex, startOffset, endOffset);
}

int RenderWidgetHostViewQtDelegateQuickAccessible::cursorPosition() const
{
    if (auto text = textInterface())
        return text->cursorPosition();
    return 0;
}

int RenderWidgetHostViewQtDelegateQuickAccessible::characterCount() const
{
    if (auto text = textInterface())
        return text->characterCount();
    return 0;
}

QString RenderWidgetHostViewQtDelegateQuickAccessible::text(int startOffset, int endOffset) const
{
    if (auto text = textInterface())
        return text->text(startOffset, endOffset);
    return QString();
}

} // namespace QtWebEngineCore
