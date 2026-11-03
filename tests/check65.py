from pathlib import Path
s=Path('FeedTransitions65.inc').read_text();t=Path('Tweak.x').read_text()
for bad in ['CADisplayLink','setDelegate','setContentOffset','setContentInset','setContentSize','addGestureRecognizer','addTarget:','addSubview','removeFromSuperview','setFrame','setBounds','UIPanGestureRecognizer','NSClassFromString','method_exchange','subviews','layoutIfNeeded']:
 assert bad not in s,bad
for token in ['visibleCells','UIAccessibilityIsReduceMotionEnabled','weakToStrongObjectsMapTable','CATransform3DConcat','CATransform3DEqualToTransform','isfinite(v)','v<6']:
 assert token in s,token
assert t.count('%hook AWENewFeedTableView')==1
assert '#include "FeedTransitions65.inc"' in t and 'feedPicker:' in t and 'JK65Names()[JK65Style()]' in t
assert 'JK65Before((UIScrollView *)self);\n    %orig;\n    JK65After' in t
assert int(next(l.split(':',1)[1] for l in Path('control').read_text().splitlines() if l.startswith('Version:')))>=65
print('PASS 65 source guards: observed-only table hook, visual-only transforms, no delegates/offset/insets/gestures/display link, picker, version')
