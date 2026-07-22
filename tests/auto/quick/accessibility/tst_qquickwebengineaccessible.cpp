// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qtest.h>
#include <testwindow.h>

#include <QEventLoop>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTimer>
#include <QtWebEngineCore/QWebEngineLoadingInfo>
#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <qaccessible.h>

class tst_QQuickWebEngineAccessible : public QObject
{
    Q_OBJECT

public:
    tst_QQuickWebEngineAccessible();

private Q_SLOTS:
    void init();
    void cleanup();

    void viewAccessibleProxy();
    void textInterfaceDelegation();
    void tableInterfaceDelegation();

private:
    inline QQuickWebEngineView *newWebEngineView();
    inline QQuickWebEngineView *webEngineView() const;

    QScopedPointer<TestWindow> m_window;
    QScopedPointer<QQmlComponent> m_component;
};

tst_QQuickWebEngineAccessible::tst_QQuickWebEngineAccessible()
{
    static QQmlEngine *engine = new QQmlEngine(this);
    m_component.reset(new QQmlComponent(engine, this));
    m_component->setData(QByteArrayLiteral("import QtQuick\n"
                                           "import QtWebEngine\n"
                                           "WebEngineView {}")
                         , QUrl());
}

QQuickWebEngineView *tst_QQuickWebEngineAccessible::newWebEngineView()
{
    QObject *viewInstance = m_component->create();
    QQuickWebEngineView *webEngineView = qobject_cast<QQuickWebEngineView *>(viewInstance);
    return webEngineView;
}

void tst_QQuickWebEngineAccessible::init()
{
    m_window.reset(new TestWindow(newWebEngineView()));
}

void tst_QQuickWebEngineAccessible::cleanup()
{
    m_window.reset();
}

inline QQuickWebEngineView *tst_QQuickWebEngineAccessible::webEngineView() const
{
    return static_cast<QQuickWebEngineView *>(m_window->webEngineView.data());
}

void tst_QQuickWebEngineAccessible::viewAccessibleProxy()
{
    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->setSize(QSizeF(640, 480));

    bool loaded = false;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(view, &QQuickWebEngineView::loadingChanged, &loop,
        [&loaded, &loop](const QWebEngineLoadingInfo &info) {
            if (info.status() == QWebEngineLoadingInfo::LoadSucceededStatus) {
                loaded = true;
                loop.quit();
            }
        });
    timer.start(10000);
    view->loadHtml("<html><body><p id='p1'>Hello</p></body></html>");
    loop.exec();
    QVERIFY(loaded);

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(view);
    QVERIFY(iface);
    QCOMPARE(iface->role(), QAccessible::WebDocument);
    QVERIFY(iface->isValid());
}

void tst_QQuickWebEngineAccessible::textInterfaceDelegation()
{
    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->setSize(QSizeF(640, 480));

    bool loaded = false;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(view, &QQuickWebEngineView::loadingChanged, &loop,
        [&loaded, &loop](const QWebEngineLoadingInfo &info) {
            if (info.status() == QWebEngineLoadingInfo::LoadSucceededStatus) {
                loaded = true;
                loop.quit();
            }
        });
    timer.start(10000);
    view->loadHtml("<html><body><input type='text' id='input1' value='Hello world'></input></body></html>");
    loop.exec();
    QVERIFY(loaded);

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(view);
    QTRY_COMPARE(iface->child(0)->childCount(), 1);
    QAccessibleInterface *document = iface->child(0);
    QAccessibleInterface *grouping = document->child(0);
    QCOMPARE(grouping->childCount(), 1);
    QAccessibleInterface *input = grouping->child(0);
    QCOMPARE(input->role(), QAccessible::EditableText);

    QAccessibleTextInterface *text = input->textInterface();
    QVERIFY(text);
    QCOMPARE(text->characterCount(), 11);

    int start = -1, end = -1;
    QString word = text->textAtOffset(0, QAccessible::WordBoundary, &start, &end);
    QVERIFY(word.startsWith(QStringLiteral("Hello")));
    QCOMPARE(start, 0);
    QVERIFY(end >= 5);

    QString before = text->textBeforeOffset(6, QAccessible::WordBoundary, &start, &end);
    QVERIFY(before.startsWith(QStringLiteral("Hello")));

    QRect rect = text->characterRect(0);
    QVERIFY(rect.width() > 0);
    QVERIFY(rect.height() > 0);
}

void tst_QQuickWebEngineAccessible::tableInterfaceDelegation()
{
    QQuickWebEngineView *view = webEngineView();
    m_window->show();
    view->setSize(QSizeF(640, 480));

    bool loaded = false;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(view, &QQuickWebEngineView::loadingChanged, &loop,
        [&loaded, &loop](const QWebEngineLoadingInfo &info) {
            if (info.status() == QWebEngineLoadingInfo::LoadSucceededStatus) {
                loaded = true;
                loop.quit();
            }
        });
    timer.start(10000);
    view->loadHtml("<html><body><table>"
        "  <thead>"
        "    <tr><th scope='col'>Col A</th><th scope='col'>Col B</th></tr>"
        "  </thead>"
        "  <tbody>"
        "    <tr><th scope='row'>Row A</th><td>A1</td></tr>"
        "  </tbody>"
        "</table></body></html>");
    loop.exec();
    QVERIFY(loaded);

    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(view);
    QTRY_COMPARE(iface->child(0)->childCount(), 1);
    QAccessibleInterface *table = iface->child(0)->child(0);

    QAccessibleTableInterface *tableIface = table->tableInterface();
    QVERIFY(tableIface);

    QAccessibleInterface *cell = tableIface->cellAt(1, 1);
    QVERIFY(cell);
    QCOMPARE(cell->child(0)->text(QAccessible::Name), QStringLiteral("A1"));

    QAccessibleTableCellInterface *cellIface = cell->tableCellInterface();
    QVERIFY(cellIface);
    QCOMPARE(cellIface->rowIndex(), 1);
    QCOMPARE(cellIface->columnIndex(), 1);

    QList<QAccessibleInterface *> colHeaders = cellIface->columnHeaderCells();
    QList<QAccessibleInterface *> rowHeaders = cellIface->rowHeaderCells();
    QVERIFY(colHeaders.size() >= 1);
    QVERIFY(rowHeaders.size() >= 1);

    QCOMPARE(tableIface->columnDescription(0), QStringLiteral("Col A"));
    QCOMPARE(tableIface->columnDescription(1), QStringLiteral("Col B"));
    QCOMPARE(tableIface->rowDescription(1), QStringLiteral("Row A"));
}

static QByteArrayList params = QByteArrayList()
    << "--force-renderer-accessibility"
    << "--enable-features=AccessibilityExposeARIAAnnotations"
    ;

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();

    QList<const char *> w_argv(argc);
    QLatin1String arg("--webEngineArgs");
    for (int i = 0; i < argc; ++i)
        w_argv[i] = argv[i];
    w_argv.append(arg.data());
    for (int i = 0; i < params.size(); ++i)
        w_argv.append(params[i].data());
    int w_argc = w_argv.size();

    QGuiApplication app(w_argc, const_cast<char **>(w_argv.data()));
    app.setAttribute(Qt::AA_Use96Dpi, true);
    QTEST_SET_MAIN_SOURCE_PATH
    tst_QQuickWebEngineAccessible tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_qquickwebengineaccessible.moc"
