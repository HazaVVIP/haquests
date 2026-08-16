#!/usr/bin/env python3
"""PHP-aware string encoder/decoder.

Usage:
    python3 compiler.py decode -i input.php -o decoded.php
    python3 compiler.py encode -i decoded.php -o encoded.php

The transformer deliberately leaves comments and heredoc/nowdoc blocks opaque.
That prevents source corruption because those constructs have their own grammar.
"""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

HEREDOC_HEADER = re.compile(
    r"^<<<(?P<indent>-)?[ \t]*(?:(?P<quote>['\"])(?P<quoted>[A-Za-z_][A-Za-z0-9_]*)\2|(?P<bare>[A-Za-z_][A-Za-z0-9_]*))[ \t]*$"
)
HEX = set("0123456789abcdefABCDEF")
OCT = set("01234567")
SIMPLE = {"n": 10, "r": 13, "t": 9, "v": 11, "f": 12, "e": 27, "\\": 92, '"': 34, "'": 39, "$": 36}


def decode_body(body: str, quote: str) -> str:
    """Decode PHP-like escapes, respecting PHP single-quote semantics."""
    out: list[str] = []
    i = 0
    while i < len(body):
        if body[i] != "\\" or i + 1 >= len(body):
            out.append(body[i])
            i += 1
            continue

        c = body[i + 1]
        if quote == "'":
            if c in ("'", "\\"):
                out.append(c)
                i += 2
            else:
                out.append("\\" + c)
                i += 2
            continue

        if c in "xX":
            j = i + 2
            while j < len(body) and j < i + 4 and body[j] in HEX:
                j += 1
            if j > i + 2:
                out.append(chr(int(body[i + 2:j], 16)))
                i = j
                continue
        if c in OCT:
            j = i + 1
            while j < len(body) and j < i + 4 and body[j] in OCT:
                j += 1
            out.append(chr(int(body[i + 1:j], 8)))
            i = j
            continue
        if c in SIMPLE:
            out.append(chr(SIMPLE[c]))
            i += 2
            continue

        # PHP retains the backslash for an unknown double-quoted escape.
        out.append("\\" + c)
        i += 2
    return "".join(out)


def canonicalize_body(body: str, quote: str) -> str:
    """Normalize escape spelling without destroying PHP source syntax.

    A decoded PHP source cannot contain an unescaped delimiter inside its own
    quoted literal. Therefore decode mode canonicalizes numeric escapes while
    retaining syntax-critical escapes and interpolation behavior.
    """
    out: list[str] = []
    i = 0
    while i < len(body):
        if body[i] != "\\" or i + 1 >= len(body):
            out.append(body[i])
            i += 1
            continue
        c = body[i + 1]
        if quote == "'":
            out.append("\\" + c if c in ("'", "\\") else "\\\\" + c)
            i += 2
            continue
        if c in "xX":
            j = i + 2
            while j < len(body) and j < i + 4 and body[j] in HEX:
                j += 1
            if j > i + 2:
                out.append(f"\\x{int(body[i + 2:j], 16):02x}")
                i = j
                continue
        if c in OCT:
            j = i + 1
            while j < len(body) and j < i + 4 and body[j] in OCT:
                j += 1
            out.append(f"\\x{int(body[i + 1:j], 8):02x}")
            i = j
            continue
        if c in SIMPLE:
            out.append("\\" + c)
            i += 2
            continue
        out.append("\\\\" + c)
        i += 2
    return "".join(out)


def is_interpolation_start(value: str, i: int) -> bool:
    """Whether $ at i starts PHP interpolation rather than a literal dollar."""
    if value[i] != "$" or i + 1 >= len(value):
        return False
    nxt = value[i + 1]
    return nxt == "{" or nxt == "(" or nxt == "$" or nxt == "_" or nxt.isalpha()


def encode_double(value: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(value):
        ch = value[i]
        code = ord(ch)
        # Existing PHP escapes are already encoded; do not double them.
        if ch == "\\" and i + 1 < len(value):
            out.append(value[i:i + 2])
            i += 2
        elif ch == '"':
            out.append('\\"')
            i += 1
        elif ch == "$" and not is_interpolation_start(value, i):
            out.append("\\$")
            i += 1
        elif ch == "\n":
            out.append("\\n")
            i += 1
        elif ch == "\r":
            out.append("\\r")
            i += 1
        elif ch == "\t":
            out.append("\\t")
            i += 1
        elif ch == "\v":
            out.append("\\v")
            i += 1
        elif ch == "\f":
            out.append("\\f")
            i += 1
        elif code < 32 or code == 127:
            out.append(f"\\x{code:02x}")
            i += 1
        else:
            out.append(ch)
            i += 1
    return "".join(out)


def encode_single(value: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(value):
        if value[i] == "\\" and i + 1 < len(value):
            out.append(value[i:i + 2])
            i += 2
        elif value[i] == "'":
            out.append("\\'")
            i += 1
        else:
            out.append(value[i])
            i += 1
    return "".join(out)


def consume_quoted(source: str, start: int, quote: str) -> tuple[str, int]:
    i = start + 1
    while i < len(source):
        if source[i] == "\\":
            i += 2
            continue
        if source[i] == quote:
            return source[start + 1:i], i + 1
        i += 1
    raise ValueError(f"unterminated {quote}-quoted string at byte offset {start}")


def consume_comment(source: str, start: int) -> int:
    if source.startswith("/*", start):
        end = source.find("*/", start + 2)
        return len(source) if end < 0 else end + 2
    end = source.find("\n", start + 2)
    return len(source) if end < 0 else end


def consume_heredoc(source: str, start: int) -> int | None:
    line_end = source.find("\n", start)
    if line_end < 0:
        return None
    full_line = source[source.rfind("\n", 0, start) + 1:line_end].rstrip("\r")
    marker = full_line.find("<<<")
    if marker < 0:
        return None
    header = full_line[marker:]
    match = HEREDOC_HEADER.match(header)
    if not match:
        return None
    label = match.group("quoted") or match.group("bare")
    pos = line_end + 1
    while pos <= len(source):
        next_end = source.find("\n", pos)
        if next_end < 0:
            next_end = len(source)
        line = source[pos:next_end].rstrip("\r")
        if line.strip() in (label, label + ";"):
            return next_end + (1 if next_end < len(source) else 0)
        if next_end >= len(source):
            break
        pos = next_end + 1
    raise ValueError(f"unterminated heredoc/nowdoc {label!r} at byte offset {start}")


def transform(source: str, mode: str) -> tuple[str, int, int]:
    out: list[str] = []
    i = 0
    strings = 0
    opaque = 0
    while i < len(source):
        if source.startswith("//", i) or source[i] == "#":
            end = consume_comment(source, i)
            out.append(source[i:end])
            i = end
            continue
        if source.startswith("/*", i):
            end = consume_comment(source, i)
            out.append(source[i:end])
            i = end
            continue
        if source.startswith("<<<", i):
            end = consume_heredoc(source, i)
            if end is not None:
                out.append(source[i:end])
                opaque += 1
                i = end
                continue
        if source[i] in ("'", '"'):
            quote = source[i]
            body, end = consume_quoted(source, i, quote)
            converted = canonicalize_body(body, quote) if mode == "decode" else (encode_double(body) if quote == '\"' else encode_single(body))
            out.append(quote + converted + quote)
            strings += 1
            i = end
            continue
        out.append(source[i])
        i += 1
    return "".join(out), strings, opaque


def sha256(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(prog="compiler.py")
    parser.add_argument("mode", choices=("decode", "encode"))
    parser.add_argument("-i", "--input", required=True, type=Path)
    parser.add_argument("-o", "--output", required=True, type=Path)
    args = parser.parse_args()
    if not args.input.is_file():
        raise SystemExit(f"error: input file not found: {args.input}")
    source = args.input.read_text(encoding="utf-8", errors="replace")
    result, strings, opaque = transform(source, args.mode)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(result, encoding="utf-8")
    print(f"mode            {args.mode}")
    print(f"input           {args.input}")
    print(f"output          {args.output}")
    print(f"strings         {strings}")
    print(f"opaque_blocks   {opaque}")
    print(f"input_sha256    {sha256(source)}")
    print(f"output_sha256   {sha256(result)}")
    print(f"output_bytes    {len(result.encode('utf-8'))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
