from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
s=(root/'Tweak.x').read_text();p=(root/'PhaseOne.inc').read_text()
base=subprocess.check_output(['git','show','2af08af:Tweak.x'],cwd=root).decode()
# The entire old renderer, bar, and action-mask implementation is unchanged
# except calls that notify floating tools of the active feed.
prefix=s[:s.index('#include "PhaseOne.inc"')].replace('static void TT122Track(UIViewController *c);\n','').replace(' TT122Track(c);','')
assert prefix.rstrip()==base[:base.index('// A custom, real-material sheet;')].rstrip()
for key in ['download','eye','progress','standard','hd','highest','audio','copy']:
 assert ('tt122.'+key) in s and ('tt122.'+key) in p
assert 'presentViewController' not in p[p.index('- (void)showMenu:'):p.index('- (void)tapped:')]
assert 'setTranslation:CGPointZero' in p and 'setObject:@{@"x":' in p
assert 'PHAssetChangeRequest creationRequestForAssetFromVideoAtFileURL' in p
assert 'NSPhotoLibraryAddUsageDescription' in p
assert 'http://' not in p and 'isEqual:@"https"' in p
assert 'removeItemAtURL' in p
assert 'tag:100+i' in s and 'sender.tag-100' in s
assert 'Version: 1.22' in (root/'control').read_text()
print('PASS 1.21 bar+glyph byte regression, toggles, playback-preserving menus, saved drags, Photos permission guards, HTTPS, cleanup')
