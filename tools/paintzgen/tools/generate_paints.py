#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, shutil, sys
from pathlib import Path

HERE = Path(__file__).resolve()
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE.parent))
from paintzgen.manifest import load_manifest
from paintzgen.ids import code_for_paint, code_to_slug, type_code
from paintzgen.render import create_base, render_label, render_surface, save_preview, save_preview_catalog, find_font
from paintzgen.dayz import emit_dayz


def load_appearance_profiles(repo_root: Path) -> dict:
    path = repo_root / 'config' / 'appearance_profiles.json'
    if not path.exists():
        return {
            'default_profile': 'used',
            'profiles': {
                'used': {'noise': 0.07, 'scratches': 0.12, 'grime': 0.08, 'rust': 0.0, 'edgewear': 0.08}
            }
        }
    data = json.loads(path.read_text(encoding='utf-8'))
    if 'profiles' not in data or not isinstance(data['profiles'], dict) or not data['profiles']:
        raise ValueError('config/appearance_profiles.json must contain a non-empty profiles object')
    default_name = data.get('default_profile', 'used')
    if default_name not in data['profiles']:
        raise ValueError(f"Appearance default_profile {default_name!r} is not defined")
    return data


def main():
    ap = argparse.ArgumentParser(description='Generate PaintZ Design 2 can labels and integration assets')
    ap.add_argument('--manifest', type=Path, default=ROOT / 'paints.json')
    ap.add_argument('--clean', action='store_true', help='remove generated output first')
    ap.add_argument('--check', action='store_true', help='validate IDs but do not render')
    args = ap.parse_args()
    manifest_path = args.manifest.resolve()
    repo_root = manifest_path.parent
    data = load_manifest(manifest_path)
    appearance_cfg = load_appearance_profiles(repo_root)
    out = repo_root / 'generated'
    if args.clean and out.exists():
        shutil.rmtree(out)
    (out / 'labels').mkdir(parents=True, exist_ok=True)
    (out / 'surfaces').mkdir(parents=True, exist_ok=True)
    (out / 'previews').mkdir(parents=True, exist_ok=True)
    (out / 'dayz').mkdir(parents=True, exist_ok=True)

    catalog = []
    codes = {}
    suggested_count = 0
    for p in data['paints']:
        code, suggested = code_for_paint(p)
        if code in codes:
            other = codes[code]
            raise SystemExit(
                'ERROR: duplicate PaintZ product code.\n'
                f"  {code}: {other['name']!r}\n"
                f"  {code}: {p['name']!r}\n"
                "Assign a different explicit 'id' suffix to one of the paints."
            )
        codes[code] = {'name': p['name'], 'type': p['type']}
        if suggested:
            suggested_count += 1
        slug = code_to_slug(code)
        catalog.append({
            'name': p['name'],
            'id': code.split('-')[-1],
            'id_source': 'suggested' if suggested else 'explicit',
            'type': p['type'],
            'type_code': type_code(p['type']),
            'code': code,
            'texture_stem': slug,
            'source': p.get('color') or p.get('pattern'),
            'appearance_profile': p.get('appearance_profile'),
            'dayz_class': p.get('dayz_class'),
        })

    if args.check:
        for x in catalog:
            marker = 'SUGGESTED' if x['id_source'] == 'suggested' else 'explicit'
            print(f"{x['code']:<16} {x['type']:<12} {marker:<9} {x['name']}")
        if suggested_count:
            print(f"WARNING: {suggested_count} paint(s) use generated ID suggestions. Add explicit 'id' values before release.")
        print(f"OK: {len(catalog)} paints; font={find_font() or 'Pillow default'}")
        return

    base = create_base(tuple(data.get('generator', {}).get('label_size', [1024, 1024])))
    base.save(repo_root / 'assets/template/military_issue_base.png')
    preview_items = []
    surface_size = tuple(data.get('generator', {}).get('surface_size', [1024, 1024]))
    for p, x in zip(data['paints'], catalog):
        label = render_label(base, p, x['code'], repo_root, appearance_cfg)
        label.save(out / 'labels' / f"{x['texture_stem']}_co.png")
        surface = render_surface(p, x['code'], repo_root, surface_size, appearance_cfg)
        surface.save(out / 'surfaces' / f"{x['texture_stem']}_co.png")
        preview_path = out / 'previews' / f"{x['texture_stem']}_preview.png"
        save_preview(label, preview_path)
        preview_items.append((p, x['code'], preview_path))
    save_preview_catalog(preview_items, out / 'preview_catalog.png')
    (out / 'catalog.json').write_text(json.dumps({
        'generator_version': '1.4.0',
        'id_scheme': 'PZ-T-CUSTOM',
        'appearance_profiles': appearance_cfg,
        'paints': catalog,
    }, indent=2) + '\n', encoding='utf-8')
    emit_dayz(catalog, data.get('dayz', {}), out / 'dayz')
    if suggested_count:
        print(f"WARNING: {suggested_count} paint(s) use generated ID suggestions. Add explicit 'id' values before release.")
    print(f"Generated {len(catalog)} paints in {out}")
    print(f"Font: {find_font() or 'Pillow default (set PAINTZ_FONT for fixed typography)'}")


if __name__ == '__main__':
    main()

