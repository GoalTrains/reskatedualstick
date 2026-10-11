DUAL STICK CONTROLS v5.73 TEST - changes since v5.71

- Prevent the companion welcome/settings menu from installing competing DXGI
  graphics hooks while ReSkate is loaded. This targets the startup conflict
  that can prevent Insert, the mod manager and map browser from appearing.
- Use the separate companion menu window with ReSkate, including old INIs
  containing MenuInGame=1. End/custom hotkey and startup opt-out still work.
- Yield companion controller input while the official ReSkate Insert menu is
  visible, using its existing status export. Poll on the menu worker to avoid
  locking the render thread from a controller callback.
- Close the companion menu when ReSkate takes ownership. Clear generated
  gestures and guard held foot inputs until release when controls return.
- Support the extended input-ownership export in the experimental ReSkate
  integration, when present. Stock ReSkate does not require that PR build.
- Gameplay mappings and Session/XL preset defaults are unchanged from v5.71.

The proposed native Dual Stick settings page remains a separate PR project;
it is not included in these standalone zips. No ReSkate.dll is distributed.
Build and nine native control regressions passed. In-game Insert/menu access
still needs confirmation; Linux/Proton gameplay has not been independently tested.
