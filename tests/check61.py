from pathlib import Path
p=Path('PhaseOne.inc').read_text();b=Path('BarColor61.inc').read_text();g=Path('Glass61.inc').read_text()
assert 'tt61.feedFloating' in p and 'TT61FloatingHidden' in p
assert 'if(b.tag!=1221&&b.tag!=1222)return;' in p
assert 'TT61FloatingButton' in p and 'return self.bounds;' in p
assert 'UIImageRenderingModeAlwaysOriginal' in Path('CustomTools.inc').read_text()
for name in ['EmojiBar.inc','SheetEye.inc']:
 s=Path(name).read_text();assert 'TTToolGlyph' not in s and 'TT61ButtonArt' not in s
assert 'JK61ActualBar' in b and 'setFrame:' not in b and '.hidden=' not in b and '.alpha=' not in b
assert 'UIColorPickerViewController' in Path('Tweak.x').read_text()
assert 'weakToStrongObjectsMapTable' in g and 'fractionComplete' in g
assert 'setValue:' not in g and 'appearance]' not in g
print('PASS 61 ownership/image-isolation/bottom-paint source guards')
