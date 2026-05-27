#!/bin/bash
set -e
REPO="$HOME/local/repos/CppKAI"
DEST="$REPO/ContinuationMobilityDemo"
SRC="$(cd "$(dirname "$0")" && pwd)"

echo "RhoMog Model installer"
echo "Source : $SRC"
echo "Dest   : $DEST"
echo ""

missing=0
for f in style.css ContinuationMobilityAnimation.html ContinuationMobilityDemo.html Description.html; do
    [ -f "$SRC/$f" ] || { echo "MISSING: $f"; missing=1; }
done
[ $missing -eq 1 ] && { echo "Aborting — re-download the zip."; exit 1; }

for f in style.css ContinuationMobilityAnimation.html ContinuationMobilityDemo.html Description.html; do
    cp "$SRC/$f" "$DEST/$f" && echo "  ✓ $f"
done

[ -f "$SRC/ContinuationMobilityDemo.rho" ] && {
    cp "$SRC/ContinuationMobilityDemo.rho" "$DEST/ContinuationMobilityDemo.rho"
    echo "  ✓ ContinuationMobilityDemo.rho"
}

[ -f "$SRC/README.md" ] && {
    cp "$SRC/README.md" "$REPO/README.md"
    echo "  ✓ README.md"
}

git -C "$REPO" add \
    ContinuationMobilityDemo/style.css \
    ContinuationMobilityDemo/ContinuationMobilityAnimation.html \
    ContinuationMobilityDemo/ContinuationMobilityDemo.html \
    ContinuationMobilityDemo/Description.html \
    ContinuationMobilityDemo/ContinuationMobilityDemo.rho \
    README.md 2>/dev/null || true

git -C "$REPO" commit -m "RhoMog Model: shared CSS, fantasy reskin, Description page, speed slider"
git -C "$REPO" push

echo ""
echo "Live at: https://cschladetsch.github.io/CppKAI/ContinuationMobilityDemo/"
