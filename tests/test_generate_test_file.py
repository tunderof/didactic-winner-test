import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
SCRIPT = ROOT_DIR / "generate_test_file.py"


class GenerateTestFileCliTest(unittest.TestCase):
    def run_script(self, *args, cwd=None):
        return subprocess.run(
            [sys.executable, str(SCRIPT), *args],
            capture_output=True,
            text=True,
            encoding="utf-8",
            cwd=cwd,
        )

    def test_short_options_generate_bin_with_requested_size(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            output_dir = Path(temp_dir)

            result = self.run_script("-s", "128B", "-t", "bin", cwd=output_dir)

            self.assertEqual(result.returncode, 0, result.stderr)
            generated_files = list(output_dir.iterdir())
            self.assertEqual(len(generated_files), 1)
            self.assertRegex(generated_files[0].name, r"^\d{8}_\d{6}_128B\.bin$")
            self.assertEqual(generated_files[0].stat().st_size, 128)

    def test_help_has_short_options_and_no_seed_option(self):
        result = self.run_script("--help")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("-s", result.stdout)
        self.assertIn("--size", result.stdout)
        self.assertIn("-t", result.stdout)
        self.assertIn("--type", result.stdout)
        self.assertNotIn("output", result.stdout.lower())
        self.assertNotIn("seed", result.stdout.lower())

    def test_size_value_is_validated_by_format(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            output_dir = Path(temp_dir)

            result = self.run_script("-s", "10XB", "-t", "bin", cwd=output_dir)

            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(list(output_dir.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
