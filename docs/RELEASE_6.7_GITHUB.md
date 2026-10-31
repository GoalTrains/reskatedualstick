## New ways to revert

- **Slide + Rotate:** start a powerslide, then turn both sticks a little further. Matching and mirrored thumb circles both work.
- **Free Foot Arc:** curve a stick through the unused direction for your stance while keeping the other stick centred. It follows your layout and stance; the controls picture shows the arc. With Session-style controls, use the right-stick top arc normally and the left-stick bottom arc in switch.
- Choose either under **End > Custom > Reverts**. The trigger options are still available, and your existing choice is preserved.

## Control fixes

- Turn with LT/RT while holding D-pad UP to set a session marker.
- Switch camera sides while holding either push button.
- Correct switch pop direction with Left/Right Foot controls, including goofy riders.
- Correct reversed flip directions in affected Session-style fakie/switch situations.
- Fix recorded cases where nollie or XL flip input changed direction at takeoff. Held flips now keep their intended input as the wheels leave the ground.
- Reduce unnecessary settings-reading work in the controller callback. Overall controller-to-screen latency has not been measured.

## Updating

Close Skate and replace **version.dll** beside Skate.exe. **Keep your existing session_controls.ini, saved presets and official ReSkate.dll.** Fresh settings are included for new installs.

Windows: download **Dual_Stick_Controls_v6.7.zip**. Linux / Steam Deck: download **Dual_Stick_Controls_v6.7_Linux.zip** and follow its Proton instructions. The optional Windows updater is unchanged; existing updater users do not need to reinstall it.

## Tested

**132 automated control tests pass**, including recorded-input replays and checks across stances and layouts. Slide reverts are confirmed working in local gameplay. Some flip/shuv-it and manual-catch issues remain under investigation, and the latest flip fixes still need wider gameplay confirmation. Grabs are unchanged. Linux / Steam Deck gameplay testing is still outstanding.
