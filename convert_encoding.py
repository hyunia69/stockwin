# -*- coding: utf-8 -*-
import chardet
import os
import glob

base_path = r'C:\Work\dasam\code\claude\windows_PRI\ALLAT_StockWin_Billkey_Easy_New_Scenario'
files = glob.glob(os.path.join(base_path, '**/*.cpp'), recursive=True) + \
        glob.glob(os.path.join(base_path, '**/*.h'), recursive=True) + \
        glob.glob(os.path.join(base_path, '**/*.hpp'), recursive=True)

converted = []
skipped = []
errors = []

for filepath in sorted(files):
    # Skip this script itself
    if 'convert_encoding.py' in filepath:
        continue

    with open(filepath, 'rb') as f:
        raw = f.read()

    rel_path = os.path.relpath(filepath, base_path)

    # Check if already UTF-8 with BOM
    if raw[:3] == b'\xef\xbb\xbf':
        skipped.append(rel_path)
        continue

    detected = chardet.detect(raw)
    enc = detected['encoding']

    if enc in ['EUC-KR', 'CP949', 'euc-kr', 'cp949']:
        source_enc = 'cp949'
    elif enc in ['ascii', 'ASCII']:
        source_enc = 'ascii'
    elif enc in ['UTF-8', 'utf-8']:
        source_enc = 'utf-8'
    else:
        source_enc = enc

    try:
        text = raw.decode(source_enc)
        utf8_bom = b'\xef\xbb\xbf' + text.encode('utf-8')
        with open(filepath, 'wb') as f:
            f.write(utf8_bom)
        converted.append(rel_path)
        print(f'[OK] {rel_path} ({enc} -> UTF-8 BOM)')
    except Exception as e:
        errors.append((rel_path, str(e)))
        print(f'[ERR] {rel_path}: {e}')

print('')
print('=== Summary ===')
print(f'Converted: {len(converted)} files')
print(f'Skipped (already UTF-8 BOM): {len(skipped)} files')
print(f'Errors: {len(errors)} files')

if errors:
    print('\nError files:')
    for f, e in errors:
        print(f'  - {f}: {e}')
