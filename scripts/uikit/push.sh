#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: scripts/uikit/push.sh [--label LABEL] [--bucket BUCKET]

Archives src/ui/assets/uikit, uploads it to Google Cloud Storage, and updates
ci/uikit.lock.json with the selected object path and SHA-256 checksum.

Options:
  --label   Archive label. Defaults to the current UTC timestamp.
  --bucket  GCS bucket name. Defaults to private_assets_analyzer.
  --help    Show this help message.
EOF
}

bucket="private_assets_analyzer"
label="$(date -u +%Y%m%d-%H%M%S)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --label)
            [[ $# -ge 2 ]] || {
                echo "--label requires a value" >&2
                exit 1
            }
            label="$2"
            shift 2
            ;;
        --bucket)
            [[ $# -ge 2 ]] || {
                echo "--bucket requires a value" >&2
                exit 1
            }
            bucket="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

command -v gcloud >/dev/null 2>&1 || {
    echo "gcloud is required but not installed." >&2
    exit 1
}

command -v shasum >/dev/null 2>&1 || {
    echo "shasum is required but not installed." >&2
    exit 1
}

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/../.." && pwd)"
assets_parent_dir="${repo_root}/src/ui/assets"
source_dir="${assets_parent_dir}/uikit"
lock_file="${repo_root}/ci/uikit.lock.json"

if [[ ! -d "$source_dir" ]]; then
    echo "Missing UIKit directory: $source_dir" >&2
    exit 1
fi

archive_name="${label}.tar.gz"
remote_path="gs://${bucket}/uikit/${archive_name}"
tmp_dir="$(mktemp -d)"
archive_path="${tmp_dir}/${archive_name}"

cleanup() {
    rm -rf "${tmp_dir}"
}
trap cleanup EXIT

tar -C "$assets_parent_dir" -czf "$archive_path" "uikit"
sha256="$(shasum -a 256 "$archive_path" | awk '{print $1}')"

gcloud storage cp "$archive_path" "$remote_path"

tmp_lock="$(mktemp)"
cat > "$tmp_lock" <<EOF
{
  "gcs_path": "${remote_path}",
  "sha256": "${sha256}"
}
EOF

mv "$tmp_lock" "$lock_file"

printf 'Uploaded UIKit archive to %s\n' "$remote_path"
printf 'Updated lock file: %s\n' "$lock_file"
printf 'SHA-256: %s\n' "$sha256"
