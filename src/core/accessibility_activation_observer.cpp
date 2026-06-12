// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "accessibility_activation_observer.h"
#include <QDebug>

#include "content/browser/accessibility/browser_accessibility_state_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/scoped_accessibility_mode.h"
#include "web_contents_adapter.h"

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

namespace {

bool isAccessibilityEnabled() {
    // On Linux accessibility can be disabled due to performance issues by setting the
    // QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY environment variable to 0. For details,
    // see QTBUG-59922.
#ifdef Q_OS_LINUX
    static bool accessibility_enabled =
            qEnvironmentVariable("QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY", u"1"_s) == "1"_L1;
#else
    const bool accessibility_enabled = true;
#endif
    return accessibility_enabled;
}

} // namespace

AccessibilityActivationObserver::AccessibilityActivationObserver()
{
    qDebug() << "AccessibilityActivationObserver::ctor: isAccessibilityEnabled=" << isAccessibilityEnabled();
    if (isAccessibilityEnabled()) {
        qDebug() << "AccessibilityActivationObserver::ctor: Installing activation observer";
        QAccessible::installActivationObserver(this);
        if (QAccessible::isActive()) {
            qDebug() << "AccessibilityActivationObserver::ctor: QAccessible is active";
            content::BrowserAccessibilityStateImpl::GetInstance()->SetActivationFromPlatformEnabled(true);
        }
    }
}

AccessibilityActivationObserver::~AccessibilityActivationObserver()
{
    qDebug() << "AccessibilityActivationObserver::~dtor: Removing activation observer";
    QAccessible::removeActivationObserver(this);
}

void AccessibilityActivationObserver::accessibilityActiveChanged(bool active)
{
    qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: active=" << active;
    if (active) {
        qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: Creating scoped_accessibility_mode_";
        scoped_accessibility_mode_ =
            content::BrowserAccessibilityStateImpl::GetInstance()->CreateScopedModeForProcess(ui::kAXModeComplete);
        qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: Created scoped_accessibility_mode_";

        // When accessibility is enabled after startup, reload all WebContents pages.
        // This ensures accessibility trees are created with proper accessibility support
        // from the renderer side, and browser-side managers are properly initialized.
        auto webContentsList = content::WebContentsImpl::GetAllWebContents();
        qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: Found" << webContentsList.size() << "web contents";
        for (auto *webContents : webContentsList) {
            if (!webContents->IsBeingDestroyed() && !webContents->IsNeverComposited()) {
                qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: Reloading webContents" << webContents;
                // Reload the page to trigger proper accessibility initialization
                webContents->GetController().Reload(content::ReloadType::NORMAL, true);
            }
        }
    } else {
        qDebug() << "AccessibilityActivationObserver::accessibilityActiveChanged: Resetting scoped_accessibility_mode_";
        scoped_accessibility_mode_.reset();
    }
}

} // namespace QtWebEngineCore
