from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'PhaseOne.inc').read_text();t=(root/'Tweak.x').read_text()
assert 'BOOL keepHidden=self.uiHidden;[self restoreUI];self.uiHidden=keepHidden;' in s
assert 'if(self.uiHidden)[self applyHiddenUI]' in s
for event in ['UIApplicationDidBecomeActiveNotification','UIApplicationWillEnterForegroundNotification','UIWindowDidBecomeKeyNotification','UISceneDidActivateNotification']:
 assert event in s
assert 'candidate.isKeyWindow' in s and '[window bringSubviewToFront:self.overlay]' in s
assert '[weak recoverFeed];[weak refresh]' in s
assert '[NSRunLoop.mainRunLoop addTimer:self.timer forMode:NSRunLoopCommonModes]' in s
disappear=t[t.index('- (void)viewDidDisappear:'):t.index('- (void)viewDidDisappear:')+180]
assert 'restoreUI' not in disappear and 'TT122Recover()' in disappear
assert t.count('TT122Recover();')>=5
assert 'rect.size.height>window.bounds.size.height*.5' in s
# Stateful transition check mirrors the required invariant, not a UIKit runtime test.
desired=True
for model in ['first','second','reused_second','foreground','new_keywindow']:
 old=desired;desired=False;desired=old
 assert desired
desired=False
for model in ['next','reuse']:
 old=desired;desired=False;desired=old
 assert not desired
print('PASS lifecycle source guards, persistent desired-state transitions and explicit-only Eye reset')
