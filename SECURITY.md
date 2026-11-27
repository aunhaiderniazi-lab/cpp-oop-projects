# Security notes

- **smart-bank/bank.cpp**: PINs are no longer written to `bank.txt`. Records now store a random salt plus a salted FNV-1a digest, and legacy cleartext PIN files are migrated on load. Month-end processing is limited to once per calendar month (persisted across restarts). EOF is handled gracefully so the destructor always saves. New account IDs use max(existing)+1 instead of `1001 + count`.
- **course-registration/reg.cpp**: the hardcoded admin password was removed; set the `ADMIN_PASSWORD` environment variable to enable admin features. Course lookups are null-checked before use, malformed `students.txt` records are rejected safely, EOF no longer bypasses the destructor, and new student IDs use max(existing)+1.

FNV-1a is a fast non-cryptographic hash used here only to avoid storing cleartext PINs in a lab project. For a production system use a memory-hard KDF (Argon2/bcrypt/scrypt) with a high work factor.
