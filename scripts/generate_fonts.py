#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
TEMP_DIR = PROJECT_ROOT / ".temp" / "fonts"
OUTPUT_DIR = PROJECT_ROOT / "main" / "fonts"
MATERIAL_SYMBOLS_HEADER = PROJECT_ROOT / "main" / "fonts" / "material_symbols.h"

INTER_GITHUB_API = "https://api.github.com/repos/rsms/inter/releases/latest"
MATERIAL_SYMBOLS_URL = "https://github.com/google/material-design-icons/raw/master/variablefont/MaterialSymbolsRounded%5BFILL%2CGRAD%2Copsz%2Cwght%5D.ttf"


def parse_material_symbols_header():
    """Extract Unicode codepoints from ICON_* defines in material_symbols.h.

    Each #define encodes a Unicode codepoint as a UTF-8 string literal, e.g.:
        #define ICON_HOME  "\\xEE\\xA2\\x8A"   ->  U+E88A
    """
    codepoints = set()
    header = MATERIAL_SYMBOLS_HEADER.read_text(encoding="utf-8")

    for line in header.splitlines():
        line = line.strip()
        if not line.startswith("#define ICON_"):
            continue

        # Extract the string literal between quotes
        start = line.find('"')
        end = line.rfind('"')
        if start == -1 or end == -1 or start == end:
            continue

        raw = line[start + 1 : end]

        # Decode the \xNN escape sequences to bytes, then UTF-8 to str
        try:
            decoded = raw.encode("ascii").decode("unicode_escape").encode("latin-1")
            char = decoded.decode("utf-8")
            cp = ord(char)
            codepoints.add(cp)
        except (UnicodeDecodeError, ValueError):
            log(f"⚠ Could not decode codepoint from: {line}")

    if not codepoints:
        log("✗ No codepoints found in material_symbols.h")
        sys.exit(1)

    sorted_cps = sorted(codepoints)
    log(f"✓ Parsed {len(sorted_cps)} icon codepoints from material_symbols.h")
    return sorted_cps


INTER_SIZES = [14, 16, 18, 24, 48]
MATERIAL_SYMBOLS_SIZES = [20, 24, 28, 40]


def log(msg):
    print(f"[generate_fonts] {msg}")


NPX = "npx.cmd" if sys.platform == "win32" else "npx"


def check_npx():
    try:
        subprocess.run(
            [NPX, "--version"],
            capture_output=True,
            check=True,
        )
        log("✓ npx found")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        log("✗ npx not found — install Node.js (https://nodejs.org/)")
        return False


def download_file(url, dest_path):
    log(f"Downloading {url}")
    try:
        urllib.request.urlretrieve(url, dest_path)
        log(f"✓ Downloaded to {dest_path}")
        return True
    except Exception as e:
        log(f"✗ Failed to download: {e}")
        return False


def download_inter_fonts():
    log("Fetching Inter release info...")
    req = urllib.request.Request(INTER_GITHUB_API, headers={"User-Agent": "ptouch-ng"})
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            tag = data["tag_name"]
            log(f"✓ Found Inter release: {tag}")
            download_url = None
            for asset in data.get("assets", []):
                if asset["name"].endswith(".zip"):
                    download_url = asset["browser_download_url"]
                    break
            if not download_url:
                log("✗ No zip asset found in release, falling back to zipball")
                download_url = data["zipball_url"]
    except Exception as e:
        log(f"✗ Failed to fetch Inter release: {e}")
        return False

    zip_path = TEMP_DIR / "inter.zip"
    if not download_file(download_url, zip_path):
        return False

    log("Extracting Inter fonts...")
    try:
        with zipfile.ZipFile(zip_path, "r") as zf:
            found_regular = False
            found_semibold = False
            for name in zf.namelist():
                basename = os.path.basename(name)
                if not basename.endswith(".ttf") or "Variable" in basename:
                    continue
                if basename == "Inter-Regular.ttf":
                    zf.extract(name, TEMP_DIR)
                    shutil.move(TEMP_DIR / name, TEMP_DIR / "Inter-Regular.ttf")
                    found_regular = True
                    log("✓ Extracted Inter-Regular.ttf")
                elif basename == "Inter-SemiBold.ttf":
                    zf.extract(name, TEMP_DIR)
                    shutil.move(TEMP_DIR / name, TEMP_DIR / "Inter-SemiBold.ttf")
                    found_semibold = True
                    log("✓ Extracted Inter-SemiBold.ttf")

        for item in TEMP_DIR.iterdir():
            if item.is_dir() and item.name != ".":
                shutil.rmtree(item, ignore_errors=True)

        if not found_regular:
            log("✗ Inter-Regular.ttf not found in archive")
            return False
        if not found_semibold:
            log("✗ Inter-SemiBold.ttf not found in archive")
            return False

        return True
    except Exception as e:
        log(f"✗ Failed to extract Inter fonts: {e}")
        return False


def download_material_symbols():
    log("Downloading Material Symbols Rounded...")
    dest = TEMP_DIR / "MaterialSymbolsRounded.ttf"
    return download_file(MATERIAL_SYMBOLS_URL, dest)


def generate_inter_fonts():
    log("Generating Inter font files...")
    regular_path = TEMP_DIR / "Inter-Regular.ttf"
    semibold_path = TEMP_DIR / "Inter-SemiBold.ttf"

    if not regular_path.exists() or not semibold_path.exists():
        log("✗ Inter font files not found")
        return False

    for size in INTER_SIZES:
        for variant, font_path in [
            ("regular", regular_path),
            ("semibold", semibold_path),
        ]:
            output_file = OUTPUT_DIR / f"inter_{variant}_{size}.c"
            cmd = [
                NPX,
                "lv_font_conv",
                "--font",
                str(font_path),
                "--size",
                str(size),
                "--bpp",
                "4",
                "--format",
                "lvgl",
                "--range",
                "0x20-0xFF",
                "--no-compress",
                "--lv-include",
                "lvgl.h",
                "--output",
                str(output_file),
            ]
            try:
                subprocess.run(cmd, check=True, capture_output=True)
                log(f"✓ Generated {output_file.name}")
            except subprocess.CalledProcessError as e:
                log(f"✗ Failed to generate {output_file.name}: {e.stderr.decode()}")
                return False

    return True


def generate_material_symbols_fonts():
    log("Generating Material Symbols font files...")
    font_path = TEMP_DIR / "MaterialSymbolsRounded.ttf"

    if not font_path.exists():
        log("✗ Material Symbols font file not found")
        return False

    codepoints = parse_material_symbols_header()
    codepoint_range = ",".join(f"0x{cp:x}" for cp in codepoints)

    for size in MATERIAL_SYMBOLS_SIZES:
        output_file = OUTPUT_DIR / f"material_symbols_{size}.c"
        cmd = [
            NPX,
            "lv_font_conv",
            "--font",
            str(font_path),
            "--size",
            str(size),
            "--bpp",
            "4",
            "--format",
            "lvgl",
            "--no-compress",
            "--lv-include",
            "lvgl.h",
            "--range",
            codepoint_range,
            "--output",
            str(output_file),
        ]
        try:
            subprocess.run(cmd, check=True, capture_output=True)
            log(f"✓ Generated {output_file.name}")
        except subprocess.CalledProcessError as e:
            log(f"✗ Failed to generate {output_file.name}: {e.stderr.decode()}")
            return False

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Download and regenerate fonts for ptouch-ng"
    )
    parser.add_argument(
        "--download-only",
        action="store_true",
        help="Only download fonts, do not regenerate",
    )
    args = parser.parse_args()

    TEMP_DIR.mkdir(parents=True, exist_ok=True)
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    log(f"Project root: {PROJECT_ROOT}")
    log(f"Temp directory: {TEMP_DIR}")
    log(f"Output directory: {OUTPUT_DIR}")

    if not download_inter_fonts():
        sys.exit(1)

    if not download_material_symbols():
        sys.exit(1)

    if args.download_only:
        log("✓ Fonts downloaded successfully (--download-only mode)")
        return

    if not check_npx():
        sys.exit(1)

    if not generate_inter_fonts():
        sys.exit(1)

    if not generate_material_symbols_fonts():
        sys.exit(1)

    log("✓ All fonts generated successfully")


if __name__ == "__main__":
    main()
