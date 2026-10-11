DUAL STICK CONTROLS v5.71 TEST - changes since v5.59

- Session-style preset now defaults one-stick flips to BLOCKED; XL-style
  defaults to ALLOWED. The setting remains editable and is saved by custom
  presets. Preset ACTIVE status also reflects this setting.
- Optional Stock ramp controls: OFF, AIR ONLY, or EARLY ON RAMP. Early mode
  switches during a qualifying grounded ramp climb for stock pop/prewind,
  then keeps stock controls until landing. Cancelled approaches return to
  foot controls. The selected mode can be saved in custom presets.
- Stock handling uses the physical left stick for steering/spins/body flips,
  right stick for board input, and triggers for grabs. R3 pumping remains.
- Added experimental detection for sustained steep approaches and rapid
  grounded tilt changes on gentler rounded surfaces, with brief retention
  across the crest. This is an orientation cue, not confirmed surface slope.
- Fixed held sideways input being blocked during stock air rotation.
- Fixed ramp qualification clearing during the pop state before takeoff,
  causing intermittent trigger grabs. Held vertical gestures stay guarded;
  landing does not turn held stock input into a fresh foot pop.
- Removed powerslide activation delay, including legacy delay settings.
  More forgiving inward/outward activation and holding; adjustable slide
  angle while both sticks are held for coping approaches.
- Fixed a powerslide latch surviving grind/stall or air entry and blocking
  normal foot handling and flip-outs. Softer stall/grind pop setup; a foot
  already held at the pop end can prepare a trick by flicking the other foot.
- Fixed goofy True Foot losing switched foot jobs after a 180 when stance
  probes reset on the ground. Regular and other layouts retain their routes.
- Simplified menu: Session-style and XL-style presets at the top, individual
  options in Advanced, and five custom preset slots with overwrite confirmation.
  Both presets translate Skate inputs across two sticks.
- Front/Back Foot now applies switch handling automatically and hides the
  redundant switch row. Renamed Front/back flips to Body flips.
- Startup welcome/menu with a saved opt-out toggle and instructions for
  reopening. Held buttons cannot accidentally apply a preset on opening.
- Custom menu hotkey, saved immediately and preserved when loading presets.
- Larger, clearer controls diagrams: six cards per page across three pages,
  larger text/sticks, brighter arrows, wrapped notes and release-pop diagrams.
- Expanded input logs with stance probes, ramp qualification and early-mode
  activity. Optional labelled ramp snapshots; change recording label in the
  menu to restart the bounded allowance without relaunching.

Ramp detection and early switching remain experimental. Fresh installs keep
stock ramp controls OFF; AIR ONLY remains available alongside EARLY ON RAMP.
Native control/menu checks and recorded-input replay passed. Linux/Proton
gameplay has not been independently tested with this build.
