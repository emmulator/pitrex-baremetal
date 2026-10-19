#!/usr/bin/env python3
"""Copies the two guides into the release folder with their links fixed (used by make_release.sh).

    python3 -I release_links.py <fork checkout> <release folder>

new_menu/README.md becomes README.md, new_menu/CHANGELOG.md CHANGELOG.md, and NEW_MENU_README.md stays NEW_MENU_README.md (also in payload/, so it lands on the
card). Links between the two point at each other; links to anything else in the repository point at the source
repository on GitHub.
"""
import re, sys

REPO = "https://github.com/emmulator/pitrex-baremetal/blob/menu-redesign/"
SHIPPED = {"NEW_MENU_README.md": "NEW_MENU_README.md", "new_menu/README.md": "README.md", "new_menu/CHANGELOG.md": "CHANGELOG.md"}


def fix(text, here):
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
        if path in SHIPPED:
            return "](%s%s)" % (SHIPPED[path], anchor)
        return "](%s%s%s)" % (REPO, path, anchor)
    return re.sub(r"\]\(([^)\s]+)\)", link, text)


def main(fork, root):
    nm = fix(open(fork + "/NEW_MENU_README.md").read(), "")
    for out in (root + "/NEW_MENU_README.md", root + "/payload/NEW_MENU_README.md"):
        open(out, "w").write(nm)
    open(root + "/README.md", "w").write(fix(open(fork + "/new_menu/README.md").read(), "new_menu/"))
    open(root + "/CHANGELOG.md", "w").write(fix(open(fork + "/new_menu/CHANGELOG.md").read(), "new_menu/"))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
