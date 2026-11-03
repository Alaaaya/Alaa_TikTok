from pathlib import Path
s=Path('HierarchyDump.inc').read_text();t=Path('Tweak.x').read_text()
for word in ['.text', '.attributedText', 'titleForState:', 'accessibilityLabel','accessibilityIdentifier','valueForKey:', 'description]', 'stringWithContentsOfFile:', 'dataWithContentsOf', 'NSURLSession', 'UIPasteboard', 'NSLog', 'addTarget:', 'addGestureRecognizer:', 'setFrame:', 'setHidden:', 'setAlpha:']:
    assert word not in s, word
for token in ['JK_DUMP_NODES 900','JK_DUMP_DEPTH 18','JK_DUMP_BYTES 160000','JK_DUMP_SCAN 4000','JK_DUMP_QUEUE 2000','age<=120','JK63BeforeSettings=nil','isMainThread','JK63Symbol','actionsForTarget:','queue.count>=JK_DUMP_QUEUE','candidateRoots=']:
    assert token in s, token
assert int(Path('control').read_text().split('Version: ')[1].splitlines()[0]) >= 63
assert 'joker_comments64.txt' in t
print('PASS 63 privacy/source guards and no fake style')
