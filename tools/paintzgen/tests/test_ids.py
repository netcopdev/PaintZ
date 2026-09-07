from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from paintzgen.ids import normalize_suffix, suggest_suffix, code_for_paint, type_code

assert type_code('solid') == 'S'
assert type_code('camo') == 'C'
assert type_code('pattern') == 'P'
assert type_code('metallic') == 'M'
assert type_code('rusted') == 'R'

# Explicit IDs are authoritative and normalized to uppercase.
code, suggested = code_for_paint({'id':'wht','name':'Arctic White','type':'solid','color':'#FFFFFF'})
assert code == 'PZ-S-WHT' and not suggested
code, suggested = code_for_paint({'id':'ucp','name':'Universal Camouflage Pattern','type':'camo','pattern':'x.png'})
assert code == 'PZ-C-UCP' and not suggested

# Longer custom suffixes are valid too.
assert normalize_suffix('desert1') == 'DESERT1'

# Missing IDs get deterministic descriptive suggestions, never hashes.
assert suggest_suffix('White') == 'WHT'
assert suggest_suffix('Black') == 'BLK'
assert suggest_suffix('Flat Dark Earth') == 'FDE'
assert suggest_suffix('Universal Camouflage Pattern') == 'UCP'
assert suggest_suffix('Woodland') == 'WDL'
code, suggested = code_for_paint({'name':'Desert Tan','type':'solid','color':'#B49A72'})
assert code == 'PZ-S-DTN' and suggested

# Visual changes do not affect an explicit product code.
a={'id':'ODG','name':'Olive Drab','type':'solid','color':'#556B2F','appearance_profile':'used'}
b={'id':'ODG','name':'Olive Drab','type':'solid','color':'#586D32','appearance_profile':'weathered'}
assert code_for_paint(a)[0] == code_for_paint(b)[0] == 'PZ-S-ODG'

print('PaintZ custom-ID tests OK')
