#!/usr/bin/env python3
"""Copies the two guides into the release folder with their links fixed (used by make_release.sh).

    python3 -I release_links.py <fork checkout> <release folder>

new_menu/README.md becomes README.md, new_menu/CHANGELOG.md CHANGELOG.md, and NEW_MENU_README.md stays NEW_MENU_README.md.
The guide is also in payload/menu/, so it lands on the card next to menu.ini; that copy links to GitHub only (it is
not next to the front page). Links between the guides in the release folder point at each other; links to anything
else in the repository point at the source repository on GitHub.
"""
import os, re, sys

REPO = "https://github.com/emmulator/pitrex-baremetal/blob/menu-redesign/"
SHIPPED = {"NEW_MENU_README.md": "NEW_MENU_README.md", "new_menu/README.md": "README.md", "new_menu/CHANGELOG.md": "CHANGELOG.md"}


def fix(text, here, shipped=SHIPPED):
    """here: the folder of the file inside the repository ("" or "new_menu/")."""
    def link(m):
        target = m.group(1)
        if re.match(r"[a-z]+:|#", target):
            return m.group(0)
        path, _, anchor = target.partition("#")
        parts = []
        for p in (here + path).split("/"):
            if p == "..":
                parts.pop()
            elif p not in ("", "."):
                parts.append(p)
        path = "/".join(parts)
        anchor = "#" + anchor if anchor else ""
        if path in shipped:
            return "](%s%s)" % (shipped[path], anchor)
        return "](%s%s%s)" % (REPO, path, anchor)
    return re.sub(r"\]\(([^)\s]+)\)", link, text)


def main(fork, root):
    src = open(fork + "/NEW_MENU_README.md").read()
    open(root + "/NEW_MENU_README.md", "w").write(fix(src, ""))
    os.makedirs(root + "/payload/menu", exist_ok=True)
    open(root + "/payload/menu/NEW_MENU_README.md", "w").write(fix(src, "", {}))
    open(root + "/README.md", "w").write(fix(open(fork + "/new_menu/README.md").read(), "new_menu/"))
    open(root + "/CHANGELOG.md", "w").write(fix(open(fork + "/new_menu/CHANGELOG.md").read(), "new_menu/"))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
