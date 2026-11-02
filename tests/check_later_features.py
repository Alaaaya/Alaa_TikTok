from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
s=(root/'Tweak.x').read_text();p=(root/'PhaseOne.inc').read_text()
seen=set();cls=None
for line in s.splitlines():
 if line.startswith('%hook '):cls=line.split()[1]
 elif line.startswith('%end'):cls=None
 elif cls and re.match(r'^[-+] \(',line):
  selector=''.join(re.findall(r'(\w+)\s*:',line.split('{')[0])) if ':' in line.split('{')[0] else re.search(r'\)\s*(\w+)',line).group(1)
  item=(cls,selector)
  assert item not in seen,('duplicate hook',item)
  seen.add(item)
keys=re.findall(r'key:@"(tt\.extra\.[^"]+)" defaultValue:(\w+) tag:(\d+)',s)
assert len(keys)==len({k for k,d,t in keys})==len({t for k,d,t in keys})
for key,default,tag in keys:
 assert default=='NO'
 assert f'sender.tag == {tag}' in s
 assert f'setBool:sender.on forKey:@"{key}"' in s
if 'tt.extra.autoScroll' in s:
 assert 'TTExtraScrollNext(self)) return;' in s
 assert 'sig.numberOfArguments!=2' in (root/'LaterFeatures.inc').read_text()
 assert 'UIApplicationStateActive' in (root/'LaterFeatures.inc').read_text()
if 'tt.extra.profileImage' in s:
 q=(root/'ProfileTools.inc').read_text()
 assert 'requestAuthorizationForAccessLevel:PHAccessLevelAddOnly' in q
 assert 'NSPhotoLibraryAddUsageDescription' in q
 assert 'creationRequestForAssetFromImage:image' in q
if 'tt.extra.longComments' in s:assert 'MAX(original,(NSUInteger)240)' in s
if 'tt.extra.copyBio' in s:
 q=(root/'ProfileTools.inc').read_text()
 assert 'TTGActionObject(vc,@"user"),@"signature"' in q
 assert 'UIPasteboard.generalPasteboard.string=bio' in q
if 'tt.extra.followsYou' in s:
 q=(root/'ProfileTools.inc').read_text()
 assert 'TT122Number(user,@"followerStatus")==1' in q and 'visibleVideosCount' in q
 assert '%hook TIKTOKProfileHeaderView\n- (void)layoutSubviews' in s
print(f'PASS {len(seen)} unique hooked methods, {len(keys)} off-by-default extra toggles and feature guards')
