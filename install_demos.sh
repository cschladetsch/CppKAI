cat > ~/local/repos/CppKAI/install_demo.sh << 'EOF'
#!/bin/bash
SRC="/mnt/d/Downloads"
DEST="$HOME/local/repos/CppKAI/ContinuationMobilityDemo"

latest() {
    ls -t "$SRC/$1"* 2>/dev/null | head -1
}

for f in Description ContinuationMobilityDemo ContinuationMobilityAnimation; do
    src=$(latest "${f}")
    if [ -z "$src" ]; then
        echo "MISSING: $f"
    else
        cp "$src" "$DEST/${f}.html"
        echo "Copied: $src -> $DEST/${f}.html"
    fi
done

git -C "$HOME/local/repos/CppKAI" add ContinuationMobilityDemo/
git -C "$HOME/local/repos/CppKAI" commit -m "Update demo HTML files"
git -C "$HOME/local/repos/CppKAI" push
EOF
chmod +x ~/local/repos/CppKAI/install_demo.sh
