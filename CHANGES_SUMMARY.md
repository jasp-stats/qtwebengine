## Summary of Changes

Fixed compilation errors in qtwebengine accessibility code for Qt 6.11.1:

### Interface Type Fixes
- Changed `QAccessible::InterfaceType::text` → `QAccessible::TextInterface` (2 occurrences)
- All text interface uses now correctly reference `QAccessible::TextInterface`
- All table cell interface uses correctly reference `QAccessible::TableCellInterface`

### Missing Method Declarations (Header)
Added to `QQuickWebEngineViewAccessible`:
- `int columnIndex() const override;`
- `int rowIndex() const override;`
- `int columnExtent() const override;`
- `int rowExtent() const override;`
- `QAccessibleInterface *table() const override;`

Added to `RenderWidgetHostViewQtDelegateQuickAccessible`:
- `QQuickWebEngineViewAccessible *viewAccessible() const;`

### Method Call Fixes  
Changed all `viewAccessible()->browserAccessible()` calls to `browserAccessible()` (21 occurrences in cpp file)

### New Method Implementation
Implemented `browserAccessible()` for `RenderWidgetHostViewQtDelegateQuickAccessible`:
```cpp
QAccessibleInterface *RenderWidgetHostViewQtDelegateQuickAccessible::browserAccessible() const
{
    return viewAccessible()->browserAccessible();
}
```

## Files Modified
- `src/webenginequick/qquickwebengine_accessible_p.h`: +7 lines
- `src/webenginequick/qquickwebengine_accessible.cpp`: +28 lines, -27 lines

Total: 37 insertions(+), 25 deletions(-)
