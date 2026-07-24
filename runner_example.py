#!/usr/bin/env python3
"""
runner_example.py  --  a worked example of using Single-HTM-Core on its own.

WHAT THIS IS
------------
One HTM: encoder -> Spatial Pooler -> Temporal Memory -> anomaly score, run
over a CSV a row at a time. It exists so the subtree can be checked out,
built and exercised end to end without a parent project, and so the API is
visible in one readable file rather than inferred from tests.

    python htm_install.py --bindings      # build the modules
    python runner_example.py              # after editing the block below

Everything that normally needs changing is in the EDIT THIS block at the top:
the CSV path, which columns to feed the model, and the model parameters. The
rest of the file is the run itself.

WHAT IT IS NOT
--------------
Not a pyramid. This is a single HTM over a single stream of values; several
columns are encoded and concatenated into one input SDR, not given their own
models. Multi-model hierarchies live in the project this subtree is vendored
into, not here.
"""

import csv
import math
import os
import re
import sys
import time

# ═══════════════════════════════════════════════════════════════════════════
#  EDIT THIS
# ═══════════════════════════════════════════════════════════════════════════
#
#  DATASET_PATH -- full path to a CSV with a header row, as a raw string.
#
#    Write it as r"..." whenever it contains backslashes. Without the r,
#    Python turns \n, \t and friends into control characters while reading
#    THIS file, and the path is destroyed before the script starts. Forward
#    slashes work on every OS and need no prefix.
#
DATASET_PATH = r"C:\CHANGE\ME\your_data.csv"

#  FEATURES -- the columns to feed the model, by header name.
#    Each is encoded separately and the results are concatenated into one
#    input SDR. Leave empty to use every numeric column in the file.
FEATURES = []

#  LABEL_COLUMN -- optional ground truth (0/1) used only for the summary at
#    the end. Set to None if the file has no labels.
LABEL_COLUMN = None

#  MAX_ROWS -- stop after this many rows. None reads the whole file.
MAX_ROWS = None

#  LEARN_PERIOD -- rows to treat as warm-up. Their scores are excluded from
#    the summary, because a model that has seen nothing calls everything an
#    anomaly.
LEARN_PERIOD = 500

#  ENCODER / SP / TM -- the model itself.
#    The RDSE needs `size` and `sparsity` to satisfy its collision check:
#    size >= 1000 and round(size * sparsity) >= 10 are safe.
ENCODER = dict(
    size=1000,          # bits per feature
    sparsity=0.02,      # fraction of bits active -> 20 active bits
    resolution=0.1,     # input distance that maps to a different encoding
)

SPATIAL_POOLER = dict(
    columnCount=2048,
    localAreaDensity=0.02,
    potentialPct=0.5,
    synPermActiveInc=0.003,
    synPermInactiveDec=0.0005,
    synPermConnected=0.2,
    boostStrength=0.0,
    globalInhibition=True,
    wrapAround=True,
)

TEMPORAL_MEMORY = dict(
    cellsPerColumn=8,
    activationThreshold=13,
    minThreshold=10,
    initialPermanence=0.21,
    connectedPermanence=0.5,
    permanenceIncrement=0.1,
    permanenceDecrement=0.1,
    predictedSegmentDecrement=0.0,
    maxSegmentsPerCell=128,
    maxSynapsesPerSegment=128,
    maxNewSynapseCount=20,
)

SEED = 42

#  OUTPUT_CSV -- where to write per-row scores. None prints only the summary.
OUTPUT_CSV = "runner_example_scores.csv"
# ═══════════════════════════════════════════════════════════════════════════


# --------------------------------------------------------------------------
# Reporting
# --------------------------------------------------------------------------
def _colors():
    if sys.stderr.isatty() and os.environ.get("NO_COLOR") is None:
        return "\033[91m", "\033[93m", "\033[92m", "\033[96m", "\033[1m", "\033[0m"
    return "", "", "", "", "", ""


RED, YELLOW, GREEN, CYAN, BOLD, RESET = _colors()


def log(msg):
    print(f"{CYAN}[runner]{RESET} {msg}", flush=True)


def die(headline, body=""):
    """Print a bright message naming what to fix, then exit."""
    sys.stderr.write(f"\n{RED}{BOLD}ERROR: {headline}{RESET}\n")
    if body:
        sys.stderr.write(body)
    sys.stderr.write(f"  File to edit: {YELLOW}{os.path.abspath(__file__)}{RESET}\n\n")
    sys.exit(1)


# --------------------------------------------------------------------------
# Path handling
#
# A path is accepted however it was typed. Backslashes are converted to
# forward slashes so one edit of this file works on Windows and on a cluster,
# and `~` is expanded. What cannot be repaired at runtime is a literal that
# Python already mangled while parsing this file -- so that case is detected
# by reading this file's own source line, which is the only place the
# rawness of a string still exists.
# --------------------------------------------------------------------------
_PLACEHOLDER = ("CHANGE", "ME")
_ESCAPE_DAMAGE = {"\n": r"\n", "\r": r"\r", "\t": r"\t", "\v": r"\v",
                  "\f": r"\f", "\a": r"\a", "\b": r"\b", "\0": r"\0"}
_DATA_EXTS = (".csv", ".tsv", ".txt")


def _require_raw_prefix(var):
    """Reject a backslash literal that was not written as r"..."."""
    try:
        with open(os.path.abspath(__file__), encoding="utf-8") as fh:
            for line in fh:
                m = re.match(rf'\s*{var}\s*=\s*([A-Za-z]*)(["\'])(.*?)\2', line)
                if m:
                    if "r" not in m.group(1).lower() and "\\" in m.group(3):
                        die(f"{var} is missing the r prefix.",
                            f"  The path contains backslashes, so the string must be raw:\n"
                            f'      {YELLOW}{var} = r"C:\\path\\to\\file.csv"{RESET}\n\n'
                            f"  Without the r, Python turns sequences like \\n and \\t into\n"
                            f"  control characters while reading this file, destroying the\n"
                            f"  path before the script starts.\n\n")
                    return
    except OSError:
        pass


def _reject_escape_damage(raw, var):
    hits = sorted({s for c, s in _ESCAPE_DAMAGE.items() if c in str(raw)})
    if hits:
        die(f"{var} was mangled by a backslash escape ({', '.join(hits)}).",
            f"  Python turned those into control characters while reading this file,\n"
            f"  so the path was already broken before the script started.\n\n"
            f'  {BOLD}Fix:{RESET} write it as a raw string -- '
            f'{YELLOW}{var} = r"C:\\path\\to\\file.csv"{RESET}\n\n')


def _is_full_path(path):
    """Absoluteness test that does not depend on the host OS."""
    return (os.path.isabs(path)
            or re.match(r"^[A-Za-z]:/", path) is not None
            or path.startswith("//"))


def resolve_dataset(raw, var="DATASET_PATH"):
    """Validate DATASET_PATH and return it absolute. Full paths only."""
    _require_raw_prefix(var)
    _reject_escape_damage(raw, var)
    path = os.path.expanduser(str(raw).replace("\\", "/").strip())

    if all(tok in str(raw).upper() for tok in _PLACEHOLDER):
        die(f"{var} has not been set yet.",
            f"  This file ships with a placeholder so it cannot silently read the\n"
            f"  wrong data. Set {YELLOW}{var}{RESET} in the {BOLD}EDIT THIS{RESET} "
            f"block to the full\n  path of your CSV.\n\n")

    if not _is_full_path(path):
        die(f"{var} must be a full path.",
            f"  Got: {YELLOW}{raw}{RESET}\n\n"
            f"  Give the complete path, starting from the drive letter or from the\n"
            f"  root.\n\n")

    if os.path.isdir(path):
        die("dataset not found.",
            f"  {var} : {YELLOW}{raw}{RESET}\n"
            f"  Problem      : it is a directory, not a file\n\n")

    candidates = [path]
    if not path.lower().endswith(_DATA_EXTS):
        candidates += [path + ext for ext in _DATA_EXTS]
    for cand in candidates:
        if os.path.isfile(cand):
            return os.path.abspath(cand)

    checked = ""
    if len(candidates) > 1:
        checked = ("  Checked      : "
                   + ", ".join(os.path.basename(c) for c in candidates) + "\n")
    die("dataset not found.",
        f"  {var} : {YELLOW}{raw}{RESET}\n{checked}"
        f"  Problem      : no such file\n\n")


# --------------------------------------------------------------------------
# Imports from the build tree
# --------------------------------------------------------------------------
def import_modules():
    """Import sdr / algorithms / encoders from this subtree's build output.

    Prefers an installed `htm` package if one is present; otherwise falls back
    to the .so files left in build/ by `python htm_install.py --bindings`, so
    the example runs straight after a build with nothing installed.
    """
    try:
        from htm.bindings import sdr, algorithms, encoders          # noqa
        log("using the installed htm package")
        return sdr, algorithms, encoders
    except ImportError:
        pass

    here = os.path.dirname(os.path.abspath(__file__))
    found = None
    for dirpath, _dirnames, filenames in os.walk(os.path.join(here, "build")):
        if any(f.startswith("sdr.") and f.endswith((".so", ".pyd"))
               for f in filenames):
            found = dirpath
            break

    if found is None:
        die("the Python modules are not built yet.",
            f"  Looked for sdr.* under: {os.path.join(here, 'build')}\n\n"
            f"  {BOLD}Fix:{RESET} build them first --\n"
            f"      {YELLOW}python htm_install.py --bindings{RESET}\n\n")

    sys.path.insert(0, found)
    import sdr, algorithms, encoders                                # noqa
    log(f"using modules from {os.path.relpath(found, here)}")
    return sdr, algorithms, encoders


# --------------------------------------------------------------------------
# Data
# --------------------------------------------------------------------------
def read_csv(path):
    """Read the CSV into a header list and a list of row dicts."""
    with open(path, newline="", encoding="utf-8-sig") as fh:
        reader = csv.DictReader(fh)
        if reader.fieldnames is None:
            die("the CSV has no header row.",
                f"  File: {path}\n\n"
                f"  This example identifies columns by name, so the first line must\n"
                f"  be a header.\n\n")
        header = [h.strip() for h in reader.fieldnames]
        rows = []
        for i, row in enumerate(reader):
            rows.append({(k.strip() if k else k): v for k, v in row.items()})
            if MAX_ROWS is not None and len(rows) >= MAX_ROWS:
                break
    return header, rows


def pick_features(header, rows):
    """Resolve FEATURES, or fall back to every column that parses as a number."""
    if FEATURES:
        missing = [f for f in FEATURES if f not in header]
        if missing:
            die("FEATURES names columns that are not in the CSV.",
                f"  Missing   : {YELLOW}{', '.join(missing)}{RESET}\n"
                f"  Available : {', '.join(header)}\n\n")
        return list(FEATURES)

    sample = rows[:200]
    numeric = []
    for col in header:
        if col == LABEL_COLUMN:
            continue
        values = [r.get(col) for r in sample]
        parsed = [v for v in values if _to_float(v) is not None]
        if len(parsed) >= max(1, len(values) // 2):
            numeric.append(col)

    if not numeric:
        die("no numeric columns found.",
            f"  Columns seen: {', '.join(header)}\n\n"
            f"  {BOLD}Fix:{RESET} list the ones to use in {YELLOW}FEATURES{RESET}.\n\n")
    log(f"FEATURES empty -> using every numeric column: {', '.join(numeric)}")
    return numeric


def _to_float(value):
    try:
        f = float(value)
        return f if math.isfinite(f) else None
    except (TypeError, ValueError):
        return None


# --------------------------------------------------------------------------
# The run
# --------------------------------------------------------------------------
def main():
    data_path = resolve_dataset(DATASET_PATH)
    sdr_mod, algorithms, encoders = import_modules()

    header, rows = read_csv(data_path)
    if not rows:
        die("the CSV has a header but no data rows.", f"  File: {data_path}\n\n")
    features = pick_features(header, rows)

    log(f"dataset  : {data_path}")
    log(f"rows     : {len(rows)}   features: {len(features)}")

    # ---- encoders: one per feature, concatenated into a single input SDR --
    params = encoders.RDSE_Parameters()
    params.size = ENCODER["size"]
    params.sparsity = ENCODER["sparsity"]
    params.resolution = ENCODER["resolution"]
    params.seed = SEED
    feature_encoders = {name: encoders.RDSE(params) for name in features}

    active_bits = round(ENCODER["size"] * ENCODER["sparsity"])
    if ENCODER["size"] < 1000 or active_bits < 10:
        log(f"{YELLOW}warning{RESET}: encoder size={ENCODER['size']} "
            f"activeBits={active_bits} is below the RDSE collision floor "
            f"(size >= 1000, activeBits >= 10); construction may fail.")

    input_width = ENCODER["size"] * len(features)
    encoded = sdr_mod.SDR([input_width])

    # ---- Spatial Pooler ---------------------------------------------------
    sp = algorithms.SpatialPooler(
        inputDimensions=[input_width],
        columnDimensions=[SPATIAL_POOLER["columnCount"]],
        localAreaDensity=SPATIAL_POOLER["localAreaDensity"],
        potentialPct=SPATIAL_POOLER["potentialPct"],
        synPermActiveInc=SPATIAL_POOLER["synPermActiveInc"],
        synPermInactiveDec=SPATIAL_POOLER["synPermInactiveDec"],
        synPermConnected=SPATIAL_POOLER["synPermConnected"],
        boostStrength=SPATIAL_POOLER["boostStrength"],
        globalInhibition=SPATIAL_POOLER["globalInhibition"],
        wrapAround=SPATIAL_POOLER["wrapAround"],
        seed=SEED,
    )
    active_columns = sdr_mod.SDR([SPATIAL_POOLER["columnCount"]])

    # A segment can only fire if the SP produces at least activationThreshold
    # active columns, and it produces columnCount * localAreaDensity of them.
    expected_active = (SPATIAL_POOLER["columnCount"]
                       * SPATIAL_POOLER["localAreaDensity"])
    if expected_active < TEMPORAL_MEMORY["activationThreshold"]:
        log(f"{YELLOW}warning{RESET}: ~{expected_active:.0f} active columns "
            f"cannot reach activationThreshold="
            f"{TEMPORAL_MEMORY['activationThreshold']}; the TM will never "
            f"predict and every score will be 1.0.")

    # ---- Temporal Memory --------------------------------------------------
    tm = algorithms.TemporalMemory(
        columnDimensions=[SPATIAL_POOLER["columnCount"]],
        seed=SEED,
        **TEMPORAL_MEMORY,
    )

    log(f"model    : {input_width} input bits -> "
        f"{SPATIAL_POOLER['columnCount']} columns x "
        f"{TEMPORAL_MEMORY['cellsPerColumn']} cells")
    log("running ...")

    # ---- the loop ---------------------------------------------------------
    scores, labels = [], []
    skipped = 0
    t0 = time.perf_counter()

    for i, row in enumerate(rows):
        parts = []
        usable = True
        for name in features:
            value = _to_float(row.get(name))
            if value is None:
                usable = False
                break
            parts.append(feature_encoders[name].encode(value))

        if not usable:
            skipped += 1
            scores.append(float("nan"))
            labels.append(row.get(LABEL_COLUMN) if LABEL_COLUMN else None)
            continue

        encoded.concatenate(parts)
        sp.compute(encoded, True, active_columns)
        tm.compute(active_columns, True)

        scores.append(tm.anomaly)
        labels.append(row.get(LABEL_COLUMN) if LABEL_COLUMN else None)

        if (i + 1) % 5000 == 0:
            log(f"  {i + 1}/{len(rows)} rows")

    elapsed = time.perf_counter() - t0
    rate = len(rows) / elapsed if elapsed > 0 else float("inf")
    log(f"done in {elapsed:.1f}s ({rate:.0f} rows/second)")
    if skipped:
        log(f"{YELLOW}note{RESET}: {skipped} row(s) skipped -- a feature was "
            f"missing or not numeric")

    # ---- summary ----------------------------------------------------------
    scored = [s for s in scores[LEARN_PERIOD:] if s == s]
    if scored:
        mean = sum(scored) / len(scored)
        variance = sum((s - mean) ** 2 for s in scored) / len(scored)
        top = sorted(scored, reverse=True)[:max(1, len(scored) // 100)]
        print()
        log(f"{BOLD}anomaly scores after the {LEARN_PERIOD}-row warm-up{RESET}")
        log(f"  rows scored : {len(scored)}")
        log(f"  mean        : {mean:.4f}")
        log(f"  std         : {math.sqrt(variance):.4f}")
        log(f"  min / max   : {min(scored):.4f} / {max(scored):.4f}")
        log(f"  top 1% mean : {sum(top) / len(top):.4f}")
        if mean > 0.95:
            log(f"{YELLOW}  the score is pinned near 1.0 -- the model is not "
                f"predicting.{RESET}")
            log(f"{YELLOW}  Check the SP/TM warning above, and that the "
                f"encoder resolution suits{RESET}")
            log(f"{YELLOW}  the scale of your data.{RESET}")

    if LABEL_COLUMN and any(l is not None for l in labels):
        _label_summary(scores, labels)

    # ---- output -----------------------------------------------------------
    if OUTPUT_CSV:
        out = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           OUTPUT_CSV)
        with open(out, "w", newline="", encoding="utf-8") as fh:
            writer = csv.writer(fh)
            writer.writerow(["row", "anomaly"]
                            + ([LABEL_COLUMN] if LABEL_COLUMN else []))
            for i, score in enumerate(scores):
                writer.writerow([i, "" if score != score else f"{score:.6f}"]
                                + ([labels[i]] if LABEL_COLUMN else []))
        log(f"scores written to {GREEN}{out}{RESET}")


def _label_summary(scores, labels):
    """Mean score on labelled-normal vs labelled-anomalous rows."""
    normal, anomalous = [], []
    for score, label in zip(scores[LEARN_PERIOD:], labels[LEARN_PERIOD:]):
        if score != score or label is None:
            continue
        flag = _to_float(label)
        if flag is None:
            continue
        (anomalous if flag > 0.5 else normal).append(score)

    if not normal or not anomalous:
        return
    mean_n = sum(normal) / len(normal)
    mean_a = sum(anomalous) / len(anomalous)
    var_n = sum((s - mean_n) ** 2 for s in normal) / len(normal)
    std_n = math.sqrt(var_n)
    print()
    log(f"{BOLD}against {LABEL_COLUMN}{RESET}")
    log(f"  normal rows    : {len(normal):>7}   mean score {mean_n:.4f}")
    log(f"  anomalous rows : {len(anomalous):>7}   mean score {mean_a:.4f}")
    if std_n > 0:
        log(f"  separability   : {(mean_a - mean_n) / std_n:+.2f} "
            f"standard deviations")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print()
        log("interrupted.")
        sys.exit(130)
