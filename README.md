# Flush+Reload Side-Channel Attack

## Files

### Attack Code

| File | Description |
|------|-------------|
| `spy2.c` | Main spy process used to perform the Flush+Reload attack |
| `victim.sh` | Command executed as the victim process |

### Key

Located in `actual_private_key/`:

- `key.txt` — Private key in binary
- `secret.txt.asc` — Raw GPG dump from which key bits were extracted

### Key Extraction (`key_extraction_with_sleep/`)

Multiple rounds of manual and automated key bit extraction:

| File | Description |
|------|-------------|
| `key_extract.txt` | Round 1: initial manual extraction |
| `key_extract2.txt` | Round 2: manual extraction with reclassification — sequences matching multiply operation length patterns reclassified from `0` to `1` |
| `key_auto.txt` | Automated extraction via Claude Sonnet 4.6 using the heuristic: *modulo sequence length ≥ 5 → classify as `1`* |
| `key_aligned.txt` | Comparison of `key_extract.txt` against the actual key |
| `key_aligned_fixed.txt` | Comparison of `key_extract2.txt` against the actual key |
| `key_aligned_all.txt` | Side-by-side comparison of all three extraction methods against the actual key |
