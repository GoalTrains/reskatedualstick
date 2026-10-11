# Submit the ReSkate menu integration

A pull request asks ReSkate's maintainer to review and merge these changes.
The standalone Dual Stick repository is separate from ReSkate, so it cannot be
used directly as the source of an upstream pull request.

1. Fork https://github.com/Dingo-Shenanigans/ReSkate into your GitHub account.
   A fork is your own copy of ReSkate, linked to the original project.
2. Create a branch in that fork and apply the changes in `dual-stick-menu.patch`.
   The patch records its original base in `README.txt`; reconcile any upstream
   changes before pushing. Keep the standalone mod in its existing repository.
3. Build and test the branch, then push it to your fork.
4. Open a draft pull request targeting `Dingo-Shenanigans/ReSkate`, base `main`,
   from your fork's branch. Use `PR_DESCRIPTION.md` for its description and
   accurately record in-game results. The maintainer reviews it before merging.

The patch adds companion settings to Insert (directly below Trainer) and the
pause menu, controller navigation, and an input-ownership export. It does not
bundle the standalone mod. No upstream PR has been submitted yet.
