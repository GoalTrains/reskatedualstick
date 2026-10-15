# ReSkate menu PR preparation

This branch holds the optional ReSkate menu contribution separately from the standalone Dual Stick release. Submitted upstream as [draft PR #254](https://github.com/Dingo-Shenanigans/ReSkate/pull/254), from [GoalTrains/ReSkate:dual-stick-menu](https://github.com/GoalTrains/ReSkate/tree/dual-stick-menu). It has not been installed into the game. The full build and automated integration checks passed; current-build in-game validation remains before marking it ready.

The patch targets Dingo-Shenanigans/ReSkate main commit 6df4f9a7bd4045aa74c853f088ff445011933175. It adds Dual Stick below Trainer in Insert, plus a Dual Stick tab in native Mod Options. It edits the existing companion INI and includes current v5.133 layouts/presets, shared custom slots and controller navigation. It does not bundle the control translator.

Files:
- dual-stick-menu.patch: source changes against upstream.
- PR_DESCRIPTION.md: proposed pull request text.
- HOW_TO_PR.md: apply the patch in a ReSkate fork and open the PR.
- VALIDATION.md: build/check results and remaining game checks.

The public standalone main branch and release zips keep the official ReSkate runtime. Do not merge this preparation branch into standalone main to install menus.
