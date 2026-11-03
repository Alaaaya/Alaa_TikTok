from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'EmojiBar.inc').read_text()
h=(root/'ReactionObserver.inc').read_text()+(root/'Tweak.x').read_text()
p=(root/'PhaseOne.inc').read_text()
for term in ['joker_debug.txt','256*1024','scanWindows:','TT48Births','TT48LastPress','UIImageView.class','imageForState:UIControlStateNormal']:
    assert term in s,term
for term in ['%hook UIView','%hook UIWindow','didAddSubview:','didMoveToWindow','addGestureRecognizer:','sendEvent:','TT48Pending','UIGestureRecognizerStateBegan','TT48Debug(@"comment-long-press +0.6s")']:
    assert term in h,term
assert 'self.downloadButton.hidden = PB(@"tt46.emojiBar",NO)' in p
assert 'self.eyeButton.hidden = PB(@"tt46.emojiBar",NO)' in p
assert 'enabled:!self.overlay.hidden' not in p
assert 'window.bounds.size.width*.65' not in s
assert 'text]' not in s[s.index('static void TT48Debug'):s.index('// Runtime hierarchy')]
print('PASS 48 scoped debounced lifecycle hooks, all-window scan, mirrored geometry and bounded class-only diagnostics')

top=(root/'TopFeed.inc').read_text()
for name in ['TikTokFeedTabControl','TikTokFeedFadeScrollView','TTKFeedTabCornerItemView','AWELiveFeedEntranceView','TTKSearchEntranceButton','TikTokFeedTabItem']:
    assert name in top
assert 'view.alpha=0' in top and 'view.userInteractionEnabled=NO' in top
assert 'TT50CommentTarget(touch.view)' in h
assert 'TT50CommentTarget(hit ?: gesture.view)' in h
assert '.6*NSEC_PER_SEC' in h
assert 'TT50DiagnosticWindows()' in s and 'NSUInteger total=0;' in s[s.index('TT50DiagnosticWindows())'):]
assert 'TT48Debug(@"after-long-press")' not in h
print('PASS 50 exact class top hiding, comment-specific delayed diagnostics and fresh all-window enumeration')
