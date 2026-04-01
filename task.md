# MasterGo Task

Requirement Description: Convert the MasterGo page into a new LVGL v9 page in this repository with fixed-size 480x480 absolute-position layout, localized assets/fonts, paired page lifecycle, successful build, and runnable desktop simulator output. The visual target is pixel-level 1:1 restoration against the provided design node.

## Page List
Home Page (layerId: sa281:4872)

## Navigation Information
None

## Implementation Summary
- Design source: MasterGo
- Target node: `fileId=131850344368735`, `layerId=sa281:4872`
- Page size: `480 x 480`
- Current scope: static LVGL page with design-faithful layout and localized fonts
- Layout strategy: page-level absolute positioning using DSL coordinates; no responsive/flex approximation for the main structure
- Naming scheme: `smart_home_page.*`, `smart_home_assets.*`, `main/assets/smart_home/*`
- Page structure:
  - full black background
  - top status/weather bar at `16,15,448,32`
  - tabs `Scenes` and `Devices` at `y=70`
  - four scene cards in a 2x2 grid
  - bottom gradient overlay at `0,428,480,52`
  - page indicator at `226,462,28,6`
- Strict alignment targets:
  - root screen size `480x480`
  - top-left card `16,130,219,145`
  - top-right card `245,130,219,135`
  - bottom-left card `16,285,219,145`
  - bottom-right card `245,285,219,145`
  - card radius `16`
- Local asset plan:
  - keep all page-private resources under `main/assets/smart_home/`
  - load fonts through `lv_tiny_ttf_create_file()`
  - implement icons with LVGL native draw callbacks where no exported local asset is available

## Key Design Values

| Module | Element | x | y | w | h | radius | font | size | weight | color | opacity | Notes |
|------|------|---|---|---|---|------|------|------|--------|------|---------|------|
| Page | Root background | 0 | 0 | 480 | 480 | 0 | - | - | - | `#000000` | 1.0 | fixed screen |
| Status | Weather block | 16 | 15 | 448 | 32 | 0 | mixed | mixed | mixed | `#FFFFFF` | 1.0 | icon + temperature + time |
| Tabs | Scenes | 16 | 70 | 111 | 48 | 0 | Source Han Sans CN | 32 | Bold | `#FFFFFF` | 0.9 | active tab |
| Tabs | Devices | 144 | 70 | 115 | 48 | 0 | Source Han Sans CN | 32 | Regular | `#FFFFFF` | 0.5 | inactive tab |
| Card | Home | 16 | 130 | 219 | 145 | 16 | Source Han Sans CN | 18 | Medium | `#FFFFFF` | 0.9 | glass card, white 15% fill |
| Card | Morning | 245 | 130 | 219 | 135 | 16 | Source Han Sans CN | 18 | Medium | `#FFFFFF` | 0.9 | blue gradient card |
| Card | Morning | 16 | 285 | 219 | 145 | 16 | Source Han Sans CN | 18 | Medium | `#FFFFFF` | 0.9 | blue gradient card |
| Card | All Lights On | 245 | 285 | 219 | 145 | 16 | Source Han Sans CN | 18 | Medium | `#FFFFFF` | 0.9 | glass card |
| Overlay | Bottom fade | 0 | 428 | 480 | 52 | 0 | - | - | - | transparent to `#000000` | 1.0 | vertical gradient |
| Pager | Indicator | 226 | 462 | 28 | 6 | 76 | - | - | - | white + `#41CDE2` | mixed | 6px dot + 18px active pill |

## Planned / Added Files
- `main/inc/smart_home_page.h`
- `main/inc/smart_home_assets.h`
- `main/src/smart_home_page.c`
- `main/src/smart_home_assets.c`
- `main/tests/smart_home_page_test.cpp`
- `main/assets/smart_home/fonts/*`
- `task.md`
