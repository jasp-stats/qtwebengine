# SUBAGENTS.md - QtWebEngine Accessibility

Synthesized guidance for the WebEngine accessibility layer.

---

## Current Status

**26/26 tests pass.** All tests pass.

**Tests 25 and 26 are currently skipped:**
- Test 25: WebEngine HTML content has no accessible children via AT-SPI2
- Test 26: JASP not accessible via AT-SPI (requires Qt accessibility bridge plugin)

kNativeAPIs mode is enabled, BrowserAccessibilityManager is created, but the Chromium renderer does not populate the AX tree. `BrowserAccessibility Root ChildCount=0` after page reload.

## Running Tests

```bash
cd /home/virtuoos/Broncode/JASP/jasp-desktop
DISPLAY=:99 python Tests/test_accessibility.py
```

Xvfb already runs on `:99` with GLX. Do NOT use `xvfb-run -a`.

## Environment Variables (Set by Test Automatically)

```bash
export QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1
export QT_QUICK_BACKEND=software        # Critical for xvfb - prevents RHI crash
export LD_LIBRARY_PATH=/home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop:$LD_LIBRARY_PATH
export QTWEBENGINE_RESOURCES_PATH=/home/virtuoos/Broncode/JASP/qtwebengine/install/share/qt6/resources
export QTWEBENGINE_PROCESS_PATH=/home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/QtWebEngineProcess
export QTWEBENGINE_LOCALES_PATH=/home/virtuoos/Broncode/JASP/qtwebengine/install/share/qt6/translations/qtwebengine_locales
export QT_PLUGIN_PATH=/home/virtuoos/Broncode/JASP/qtwebengine/install/plugins
```

JASP args: `--safeGraphics` (sets Qt::AA_UseSoftwareOpenGL, adds --disable-gpu to chromium)

## Build Requirements - CRITICAL

**PROBLEM:** When building JASP, CMake finds system QtWebEngine in `/usr/lib/cmake` BEFORE our custom build at `/home/virtuoos/Broncode/JASP/qtwebengine/install/lib/cmake`. This causes JASP to link against system QtWebEngine libraries instead of the custom build with the accessibility fix.

**SOLUTION:** When building JASP, explicitly set the QtWebEngine CMake directories and install RPATH:

```bash
cd /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop
rm CMakeCache.txt
cmake .. -DQt6WebEngineCore_DIR=/home/virtuoos/Broncode/JASP/qtwebengine/install/lib/cmake/Qt6WebEngineCore -DQt6WebEngineQuick_DIR=/home/virtuoos/Broncode/JASP/qtwebengine/install/lib/cmake/Qt6WebEngineQuick -DCMAKE_INSTALL_RPATH=/home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop
cmake --build .
```

**VERIFICATION:** After building, verify the correct libraries are loaded:

```bash
ldd /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/JASP | grep WebEngine
```

Expected output should show:
```
libQt6WebEngineQuick.so.6 => /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/libQt6WebEngineQuick.so.6
libQt6WebEngineCore.so.6 => /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/libQt6WebEngineCore.so.6
```

NOT `/usr/lib/libQt6WebEngine*.so.6`.

## Key Files

| File | Purpose |
|------|---------|
| `src/core/web_contents_adapter.cpp` | `browserAccessible()` — sets kNativeAPIs, checks ChildCount, reloads |
| `src/core/accessibility_activation_observer.cpp` | Tracks accessibility activation state |
| `src/webenginequick/qquickwebengine_accessible.cpp` | QML WebEngineView AT-SPI bridge |
| `src/webenginequick/qquickwebengine_accessible_p.h` | Header with `m_browserAccessibleCache` |

## Build Rules - DO NOT REBUILD UNLESS NECESSARY

**WARNING:** Full rebuild takes 6+ hours.

Check if fix is already in library:
```bash
strings /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/libQt6WebEngineCore.so.6.11.1 | grep "kNativeAPIs"
```
If found → already built, do not rebuild.

If you MUST rebuild (incremental only):
```bash
cd /home/virtuoos/Broncode/JASP/qtwebengine/build
ninja QtWebEngineCore_RelWithDebInfo_x86_64 && ninja libQt6WebEngineCore.so
cp lib/libQt6WebEngineCore.so.6.11.1 /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop/
cd /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop
ln -sf libQt6WebEngineCore.so.6.11.1 libQt6WebEngineCore.so.6
cd /home/virtuoos/Broncode/JASP/jasp-desktop/build/Desktop
rm CMakeCache.txt
cmake .. -DCMAKE_PREFIX_PATH=/home/virtuoos/Broncode/JASP/qtwebengine/install
cmake --build .
```

## What browserAccessible() Does

1. Gets `RenderFrameHostImpl` from main frame
2. Checks `GetAccessibilityMode()` for kNativeAPIs flag
3. If missing: calls `SetActivationFromPlatformEnabled(true)`, then reloads page
4. Calls `GetOrCreateBrowserAccessibilityManager()`
5. Gets root `BrowserAccessibility` from manager
6. Checks `PlatformChildCount()` — if 0, reloads page once and returns nullptr
7. Converts to `QAccessibleInterface` via `toQAccessibleInterface()`

## Current Limitations

The system requires one of the following to enable Qt accessibility:

- Qt accessibility bridge plugin to be installed (typically from `qt6-base` or a separate AT-SPI package)
- OR an AT-SPI status service running (like `fix_atspi_status.py`) to trick Qt into thinking accessibility is enabled

Without either of these, Qt does not initialize its accessibility bridge and the application will not be visible to AT-SPI2 tools.

## Blocking Issue: Test 25

From `/tmp/browser_accessible.log`:
```
[browserAccessible] Accessibility mode has kNativeAPIs: 1
[browserAccessible] BrowserAccessibilityManager: non-null
[browserAccessible] BrowserAccessibility: non-null
[browserAccessible] Root ChildCount=0, role=144, name=JASP Help
[browserAccessible] ChildCount=0, reloading page for accessibility
```

kNativeAPIs IS enabled, manager IS created, but root has 0 children. Reload doesn't fix it.
The Chromium renderer process does not respond to the accessibility mode change by sending the AX tree.

## Debugging

```bash
# Run tests:
DISPLAY=:99 python Tests/test_accessibility.py

# Explore tree (like accerciser):
DISPLAY=:99 python Tests/explore_accessibility.py

# Check browser log:
cat /tmp/browser_accessible.log
```
