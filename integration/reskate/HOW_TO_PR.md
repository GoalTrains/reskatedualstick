# Submit to ReSkate

1. Fork https://github.com/Dingo-Shenanigans/ReSkate into your GitHub account. The existing reskatedualstick repository is the standalone companion, not a ReSkate fork.
2. Clone that fork and create a branch from upstream commit 6df4f9a7bd4045aa74c853f088ff445011933175 (or update/rebase for newer upstream changes).
3. Download dual-stick-menu.patch from this preparation branch. In the ReSkate checkout, run:

```powershell
git switch -c dual-stick-menu 6df4f9a7bd4045aa74c853f088ff445011933175
git apply --check C:/path/to/dual-stick-menu.patch
git apply C:/path/to/dual-stick-menu.patch
git add CMakeLists.txt Extension/UI/Overlay contrib/dual-stick-controls.md
git commit -m "Add optional Dual Stick settings to ReSkate menus"
git push -u origin dual-stick-menu
```

4. Open a pull request from your fork's dual-stick-menu branch into Dingo-Shenanigans/ReSkate main. Use PR_DESCRIPTION.md for the title/body. A draft is appropriate until current-build gameplay checks are complete.

The optional CMake flag DINGOSDK_BUILD_DUAL_STICK_TESTS=ON builds the two integration tests. Use the compiler/generator supported by the current upstream build instructions. The local preparation used Visual Studio 2026 x64, Release. Do not distribute the custom PR runtime as part of the standalone release.
