// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKWEBENGINE_ACCESSIBLE_H
#define QQUICKWEBENGINE_ACCESSIBLE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qpointer.h>
#include <QtGui/qaccessibleobject.h>
#include <QtGui/qaccessibletextinterface.h>
#include <QtGui/qaccessibletablecellinterface.h>

QT_BEGIN_NAMESPACE
class QQuickWebEngineView;

class QQuickWebEngineViewAccessible : public QAccessibleObject, public QAccessibleTextInterface, public QAccessibleTableCellInterface
{
public:
    QQuickWebEngineViewAccessible(QQuickWebEngineView *o);
    bool isValid() const override;
    QAccessibleInterface *parent() const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *) const override;
    QString text(QAccessible::Text) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    void *interface_cast(QAccessible::InterfaceType t) override;

    void scrollToSubstring(int startIndex, int endIndex) override;
    void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    void addSelection(int startOffset, int endOffset) override;
    void removeSelection(int selectionIndex) override;
    void setCursorPosition(int position) override;
    QString attributes(int offset, int *startOffset, int *endOffset) const override;
    QString textAtOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    QString textAfterOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    QString textBeforeOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    int offsetAtPoint(const QPoint &point) const override;
    QRect characterRect(int offset) const override;
    int selectionCount() const override;
    void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
    int cursorPosition() const override;
    int characterCount() const override;
    QString text(int startOffset, int endOffset) const override;

    int columnIndex() const override;
    int rowIndex() const override;
    int columnExtent() const override;
    int rowExtent() const override;
    QAccessibleInterface *table() const override;

private:
    QAccessibleInterface *browserAccessible() const;
    QQuickWebEngineView *engineView() const;
    mutable QAccessibleInterface *m_browserAccessibleCache = nullptr;
};

QT_END_NAMESPACE

namespace QtWebEngineCore {
class RenderWidgetHostViewQtDelegateQuickAccessible : public QAccessibleObject, public QAccessibleTextInterface
{
public:
    RenderWidgetHostViewQtDelegateQuickAccessible(QObject *o, QQuickWebEngineView *view);

    bool isValid() const override;
    QAccessibleInterface *parent() const override;
    QString text(QAccessible::Text t) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *) const override;

    void *interface_cast(QAccessible::InterfaceType t) override;

    void scrollToSubstring(int startIndex, int endIndex) override;
    void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    void addSelection(int startOffset, int endOffset) override;
    void removeSelection(int selectionIndex) override;
    void setCursorPosition(int position) override;
    QString attributes(int offset, int *startOffset, int *endOffset) const override;
    QString textAtOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    QString textAfterOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    QString textBeforeOffset(int offset, QAccessible::BoundaryType type, int *startOffset, int *endOffset) const override;
    int offsetAtPoint(const QPoint &point) const override;
    QRect characterRect(int offset) const override;
    int selectionCount() const override;
    void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
    int cursorPosition() const override;
    int characterCount() const override;
    QString text(int startOffset, int endOffset) const override;

private:
    QQuickWebEngineViewAccessible *viewAccessible() const;
    QPointer<QQuickWebEngineView> m_view;
};
} // namespace QtWebEngineCore

#endif // QQUICKWEBENGINE_ACCESSIBLE_H
