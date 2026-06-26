# Git Workflow & Branch Strategy

## The One Rule That Prevents All Merge Conflicts

> **Never commit directly to `main`. Always work on your own branch and open a PR.**

Every dev works in isolation on their own branch. Conflicts happen when two
people edit the same file on the same branch at the same time. If you're on
your own branch and only touching your own files, conflicts become nearly impossible.

---

## Branch Naming Convention

Create your branch before writing a single line of code:

```
dev2/auth-system
dev3/user-features
dev4/admin-panel
```

To create and switch to your branch:
```bash
git checkout main
git pull origin main          # always start from the latest main
git checkout -b dev2/auth-system
```

---

## The Full PR Lifecycle

```
main (protected)
  │
  ├─── dev2/auth-system ──────── PR ──────────────────────────────┐
  │                                                                │
  ├─── dev3/user-features ──────  PR ─────────────────────────────┤  → supervisor reviews
  │                                                                │     CI must pass
  └─── dev4/admin-panel ────────  PR ─────────────────────────────┘     merge into main
```

### Step 1 — Start your branch

```bash
git checkout main
git pull origin main          # get the latest code from main
git checkout -b devN/your-feature
```

### Step 2 — Work on your files only

Only modify the files assigned to you in `docs/dev-assignments.md`.
If you need to touch someone else's file, talk to the supervisor first.

Commit often with clear messages:
```bash
git add include/auth/session.h src/auth/session.cpp
git commit -m "Dev 2 [Auth]: implement Session::start/end/current"
```

### Step 3 — Before opening a PR, sync with main

```bash
git checkout main
git pull origin main           # fetch latest changes
git checkout devN/your-feature
git merge main                 # bring main's changes into your branch
```

If there are conflicts at this step, resolve them NOW — before the PR.
See [Resolving Conflicts](#resolving-conflicts) below.

### Step 4 — Push your branch

```bash
git push origin devN/your-feature
```

### Step 5 — Open a PR on GitHub

1. Go to the repo on GitHub
2. Click **Compare & pull request**
3. Set base: `main`, compare: `devN/your-feature`
4. Fill in the PR template completely (every checkbox)
5. Wait for the **Build Check** CI to go green
6. Request review from the supervisor (`@parnish007`)
7. Do NOT merge yourself — the supervisor merges

### Step 6 — After your PR is merged

```bash
git checkout main
git pull origin main           # sync your local main with the merge
```

---

## Build Order & When to Open PRs

| Dev | Can open PR when… |
|-----|------------------|
| Dev 1 | Immediately — no dependencies |
| Dev 2 | Dev 1 is merged into main |
| Dev 3 | Dev 2 is merged into main |
| Dev 4 | Dev 2 is merged into main |

Dev 3 and Dev 4 PRs can be open at the same time (they own different files).

---

## CI — Build Check

Every PR triggers a GitHub Actions workflow (`.github/workflows/build.yml`).
It compiles the entire project on Ubuntu with GCC. If the build fails:

- The PR shows a red ✗ next to the commit
- The supervisor will NOT merge a PR with a failing build
- Fix the error, push a new commit to your branch — CI runs again automatically

You can see the build logs by clicking the red ✗ → **Details**.

---

## CODEOWNERS

`.github/CODEOWNERS` maps every file in the repo to its owner.
GitHub uses this to automatically request a review from the right person
when a PR touches those files. Every PR requires `@parnish007` approval
regardless of which files were changed.

When a new dev joins and their GitHub username is known, update the
CODEOWNERS entry for their files:
```
# Before:
src/auth/auth.cpp    @parnish007

# After (Dev 2's GitHub username is @teammate2):
src/auth/auth.cpp    @parnish007 @teammate2
```

---

## Resolving Conflicts

If `git merge main` produces a conflict:

1. Open the conflicted file — look for the markers:
```
<<<<<<< HEAD           (your branch's version)
... your code ...
=======
... main's version ...
>>>>>>> main
```

2. Edit the file to keep the correct code (usually a combination of both).
3. Remove all `<<<<<<<`, `=======`, `>>>>>>>` markers.
4. Test that it still compiles: run `build.bat`.
5. Stage and commit the resolution:
```bash
git add <conflicted-file>
git commit -m "Dev N: resolve merge conflict in <file>"
```
6. In your PR description, explain what conflicted and what you kept.

---

## What the Supervisor (you) Does

1. Watch for incoming PRs
2. Check the CI status — green ✓ is required before review
3. Read the PR description and `updatelog/devN.md`
4. Review the diff — verify the dev only touched their assigned files
5. Approve and merge using **Squash and merge** to keep main history clean
6. After merging, check that `main` still builds (the CI runs on push to main too)

---

## GitHub Repo Settings to Enable

Go to **Settings → Branches → Add rule** for `main`:

| Setting | Value | Why |
|---------|-------|-----|
| Require a pull request before merging | ✅ On | No direct pushes to main |
| Require approvals | 1 | Supervisor must approve |
| Require status checks to pass | ✅ On, select **Build Check** | CI must be green |
| Require branches to be up to date | ✅ On | Branch must include latest main before merge |
| Include administrators | ✅ On | Rules apply to supervisor too |

These settings cannot be configured from the repo files — they must be set
in the GitHub web UI by the repo owner (`@parnish007`).
