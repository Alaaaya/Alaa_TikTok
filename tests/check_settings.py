from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
s=(root/'Tweak.x').read_text();p=(root/'PhaseOne.inc').read_text();c=(root/'CustomTools.inc').read_text()
assert 'self.pageTitle.text=back?@"Joker"' in s
assert 'Name: Joker\n' in (root/'control').read_text()
assert 'Package: com.alaaaya.tiktokglass\n' in (root/'control').read_text()
assert (root/'Alaa_TikTok.plist').exists()
assert 'TWEAK_NAME = Alaa_TikTok' in (root/'Makefile').read_text()
assert 'brand.text=copy?@"Joker · نسخ":@"Joker · تنزيل"' in p
page=s[s.index('- (void)showCategory:'):s.index('- (void)loadView {',s.index('- (void)showCategory:'))]
home=page[page.index('    if(back) {'):page.index('    } else {')]
assert 'for(NSInteger i=0;i<5;i+=2)' in home and '[self categoryTile:i]' in home
assert 'row.distribution=UIStackViewDistributionFillEqually' in home
assert 'self row:' not in home
assert '[self showCategory:-1 animated:YES]' in s
assert '[back.leftAnchor constraintEqualToAnchor:header.leftAnchor]' in s
assert '[scroll.topAnchor constraintEqualToAnchor:header.bottomAnchor]' in s
assert '[self.stack removeArrangedSubview:header]' in s
assert 'snapshotViewAfterScreenUpdates:NO' in page and 'CGAffineTransformMakeTranslation' in page
assert 'UIAccessibilityIsReduceMotionEnabled()' in page and 'UIAccessibilityScreenChangedNotification' in page
assert '(category==0 && i!=1) || (category==1 && i==1)' in page
for category,tags in [(0,[201,202]),(2,[200,203,204,205]),(3,[206]),(4,[0,1,2,3])]:
 block=page.split(f'    if(category=={category}) {{',1)[1].split('\n    }',1)[0]
 for tag in tags: assert f'tag:{tag}]' in block,(category,tag)
assert s.count('[self iconRow:')==2
assert 'PHPickerViewControllerDelegate' in c and 'PHPickerFilter.imagesFilter' in c
assert 'UIDocumentPickerDelegate' in c and '@[UTTypePNG,UTTypeJPEG]' in c
assert 'startAccessingSecurityScopedResource' in c and 'stopAccessingSecurityScopedResource' in c
assert '20*1024*1024' in c and 'kCGImageSourceThumbnailMaxPixelSize:@256' in c
assert 'NSDocumentDirectory' in c and 'NSDataWritingAtomic' in c
assert 'إعادة الافتراضي' in c and 'removeItemAtPath:path' in c
assert 'TTToolGlyph(1222,self.uiHidden?' in p and 'TTToolGlyph(1221,@"arrow.down")' in p
assert 'requestAuthorization' not in c # PHPicker requires no broad Photo Library access.
assert 'TTToolRefresh();' in c
print('PASS five LTR category tiles, scoped toggles, back/slide/reduced-motion, Joker display-only rename, safe persisted custom tool art')
