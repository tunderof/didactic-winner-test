import argparse
import random
import re
import string
import sys
from datetime import datetime
from pathlib import Path


SIZE_UNITS = {
    "B": 1,
    "KB": 1024,
    "MB": 1024 * 1024,
    "GB": 1024 * 1024 * 1024,
}

CHUNK_SIZE = 1024 * 1024
SIZE_RE = re.compile(r"^\s*(\d+(?:\.\d+)?)\s*(B|KB|MB|GB)?\s*$", re.IGNORECASE)


def configure_output_encoding() -> None:
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8")


def parse_size_parts(value: str) -> tuple[int, str]:
    match = SIZE_RE.fullmatch(value)
    if not match:
        raise argparse.ArgumentTypeError("Размер должен быть числом с необязательным суффиксом B, KB, MB или GB")

    number_text = match.group(1)
    unit = (match.group(2) or "B").upper()
    size = int(float(number_text) * SIZE_UNITS[unit])
    return size, f"{number_text}{unit}"


def random_bytes(rng: random.Random, count: int) -> bytes:
    if hasattr(rng, "randbytes"):
        return rng.randbytes(count)
    return bytes(rng.getrandbits(8) for _ in range(count))


def random_text(rng: random.Random, count: int) -> bytes:
    alphabet = string.ascii_letters + string.digits + " .,;:-_!?()[]{}\r\n"
    return "".join(rng.choice(alphabet) for _ in range(count)).encode("ascii")


def build_output_path(file_type: str, size_label: str) -> Path:
    timestamp = datetime.now().strftime("%d%m%Y_%H%M%S")
    return Path.cwd() / f"{timestamp}_{size_label}.{file_type}"


def generate_file(output_path: Path, size: int, file_type: str) -> None:
    rng = random.Random()
    bytes_left = size

    with output_path.open("wb") as output:
        while bytes_left > 0:
            current_size = min(CHUNK_SIZE, bytes_left)
            if file_type == "txt":
                data = random_text(rng, current_size)
            else:
                data = random_bytes(rng, current_size)

            output.write(data)
            bytes_left -= current_size


def main() -> int:
    configure_output_encoding()

    parser = argparse.ArgumentParser(description="Генератор случайных .bin или .txt файлов.")
    parser.add_argument("-s", "--size", required=True, help="Размер файла: 1024, 10KB, 50MB, 1GB")
    parser.add_argument("-t", "--type", choices=("bin", "txt"), default="bin", help="Тип файла")
    args = parser.parse_args()

    size, size_label = parse_size_parts(args.size)
    output_path = build_output_path(args.type, size_label)
    generate_file(output_path, size, args.type)
    print(f"Создан файл {output_path} ({size} байт, {args.type})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
