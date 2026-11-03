from pathlib import Path
s=Path('Comments65.inc').read_text();t=Path('Tweak.x').read_text()
for bad in ['.text]','stringWithContentsOfFile','NSURLSession','UIPasteboard','NSLog','addTarget:action:forControlEvents:UIControlEventTouchUpInside','addGestureRecognizer','method_exchange','objc_msgSend','removeFromSuperview','addSubview','setHidden','NSTimer','CADisplayLink']:
    assert bad not in s,bad
for tok in ['jk65.card','JK65Restore','tt.comments.style','insertSublayer','dividerAlpha','resolvedColorWithTraitCollection']:
    assert tok in s,tok
for tok in ['%hook AWECommentPanelCell','JK65AddPicker(self.stack)','prepareForReuse']:
    assert tok in t,tok
assert 'Version: 65' in Path('control').read_text()
print('PASS 65 cards: visuals only, no added views/actions, restore path present')
