from pathlib import Path
s=Path('HierarchyDump.inc').read_text();t=Path('Tweak.x').read_text()
for word in ['.text', '.attributedText', 'titleForState:', 'accessibilityLabel','accessibilityIdentifier','valueForKey:', 'description]', 'stringWithContentsOfFile:', 'dataWithContentsOf', 'NSURLSession', 'UIPasteboard', 'NSLog', 'addTarget:', 'addGestureRecognizer:', 'setFrame:', 'setHidden:', 'setAlpha:']:
    assert word not in s, word
for token in ['JK_DUMP_NODES 900','JK_DUMP_DEPTH 18','JK_DUMP_BYTES 160000','JK_DUMP_SCAN 4000','JK_DUMP_QUEUE 2000','age<=120','JK63BeforeSettings=nil','isMainThread','JK63Symbol','actionsForTarget:','queue.count>=JK_DUMP_QUEUE','candidateRoots=']:
    assert token in s, token
assert 'Version: 63' in Path('control').read_text()
assert 'JK63RememberBeforeSettings();' in t
assert t.index('JK63RememberBeforeSettings();',t.index('static void TTGShowSettings(UITabBar *bar) {')) < t.index('[top presentViewController:controller')
assert 'joker_comments63.txt' in t
assert 'كروت' not in t # No fake selectable style.
print('PASS 63 privacy/source guards and no fake style')
