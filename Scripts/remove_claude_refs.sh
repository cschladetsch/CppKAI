#!/bin/bash

# Script to remove Claude references from commit messages

echo "This will rewrite git history to remove Claude references."
echo "Make sure you have a backup of your repository!"
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

# Create a backup branch
git branch backup-before-claude-removal

# Remove Claude references from commit messages
git filter-branch -f --msg-filter '
    sed -e "/🤖 Generated with \[Claude Code\]/d" \
        -e "/Co-Authored-By: Claude/d" \
        -e "s/Claude/AI Assistant/g" \
        -e "s/claude/AI assistant/g"
' -- --all

echo "Done! Claude references have been removed from commit messages."
echo "A backup branch 'backup-before-claude-removal' was created."
echo "If everything looks good, you can delete the backup with:"
echo "  git branch -D backup-before-claude-removal"