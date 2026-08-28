"""
Character Swapper - swap the playable character on a pawn Blueprint.

Replaces the skeletal mesh on a Character Blueprint's Mesh component (e.g.
BP_TopDownCharacter), optionally refitting the capsule and mesh offset so
characters of different heights stand correctly.

Usage from the Unreal Python console:

    import character_swapper as cs
    cs.list_characters("/Game/Characters")
    cs.get_current("/Game/TopDown/Blueprints/BP_TopDownCharacter")
    cs.swap("/Game/TopDown/Blueprints/BP_TopDownCharacter",
            "/Game/Characters/GirlCharacter/SKM_Girl_Character")
    cs.restore("/Game/TopDown/Blueprints/BP_TopDownCharacter")

Design notes
------------
* Mesh and CapsuleComponent on ACharacter are NATIVE components (created in
  C++), so they live on the Blueprint's Class Default Object and are edited
  there. Components added inside a Blueprint would instead need editing via
  the SimpleConstructionScript.
* A mesh whose skeleton differs from the pawn's current one is REFUSED: the
  pawn's Animation Blueprint is bound to a specific skeleton, and swapping in
  a foreign mesh produces a frozen character rather than an obvious error.
* Auto-fit keeps the invariant "mesh feet sit at the capsule bottom":
  half_height = character_height / 2, and mesh relative Z = -half_height.
  The capsule RADIUS is deliberately left alone - it affects how close the
  character can get to walls, which is gameplay tuning, not a mesh fix.
"""

import json
import os

import unreal

_BACKUP_NAME = "character_swapper_backup.json"


# --------------------------------------------------------------------- utils


def _backup_path():
    return os.path.join(unreal.Paths.project_saved_dir(), _BACKUP_NAME)


def _load_backups():
    path = _backup_path()
    if not os.path.isfile(path):
        return {}
    try:
        with open(path, "r", encoding="utf-8") as handle:
            return json.load(handle)
    except Exception:
        return {}


def _save_backups(data):
    try:
        with open(_backup_path(), "w", encoding="utf-8") as handle:
            json.dump(data, handle, indent=2)
    except Exception as exc:
        unreal.log_warning("Could not write backup file: %s" % exc)


def _obj_path(asset):
    """Content path of an asset, without the .ClassName suffix."""
    if asset is None:
        return None
    return asset.get_path_name().split(".")[0]


def _load(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        raise ValueError("Could not load asset: %s" % path)
    return asset


def _pawn_parts(pawn_bp_path):
    """Return (blueprint, cdo, mesh_component, capsule_component)."""
    bp = _load(pawn_bp_path)
    gen = bp.generated_class()
    if gen is None:
        raise ValueError("%s is not a Blueprint with a generated class." % pawn_bp_path)
    cdo = unreal.get_default_object(gen)
    if cdo is None:
        raise ValueError("Could not get the default object for %s" % pawn_bp_path)

    mesh = cdo.get_editor_property("mesh")
    if mesh is None:
        raise ValueError(
            "%s has no 'Mesh' component. Is it a Character Blueprint?" % pawn_bp_path
        )
    try:
        capsule = cdo.get_editor_property("capsule_component")
    except Exception:
        capsule = None
    return bp, cdo, mesh, capsule


def _skeleton_of(mesh_asset):
    try:
        return mesh_asset.get_editor_property("skeleton")
    except Exception:
        return None


def _height_of(mesh_asset):
    """Total height of the mesh in cm, from its reference-pose bounds."""
    bounds = mesh_asset.get_bounds()
    return float(bounds.box_extent.z) * 2.0


# ---------------------------------------------------------------- public API


def list_characters(folder="/Game", require_skeleton=None):
    """List skeletal mesh asset paths under `folder`.

    require_skeleton: optional Skeleton asset or content path. When given,
    only meshes using that skeleton are returned.
    """
    if isinstance(require_skeleton, str):
        require_skeleton = unreal.EditorAssetLibrary.load_asset(require_skeleton)

    found = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(folder, recursive=True):
        data = unreal.EditorAssetLibrary.find_asset_data(asset_path)
        if data is None:
            continue
        if data.asset_class_path.asset_name != "SkeletalMesh":
            continue
        clean = asset_path.split(".")[0]
        if require_skeleton is not None:
            mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
            if mesh is None or _skeleton_of(mesh) != require_skeleton:
                continue
        found.append(clean)
    return sorted(found)


def get_current(pawn_bp_path):
    """Report what the pawn currently uses."""
    _bp, _cdo, mesh, capsule = _pawn_parts(pawn_bp_path)
    current = mesh.get_editor_property("skeletal_mesh_asset")
    anim = mesh.get_editor_property("anim_class")
    loc = mesh.get_editor_property("relative_location")
    info = {
        "pawn": pawn_bp_path,
        "mesh": _obj_path(current),
        "skeleton": _obj_path(_skeleton_of(current)) if current else None,
        "anim_class": anim.get_name() if anim else None,
        "mesh_z": round(float(loc.z), 3),
        "capsule_half_height": (
            round(float(capsule.get_editor_property("capsule_half_height")), 3)
            if capsule
            else None
        ),
        "capsule_radius": (
            round(float(capsule.get_editor_property("capsule_radius")), 3)
            if capsule
            else None
        ),
    }
    if current is not None:
        info["mesh_height"] = round(_height_of(current), 2)
    return info


def preview_fit(mesh_path):
    """What auto-fit WOULD apply for this mesh, without changing anything."""
    mesh_asset = _load(mesh_path)
    height = _height_of(mesh_asset)
    half = height / 2.0
    return {
        "mesh": mesh_path,
        "height": round(height, 2),
        "capsule_half_height": round(half, 2),
        "mesh_z": round(-half, 2),
    }


def swap(
    pawn_bp_path,
    mesh_path,
    auto_fit=True,
    half_height=None,
    mesh_z=None,
    allow_skeleton_mismatch=False,
):
    """Make `mesh_path` the pawn's character.

    auto_fit   - recompute capsule half height and mesh Z from the mesh bounds.
    half_height / mesh_z - explicit overrides; either one wins over auto_fit.
    """
    bp, _cdo, mesh_comp, capsule = _pawn_parts(pawn_bp_path)
    new_mesh = _load(mesh_path)

    if not isinstance(new_mesh, unreal.SkeletalMesh):
        raise ValueError("%s is not a Skeletal Mesh." % mesh_path)

    current = mesh_comp.get_editor_property("skeletal_mesh_asset")
    ref_skeleton = _skeleton_of(current) if current is not None else None
    new_skeleton = _skeleton_of(new_mesh)

    if (
        ref_skeleton is not None
        and new_skeleton is not None
        and ref_skeleton != new_skeleton
        and not allow_skeleton_mismatch
    ):
        raise ValueError(
            "Skeleton mismatch.\n"
            "  pawn uses : %s\n"
            "  mesh uses : %s\n"
            "The pawn's Animation Blueprint is bound to the first skeleton, so this\n"
            "mesh would swap in and then not animate. Re-export the character bound\n"
            "to the same skeleton, or pass allow_skeleton_mismatch=True if you also\n"
            "intend to change the Animation Blueprint."
            % (_obj_path(ref_skeleton), _obj_path(new_skeleton))
        )

    # remember the original state once, so Restore is a true undo
    backups = _load_backups()
    if pawn_bp_path not in backups and current is not None:
        loc = mesh_comp.get_editor_property("relative_location")
        backups[pawn_bp_path] = {
            "mesh": _obj_path(current),
            "mesh_z": float(loc.z),
            "capsule_half_height": (
                float(capsule.get_editor_property("capsule_half_height"))
                if capsule
                else None
            ),
        }
        _save_backups(backups)

    # ---- apply
    mesh_comp.set_editor_property("skeletal_mesh_asset", new_mesh)

    applied_half = None
    applied_z = None
    if auto_fit or half_height is not None or mesh_z is not None:
        fit = preview_fit(mesh_path)
        applied_half = half_height if half_height is not None else fit["capsule_half_height"]
        applied_z = mesh_z if mesh_z is not None else fit["mesh_z"]
        if capsule is not None:
            capsule.set_editor_property("capsule_half_height", float(applied_half))
        loc = mesh_comp.get_editor_property("relative_location")
        mesh_comp.set_editor_property(
            "relative_location", unreal.Vector(loc.x, loc.y, float(applied_z))
        )

    _finalise(bp, pawn_bp_path)

    result = {
        "swapped_to": mesh_path,
        "capsule_half_height": applied_half,
        "mesh_z": applied_z,
        "skeleton": _obj_path(new_skeleton),
    }
    unreal.log("Character Swapper: %s" % json.dumps(result))
    return result


def restore(pawn_bp_path):
    """Put back the mesh, capsule and offset recorded on the first swap."""
    backups = _load_backups()
    saved = backups.get(pawn_bp_path)
    if not saved:
        raise ValueError(
            "No saved original for %s. Restore only works after a swap." % pawn_bp_path
        )

    bp, _cdo, mesh_comp, capsule = _pawn_parts(pawn_bp_path)
    original = _load(saved["mesh"])
    mesh_comp.set_editor_property("skeletal_mesh_asset", original)

    loc = mesh_comp.get_editor_property("relative_location")
    mesh_comp.set_editor_property(
        "relative_location", unreal.Vector(loc.x, loc.y, float(saved["mesh_z"]))
    )
    if capsule is not None and saved.get("capsule_half_height") is not None:
        capsule.set_editor_property(
            "capsule_half_height", float(saved["capsule_half_height"])
        )

    _finalise(bp, pawn_bp_path)
    unreal.log("Character Swapper: restored %s" % saved["mesh"])
    return saved


def _finalise(bp, pawn_bp_path):
    """Compile and save the pawn Blueprint so the change sticks."""
    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    except Exception as exc:
        unreal.log_warning("Compile step failed: %s" % exc)
    try:
        unreal.EditorAssetLibrary.save_asset(pawn_bp_path, only_if_is_dirty=False)
    except Exception as exc:
        unreal.log_warning("Save step failed: %s" % exc)


# ------------------------------------------------------- settings + menu API
#
# Everything below is what the Tools menu calls. It works off the Content
# Browser selection so there is no UI to build.

_SETTINGS_NAME = "character_swapper_settings.json"
_DEFAULT_PAWN = "/Game/TopDown/Blueprints/BP_TopDownCharacter"


def _settings_path():
    return os.path.join(unreal.Paths.project_saved_dir(), _SETTINGS_NAME)


def get_pawn():
    """The pawn Blueprint the menu acts on."""
    try:
        with open(_settings_path(), "r", encoding="utf-8") as handle:
            return json.load(handle).get("pawn", _DEFAULT_PAWN)
    except Exception:
        return _DEFAULT_PAWN


def set_pawn(pawn_bp_path):
    with open(_settings_path(), "w", encoding="utf-8") as handle:
        json.dump({"pawn": pawn_bp_path}, handle, indent=2)
    return pawn_bp_path


def _dialog(title, message):
    try:
        unreal.EditorDialog.show_message(
            title, message, unreal.AppMsgType.OK
        )
    except Exception:
        unreal.log("%s: %s" % (title, message))


def _selected_assets():
    try:
        return list(unreal.EditorUtilityLibrary.get_selected_assets())
    except Exception:
        return []


def menu_make_playable():
    """Tools > Character Swapper > Make Selected Playable."""
    chosen = [a for a in _selected_assets() if isinstance(a, unreal.SkeletalMesh)]
    if not chosen:
        _dialog(
            "Character Swapper",
            "Select a Skeletal Mesh in the Content Browser first.",
        )
        return
    if len(chosen) > 1:
        _dialog(
            "Character Swapper",
            "Select exactly one Skeletal Mesh (you have %d selected)." % len(chosen),
        )
        return

    mesh_path = _obj_path(chosen[0])
    pawn = get_pawn()
    try:
        result = swap(pawn, mesh_path)
    except Exception as exc:
        _dialog("Character Swapper - not swapped", str(exc))
        return
    _dialog(
        "Character Swapper",
        "Now playing as:\n  %s\n\nCapsule half height: %s\nMesh Z offset: %s\nPawn: %s"
        % (
            mesh_path.rsplit("/", 1)[-1],
            result["capsule_half_height"],
            result["mesh_z"],
            pawn,
        ),
    )


def menu_restore():
    """Tools > Character Swapper > Restore Original."""
    pawn = get_pawn()
    try:
        saved = restore(pawn)
    except Exception as exc:
        _dialog("Character Swapper - not restored", str(exc))
        return
    _dialog(
        "Character Swapper",
        "Restored original character:\n  %s" % saved["mesh"],
    )


def menu_show_current():
    """Tools > Character Swapper > Show Current."""
    pawn = get_pawn()
    try:
        info = get_current(pawn)
    except Exception as exc:
        _dialog("Character Swapper", str(exc))
        return
    lines = [
        "Pawn:      %s" % info["pawn"],
        "Character: %s" % info["mesh"],
        "Skeleton:  %s" % info["skeleton"],
        "AnimBP:    %s" % info["anim_class"],
        "Height:    %s" % info.get("mesh_height"),
        "Capsule:   %s   Mesh Z: %s"
        % (info["capsule_half_height"], info["mesh_z"]),
    ]
    _dialog("Character Swapper - current", "\n".join(lines))


def register_menu():
    """Add 'Character Swapper' to the editor's Tools menu.

    Safe to call repeatedly. Called automatically by init_unreal.py at editor
    startup; call it by hand to register without restarting.
    """
    entries = [
        ("MakePlayable", "Make Selected Playable", "menu_make_playable",
         "Swap the Content Browser selection in as the playable character."),
        ("Restore", "Restore Original", "menu_restore",
         "Put back the character that was there before the first swap."),
        ("ShowCurrent", "Show Current", "menu_show_current",
         "Report which character, skeleton and capsule the pawn is using."),
        ("SetPawn", "Set Pawn Blueprint From Selection",
         "menu_set_pawn_from_selection",
         "Use the selected Character Blueprint as the pawn to modify."),
    ]

    menus = unreal.ToolMenus.get()
    tools = menus.find_menu("LevelEditor.MainMenu.Tools")
    if tools is None:
        unreal.log_warning("Character Swapper: Tools menu not found.")
        return False

    submenu = tools.add_sub_menu(
        tools.get_name(), "Python", "CharacterSwapper", "Character Swapper"
    )
    if submenu is None:
        unreal.log_warning("Character Swapper: could not create the submenu.")
        return False

    for name, label, func, tip in entries:
        entry = unreal.ToolMenuEntry(name=name, type=unreal.MultiBlockType.MENU_ENTRY)
        entry.set_label(label)
        entry.set_tool_tip(tip)
        entry.set_string_command(
            unreal.ToolMenuStringCommandType.PYTHON,
            "",
            string="import character_swapper as cs; cs.%s()" % func,
        )
        submenu.add_menu_entry("Actions", entry)

    menus.refresh_all_widgets()
    unreal.log("Character Swapper: menu registered under Tools.")
    return True


def menu_set_pawn_from_selection():
    """Tools > Character Swapper > Set Pawn Blueprint From Selection."""
    chosen = [a for a in _selected_assets() if isinstance(a, unreal.Blueprint)]
    if len(chosen) != 1:
        _dialog(
            "Character Swapper",
            "Select exactly one Character Blueprint in the Content Browser.",
        )
        return
    path = _obj_path(chosen[0])
    try:
        _pawn_parts(path)          # validate it really is a Character BP
    except Exception as exc:
        _dialog("Character Swapper - not set", str(exc))
        return
    set_pawn(path)
    _dialog("Character Swapper", "Pawn Blueprint set to:\n  %s" % path)
