modules = [
    'bitutils',
    'property_bits',
    'property_adder',
    'property_inserter',
    'd2i_full_parser',
    'add_property_with_param',
    'add_skill_property',
    'add_blink_property',
    'reparse_modified'
]

print('Import check:')
for m in modules:
    try:
        __import__(m)
        print(f'  OK: imported {m}')
    except Exception as e:
        print(f'  FAIL: {m}: {e.__class__.__name__}: {e}')
        import traceback
        traceback.print_exc()
