from pathlib import Path
m=Path('Motion62.inc').read_text();n=Path('NoFlash.inc').read_text();t=Path('Tweak.x').read_text()
assert 'tt62.collapseMotion' in m
for token in ['isfinite(v)','v==floor(v)','UIAccessibilityIsReduceMotionEnabled','presentationLayer','JK62Generation','JK62Track','CASpringAnimation','kCAFillModeBoth','removeAnimationForKey:JK62Track']:
 assert token in m,token
for forbidden in ['CADisplayLink','layoutIfNeeded','setFrame:','removeAllAnimations','addTarget:','addGestureRecognizer:','setTransform:']:
 assert forbidden not in m,forbidden
assert 'static BOOL JK62Hold' in n and 'JK62Hold(v))return NO' in n
assert 'JK62Hold(v))return;JK60Save' in n
assert 'motionPicker:' in t and 'JK62Names()[JK62Style()]' in t
assert 'style==4' in m and 'return reduce&&style!=4?1:style' in m
assert 'JK62Finish();JK60Restore();' in Path('Style59.inc').read_text()
assert 'Version: 62' in Path('control').read_text()
# Native action / art / global glass implementations are intentionally untouched.
print('PASS 62 source guards: styles/persistence/reduce/interrupt/compositor-only/keyed cleanup')
