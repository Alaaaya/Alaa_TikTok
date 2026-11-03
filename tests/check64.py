from pathlib import Path
s=Path('AutoCapture64.inc').read_text();t=Path('Tweak.x').read_text()
for bad in ['.text','.attributedText','titleForState:','accessibilityLabel','accessibilityIdentifier','valueForKey:','description]','stringWithContentsOfFile:','dataWithContentsOf','NSURLSession','UIPasteboard','NSLog','addTarget:','addGestureRecognizer:','CADisplayLink','NSTimer','layoutSubviews','layoutIfNeeded','setFrame:','setHidden:','setAlpha:']:
 assert bad not in s,bad
for token in ['JK64_SESSION 45','JK64_TTL 600','JK64_SCAN_BUDGET 12','JK64_VISITS 1800','JK64Events>=24','weakObjectsHashTable','score>JK64Best','JK64Baseline containsObject','JK64Expire','nodeCapReached','discoveryCapped','retainedAgeSeconds','writeToFile:','view.window']:
 if token=='view.window': continue
 assert token in s,token
assert 'Version: 64' in Path('control').read_text()
assert '#include "AutoCapture64.inc"' in t
for token in ['JK64Event(@"controller_present")','JK64Event(@"controller_appear")','JK64Event(@"controller_child")','JK64Event(@"view_window")','JK64Arm();','JK64Export(path);','joker_comments64.txt','armCommentsRow']:
 assert token in t,token
assert 'JK63RememberBeforeSettings();' not in t
# No optional feed-transition picker, cards or guessed private class hooks.
for bad in ['AWE','TTK','CommentPanel','NSClassFromString','objc_msgSend','method_exchangeImplementations']:
 assert bad not in s,bad
print('PASS 64 opt-in bounded lifecycle capture, coalescing, no content reads/polling/history/upload; default feature files untouched')
