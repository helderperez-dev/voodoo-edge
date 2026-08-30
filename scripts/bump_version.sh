#!/usr/bin/env bash
# bump_version.sh — Update version across all manifests and optionally tag
#
# Usage:
#   ./scripts/bump_version.sh 1.1.0
#   ./scripts/bump_version.sh 1.1.0 --tag    # also creates a git tag
#
# This script updates the version in:
#   - library.json
#   - library.properties
#   - CHANGELOG.md (adds a new section if missing)

set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <version> [--tag]"
  echo ""
  echo "Examples:"
  echo "  $0 1.1.0"
  echo "  $0 1.1.0 --tag"
  exit 1
fi

VERSION="$1"
CREATE_TAG="${2:-}"

# Validate semver format
if ! echo "$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.]+)?$'; then
  echo "Error: '$VERSION' is not a valid semver version (e.g. 1.2.3 or 1.2.3-beta.1)"
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Bumping version to $VERSION"
echo ""

# ── library.json ──────────────────────────────────────────────────
echo "  Updating library.json..."
python3 -c "
import json, sys
with open('$ROOT_DIR/library.json', 'r') as f:
    data = json.load(f)
data['version'] = '$VERSION'
with open('$ROOT_DIR/library.json', 'w') as f:
    json.dump(data, f, indent=2)
    f.write('\n')
"
echo "    ✓ library.json"

# ── library.properties ────────────────────────────────────────────
echo "  Updating library.properties..."
sed -i.bak "s/^version=.*/version=$VERSION/" "$ROOT_DIR/library.properties"
rm -f "$ROOT_DIR/library.properties.bak"
echo "    ✓ library.properties"

# ── CHANGELOG.md ──────────────────────────────────────────────────
TODAY=$(date +%Y-%m-%d)
if grep -q "^## \[$VERSION\]" "$ROOT_DIR/CHANGELOG.md"; then
  echo "  CHANGELOG.md already has section for $VERSION — skipping"
else
  echo "  Adding section to CHANGELOG.md..."
  # Insert new version section after the header
  sed -i.bak "/^## \[/i\\
## [$VERSION] - $TODAY\\
\\
### Changed\\
- (describe changes here)\\
" "$ROOT_DIR/CHANGELOG.md"
  rm -f "$ROOT_DIR/CHANGELOG.md.bak"
  echo "    ✓ CHANGELOG.md"
fi

echo ""
echo "Version bumped to $VERSION"
echo ""

# ── Git tag ───────────────────────────────────────────────────────
if [ "$CREATE_TAG" = "--tag" ]; then
  echo "Creating git tag v$VERSION..."
  git add library.json library.properties CHANGELOG.md
  git commit -m "chore: bump version to $VERSION"
  git tag -a "v$VERSION" -m "Release v$VERSION"
  echo ""
  echo "Tagged v$VERSION. Push with:"
  echo "  git push origin main --tags"
else
  echo "Next steps:"
  echo "  1. Review and edit CHANGELOG.md"
  echo "  2. Commit: git add -A && git commit -m 'chore: bump version to $VERSION'"
  echo "  3. Tag:    git tag -a v$VERSION -m 'Release v$VERSION'"
  echo "  4. Push:   git push origin main --tags"
  echo ""
  echo "Or run: $0 $VERSION --tag"
fi
