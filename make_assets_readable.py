from __future__ import annotations

import gc
import os
import re
import shutil
import tempfile
from collections import Counter
from pathlib import Path

import UnityPy


ASSET_NAME_PATTERN = re.compile(
    r"^(?:sharedassets\d+\.assets|level\d+)$",
    re.IGNORECASE,
)

BACKUP_SUFFIX = ".pre-readable.bak"


def find_target_files(directory: Path) -> list[Path]:
    """Find serialized asset files without selecting resource sidecars."""
    return sorted(
        (
            path
            for path in directory.iterdir()
            if path.is_file() and ASSET_NAME_PATTERN.fullmatch(path.name)
        ),
        key=lambda path: path.name.lower(),
    )


def object_signature(env) -> Counter:
    """
    Record object identities and types.
    """
    return Counter(
        (obj.path_id, obj.type.name)
        for obj in env.objects
    )


def validate_saved_file(
    temporary_path: Path,
    expected_signature: Counter,
    patched_mesh_ids: set[int],
) -> None:
    """
    Reload the newly written file and verify its basic structure and changes.
    """
    validation_env = UnityPy.load(str(temporary_path))

    try:
        actual_signature = object_signature(validation_env)

        if actual_signature != expected_signature:
            missing = expected_signature - actual_signature
            added = actual_signature - expected_signature

            raise RuntimeError(
                "Object table changed while saving.\n"
                f"Missing/changed objects: {dict(missing)}\n"
                f"Unexpected objects: {dict(added)}"
            )

        validated_mesh_ids: set[int] = set()

        for obj in validation_env.objects:
            if obj.type.name != "Mesh":
                continue

            if obj.path_id not in patched_mesh_ids:
                continue

            tree = obj.parse_as_dict()

            if "m_IsReadable" not in tree:
                raise RuntimeError(
                    f"Mesh PathID {obj.path_id} lost its m_IsReadable field."
                )

            if not bool(tree["m_IsReadable"]):
                raise RuntimeError(
                    f"Mesh PathID {obj.path_id} was not saved as readable."
                )

            validated_mesh_ids.add(obj.path_id)

        missing_meshes = patched_mesh_ids - validated_mesh_ids
        if missing_meshes:
            raise RuntimeError(
                "The following patched meshes could not be validated: "
                f"{sorted(missing_meshes)}"
            )
    finally:
        del validation_env
        gc.collect()


def patch_asset_file(file_path: Path) -> int:
    """
    Patch all unreadable meshes in one serialized asset file.
    """
    print(f"Processing {file_path.name}...")

    env = UnityPy.load(str(file_path))
    temporary_path: Path | None = None

    try:
        original_signature = object_signature(env)
        patched_mesh_ids: set[int] = set()

        for obj in env.objects:
            if obj.type.name != "Mesh":
                continue

            tree = obj.parse_as_dict()

            if "m_IsReadable" not in tree:
                print(
                    f"  WARNING: Mesh PathID {obj.path_id} has no "
                    "m_IsReadable field; skipping it."
                )
                continue

            if bool(tree["m_IsReadable"]):
                continue

            tree["m_IsReadable"] = True
            obj.patch(tree)
            patched_mesh_ids.add(obj.path_id)

        if not patched_mesh_ids:
            print("  -> No unreadable meshes found.")
            return 0

        backup_path = file_path.with_name(file_path.name + BACKUP_SUFFIX)

        if backup_path.exists():
            raise FileExistsError(
                f"Backup already exists: {backup_path}\n"
                "Move/delete it only after confirming which copy is clean."
            )

        serialized_data = env.file.save()

        if not isinstance(serialized_data, (bytes, bytearray, memoryview)):
            raise TypeError(
                f"UnityPy returned unexpected save type: "
                f"{type(serialized_data).__name__}"
            )

        temp_fd, temp_name = tempfile.mkstemp(
            prefix=f".{file_path.name}.",
            suffix=".unitypy.tmp",
            dir=str(file_path.parent),
        )
        temporary_path = Path(temp_name)

        try:
            with os.fdopen(temp_fd, "wb") as temporary_file:
                temporary_file.write(serialized_data)
                temporary_file.flush()
                os.fsync(temporary_file.fileno())

            shutil.copymode(file_path, temporary_path)

            print(
                f"  -> Wrote temporary file; validating "
                f"{len(patched_mesh_ids)} meshes..."
            )

            validate_saved_file(
                temporary_path=temporary_path,
                expected_signature=original_signature,
                patched_mesh_ids=patched_mesh_ids,
            )

            shutil.copy2(file_path, backup_path)

            # Release UnityPy objects before replacing source file
            del env
            env = None
            gc.collect()

            os.replace(temporary_path, file_path)
            temporary_path = None

            print(
                f"  -> Successfully patched {len(patched_mesh_ids)} meshes.\n"
                f"  -> Original backed up as {backup_path.name}"
            )

            return len(patched_mesh_ids)

        except Exception:
            if temporary_path is not None:
                temporary_path.unlink(missing_ok=True)
            raise

    finally:
        if env is not None:
            del env

        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)

        gc.collect()


def main() -> None:
    working_directory = Path.cwd()
    target_files = find_target_files(working_directory)

    if not target_files:
        print("No matching Unity serialized asset files were found.")
        return

    print(f"UnityPy version: {UnityPy.__version__}")
    print(f"Found {len(target_files)} target file(s).\n")

    total_patched = 0
    failed_files: list[tuple[Path, Exception]] = []

    for file_path in target_files:
        try:
            total_patched += patch_asset_file(file_path)
        except Exception as exc:
            failed_files.append((file_path, exc))
            print(f"  ERROR: {exc}")
            print("  -> Original file was not replaced.\n")

    print(f"\nDone. Patched a total of {total_patched} meshes.")

    if failed_files:
        print(f"\n{len(failed_files)} file(s) failed:")
        for file_path, error in failed_files:
            print(f"  {file_path.name}: {error}")
        raise SystemExit(1)


if __name__ == "__main__":
    main()
