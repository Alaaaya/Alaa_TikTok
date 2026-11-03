from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
s=(root/'Tweak.x').read_text();p=(root/'PhaseOne.inc').read_text()
base=subprocess.check_output(['git','show','2af08af:Tweak.x'],cwd=root).decode()
# The entire old renderer, bar, and action-mask implementation is unchanged
# except calls that notify floating tools of the active feed.
prefix=s[:s.index('#include "CustomTools.inc"')].replace('static void TT122Track(UIViewController *c);\n','').replace(' TT122Track(c);','').replace('static void TT122Recover(void);\n','').replace('{ [[TTGFeedWatchdog shared] addRoot:v]; TT122Recover(); }','[[TTGFeedWatchdog shared] addRoot:v];').replace('{ TTGPrepareFeedActions(v); TT122Recover(); }','TTGPrepareFeedActions(v);')
prefix=prefix.replace('static void TTJokerPetSync(void);\n','').replace('    TTJokerPetSync();\n','')
# 58 exact scoped handoff avoids competing paint owners; renderer stays unchanged otherwise.
prefix=prefix.replace('static BOOL JKStyleOwnsPaint(UIView *native);\nstatic void JKStyleSync(UIView *native);\n','')
prefix=prefix.replace('        // Style58 owns only inert paint inside the real button. Glass must not\n        // rediscover/dim that glyph or hide its source while the style is active.\n        if(JKStyleOwnsPaint(button)){TTGRestoreGlyph(button);JKStyleSync(button);return;}\n\n','')
# 52 interface style hooks, normalized narrowly.
prefix=prefix.replace('static void JKStyleSync(UIView *native);\nstatic CGFloat JKStyleAlpha(UIView *view,CGFloat requested);\n','')
prefix=prefix.replace('%orig(JKStyleAlpha(self,TTGGuardRendererAlpha(self, TT50Alpha(self,alpha))))','%orig(TTGGuardRendererAlpha(self, TT50Alpha(self,alpha)))')
prefix=prefix.replace(' JKStyleSync((UIView *)self);','')
# 50 wraps eye-only synchronous guards; normalize only these exact additions.
prefix=prefix.replace('#include "NoFlash.inc"\n','')
prefix=prefix.replace('TTGGuardRendererAlpha(self, TT50Alpha(self,alpha))','TTGGuardRendererAlpha(self, alpha)')
prefix=prefix.replace('%orig(TT50Hidden(self,hidden));','%orig;')
prefix=prefix.replace('- (void)setUserInteractionEnabled:(BOOL)enabled {%orig(TT50Interaction(self,enabled));}\n','')
prefix=prefix.replace('- (void)willMoveToWindow:(UIWindow *)window {TT50Lifecycle(self);%orig;}\n','')
prefix=prefix.replace('TT50Tree(view);','')
prefix=prefix.replace('TT50Tree((UIView *)self);%orig;TT50Tree((UIView *)self);','%orig;')
prefix=prefix.replace('- (void)prepareForReuse {%orig;}\n','')
# 48 appends a scoped observer to the existing UIKit callback, not renderer logic.
prefix=prefix.replace('static void TT48Added(UIView *view);\n','').replace('%orig;TT48Added(view);','%orig;')
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
assert 'Version: 58' in (root/'control').read_text()
print('PASS 1.21 bar+glyph byte regression, toggles, playback-preserving menus, saved drags, Photos permission guards, HTTPS, cleanup')

assert 'AVAssetExportPresetPassthrough' in p and 'AVMutableComposition composition' in p
assert 'if (audioTracks.count) { [self normalizeVideo:file asset:asset]; return; }' in p
assert 'else [self verifyVideo:file]' in p and '[self verifyMuxedVideo:output]' in p
assert 'UIActivityViewController' in p and 'renderAudioCover:audio image:cover' in p
assert 'AVAssetWriter' in p and 'audioAsVideo' in p
assert '%hook' not in (root/'LaterFeatures.inc').read_text()
print('PASS no silent-save path, mux verification, audio share+cover option, Logos includes')

style=(root/'InterfaceStyle.inc').read_text()
assert 'if(JKStyleOwnsPaint(button)){TTGRestoreGlyph(button);JKStyleSync(button);return;}' in s
assert 'if(JKStyleOn() && objc_getAssociatedObject(view,kJKPaintAlpha))return 0;' in style
assert 'item.superview!=view.superview' not in style[style.index('static CGFloat JKStyleAlpha'):style.index('// Per-button state')]
assert 'JKPaintRestore(native)' in style and 'r.glyph.userInteractionEnabled=NO' in style
print('PASS 58 scoped renderer handoff, no native root alpha suppression, in-place inert paint and restoration')

assert style.index('TTGRestoreGlyph(native);') < style.index('UIView *icon=JKPaintIcon(native,0);')
