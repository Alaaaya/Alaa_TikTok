
#import <UIKit/UIKit.h>
#import <Photos/Photos.h>
#import <AVFoundation/AVFoundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#import "../CustomTools.inc"
static NSString *kPrefEnabled=@"enabled",*kPrefFloating=@"floating",*kPrefUnder=@"under",*kPrefLabels=@"labels",*kPrefStyle=@"style";
static NSInteger kGlassTag=777,kSelTag=778;
static NSUserDefaults *D(void){return NSUserDefaults.standardUserDefaults;}
static BOOL PB(NSString *key,BOOL fallback){id v=[D() objectForKey:key];return v?[v boolValue]:fallback;}
static NSInteger TTGStyle(void){return [D() integerForKey:kPrefStyle];}
static void TTGRefreshAll(void){}
static void TTToolRefresh(void){}
#include "../Mascot.inc"
#include "../EmojiBar.inc"
#include "../TopFeed.inc"
#include "../NoFlash.inc"
#include "../SheetEye.inc"
// UIView-based attributed-text fixture for the observed non-UILabel description class.
@interface JKFixtureAttributedView:UIView
@property(nonatomic,copy) NSAttributedString *attributedText;
@property(nonatomic) NSTextAlignment textAlignment;
@end
@implementation JKFixtureAttributedView
- (void)setAttributedText:(NSAttributedString *)text{_attributedText=[text copy];[self setNeedsDisplay];}
- (void)drawRect:(CGRect)r{[_attributedText drawInRect:self.bounds];}
@end
static void TTGRestoreGlyph(UIView *native){} // production renderer is outside the UIKit fixture
#include "../InterfaceStyle.inc"
#include "../HierarchyDump.inc"
@interface TT122Hub:NSObject
+(instancetype)shared;
-(void)refresh;
@end
@implementation TT122Hub
+(instancetype)shared {static TT122Hub *s;if(!s)s=[self new];return s;}
-(void)refresh {}
@end
// Physical LTR containers also resist inherited RTL appearance overrides.
// Scoped to Joker only. Never change TikTok's global language or appearance.
static void TTJokerForceLTR(UIView *view) {
    view.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    if([view isKindOfClass:UILabel.class]) ((UILabel *)view).textAlignment=NSTextAlignmentLeft;
    if([view isKindOfClass:UIButton.class]) {
        ((UIButton *)view).titleLabel.textAlignment=NSTextAlignmentLeft;

    }
    if([view isKindOfClass:UIControl.class])return;
    for(UIView *child in view.subviews) TTJokerForceLTR(child);
}
// Physical layer coordinates avoid host-app UIView frame mirroring swizzles.
// This helper is only used for Joker's manually laid-out children.
static void TTJokerPhysicalFrame(UIView *view,CGRect frame) {
    view.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    view.transform=CGAffineTransformIdentity;
    view.bounds=(CGRect){CGPointZero,frame.size};
    view.layer.bounds=view.bounds;
    view.layer.position=CGPointMake(frame.origin.x+frame.size.width*view.layer.anchorPoint.x,
                                   frame.origin.y+frame.size.height*view.layer.anchorPoint.y);
}
@interface TTJokerSwitch : UISwitch
@end
@implementation TTJokerSwitch
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)value {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UIUserInterfaceLayoutDirection)effectiveUserInterfaceLayoutDirection {return UIUserInterfaceLayoutDirectionLeftToRight;}
- (void)didMoveToWindow {[super didMoveToWindow];TTJokerForceLTR(self);}
- (void)layoutSubviews {[super layoutSubviews];TTJokerForceLTR(self);}
@end
@interface TTJokerRoot : UIView
@end
@implementation TTJokerRoot
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)value {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
- (UIUserInterfaceLayoutDirection)effectiveUserInterfaceLayoutDirection {return UIUserInterfaceLayoutDirectionLeftToRight;}
- (void)layoutSubviews {[super layoutSubviews];TTJokerForceLTR(self);}
@end
@interface TTJokerLabel : UILabel
@end
@implementation TTJokerLabel
- (NSTextAlignment)textAlignment {return NSTextAlignmentLeft;}
- (void)setTextAlignment:(NSTextAlignment)value {[super setTextAlignment:NSTextAlignmentLeft];}
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)value {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
@end
@interface TTJokerRow : TTJokerRoot
@property(nonatomic,weak) UIView *icon,*caption,*toggle;
@end
@implementation TTJokerRow
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat w=self.bounds.size.width,h=self.bounds.size.height;
    CGSize ts=[self.toggle sizeThatFits:CGSizeZero];
    TTJokerPhysicalFrame(self.icon,CGRectMake(16,(h-24)/2,24,24));
    TTJokerPhysicalFrame(self.toggle,CGRectMake(w-14-ts.width,(h-ts.height)/2,ts.width,ts.height));
    TTJokerPhysicalFrame(self.caption,CGRectMake(52,12,MAX(0,w-52-ts.width-26),MAX(0,h-24)));
}
@end

@interface TTJokerTile : UIButton
@property(nonatomic,strong) UIImageView *glyph;
@property(nonatomic,strong) UILabel *caption;
@property(nonatomic,strong) UIView *glass;
@end
@implementation TTJokerTile
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)v {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UIUserInterfaceLayoutDirection)effectiveUserInterfaceLayoutDirection {return UIUserInterfaceLayoutDirectionLeftToRight;}
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat w=self.bounds.size.width;
    TTJokerPhysicalFrame(self.glass,self.bounds);
    TTJokerPhysicalFrame(self.glyph,CGRectMake(10,12,25,25));
    TTJokerPhysicalFrame(self.caption,CGRectMake(8,44,MAX(0,w-16),34));
    self.caption.textAlignment=NSTextAlignmentLeft;
}
@end
@interface TTJokerGrid : UIView
@end
@implementation TTJokerGrid
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)value {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
- (UIUserInterfaceLayoutDirection)effectiveUserInterfaceLayoutDirection {return UIUserInterfaceLayoutDirectionLeftToRight;}
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat gap=8,w=(self.bounds.size.width-gap*2)/3;
    for(NSInteger i=0;i<(NSInteger)self.subviews.count;i++)
        {
        UIView *tile=self.subviews[i];TTJokerPhysicalFrame(tile,CGRectMake((i%3)*(w+gap),(i/3)*94,w,86));
        [tile setNeedsLayout];[tile layoutIfNeeded];
        }
}
@end
@interface TTJokerHeader : UIView
@property(nonatomic,weak) UIView *back,*titleView,*subtitleView,*closeView;
@property(nonatomic) BOOL home;
@end
@implementation TTJokerHeader
- (void)setSemanticContentAttribute:(UISemanticContentAttribute)value {[super setSemanticContentAttribute:UISemanticContentAttributeForceLeftToRight];}
- (UISemanticContentAttribute)semanticContentAttribute {return UISemanticContentAttributeForceLeftToRight;}
- (UIUserInterfaceLayoutDirection)effectiveUserInterfaceLayoutDirection {return UIUserInterfaceLayoutDirectionLeftToRight;}
- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat hw=self.bounds.size.width,tx=self.home?0:36;
    TTJokerPhysicalFrame(self.back,CGRectMake(0,9,32,44));
    TTJokerPhysicalFrame(self.titleView,CGRectMake(tx,5,MAX(0,hw-tx-38),27));
    TTJokerPhysicalFrame(self.subtitleView,CGRectMake(tx,33,MAX(0,hw-tx-38),18));
    TTJokerPhysicalFrame(self.closeView,CGRectMake(hw-32,9,32,44));
}
@end
static UIVisualEffect *TTJokerGlass(void) {
    // Stable dense live backdrop blur; clear native glass was unreadable over video.
    return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
}
// A custom, real-material sheet; no action-sheet emoji or developer-copy action.
@interface TTGSettingsController : UIViewController
@property(nonatomic, weak) UITabBar *bar;
@property(nonatomic, strong) UIVisualEffectView *panel;
@property(nonatomic, strong) UIView *shade;
@property(nonatomic, strong) UIStackView *stack;
@property(nonatomic, strong) NSLayoutConstraint *panelHeight;
@property(nonatomic, strong) NSLayoutConstraint *panelCenter;
@property(nonatomic, strong) UIButton *backButton;
@property(nonatomic, strong) UILabel *pageTitle;
@property(nonatomic, strong) UIView *pageHeader;
@property(nonatomic, strong) UIScrollView *pageScroll;
@property(nonatomic) NSInteger category;
@property(nonatomic) BOOL pageAnimating;
@property(nonatomic,strong) UILabel *subtitle;
@property(nonatomic,strong) UIButton *closeButton;
@property(nonatomic,strong) CAGradientLayer *specular;
@property(nonatomic,strong) CAShapeLayer *edgeMask;
@property(nonatomic,strong) CAGradientLayer *sheen;
@property(nonatomic,strong) UIView *panelShadow;
@end
@implementation TTGSettingsController
- (NSURL *)diagnosticLogURL {
    NSString *folder=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
    if(!folder)return nil;
    NSString *path=[folder stringByAppendingPathComponent:@"joker_debug.txt"];
    @synchronized(NSFileManager.defaultManager) {
        if(![NSFileManager.defaultManager fileExistsAtPath:path]) {
            NSString *notice=@"Joker 50\nNo diagnostics recorded yet. Enable the emoji-bar option and long-press comments, then share again.\n";
            if(![notice writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil])return nil;
        }
    }
    JKAppendCapture(path);
    return [NSURL fileURLWithPath:path];
}
- (void)shareDiagnostics:(UIButton *)sender {
    if(self.presentedViewController)return;
    NSURL *url=[self diagnosticLogURL];
    if(!url) {
        UIAlertController *alert=[UIAlertController alertControllerWithTitle:@"تعذّر فتح السجل" message:@"جرّب مرة ثانية" preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"حسناً" style:UIAlertActionStyleCancel handler:nil]];
        [self presentViewController:alert animated:YES completion:nil];return;
    }
    UIActivityViewController *share=[[UIActivityViewController alloc] initWithActivityItems:@[url] applicationActivities:nil];
    share.popoverPresentationController.sourceView=sender;
    share.popoverPresentationController.sourceRect=sender.bounds;
    [self presentViewController:share animated:YES completion:nil];
}
- (UIButton *)diagnosticsRow {
    UIButton *row=[UIButton buttonWithType:UIButtonTypeSystem];
    row.backgroundColor=[UIColor colorWithWhite:1 alpha:.08];row.layer.cornerRadius=18;
    row.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    row.contentHorizontalAlignment=UIControlContentHorizontalAlignmentLeft;
    [row setTitle:@"  مشاركة سجل التشخيص" forState:UIControlStateNormal];
    [row setImage:[UIImage systemImageNamed:@"square.and.arrow.up"] forState:UIControlStateNormal];
    row.tintColor=UIColor.whiteColor;row.titleLabel.font=[UIFont systemFontOfSize:14 weight:UIFontWeightMedium];
    row.titleLabel.adjustsFontSizeToFitWidth=YES;
    row.accessibilityLabel=@"مشاركة سجل التشخيص";
    [row.heightAnchor constraintEqualToConstant:58].active=YES;
    [row addTarget:self action:@selector(shareDiagnostics:) forControlEvents:UIControlEventTouchUpInside];
    TTJokerForceLTR(row);return row;
}
- (BOOL)shijimaInstalled {
    for(NSString *root in @[@"/var/jb/Applications",@"/Applications"])
        if([NSFileManager.defaultManager fileExistsAtPath:[root stringByAppendingPathComponent:@"ShijimaPrefs.app/Info.plist"]]) return YES;
    return NO;
}
- (void)openShijima {
    NSString *message=@"Shijima غير مثبت (not installed)";
    if([self shijimaInstalled]) {
        // Launch by the bundle identifier verified in the supplied app's Info.plist.
        Class cls=NSClassFromString(@"LSApplicationWorkspace");
        SEL shared=NSSelectorFromString(@"defaultWorkspace"),open=NSSelectorFromString(@"openApplicationWithBundleID:");
        if([cls respondsToSelector:shared]) {
            NSMethodSignature *sig=[cls methodSignatureForSelector:shared];
            if(sig.numberOfArguments==2 && sig.methodReturnType[0]=='@') {
                id workspace=((id(*)(id,SEL))objc_msgSend)(cls,shared);
                NSMethodSignature *launch=[workspace methodSignatureForSelector:open];
                if(launch.numberOfArguments==3 && !strcmp(launch.methodReturnType,@encode(BOOL)) &&
                   [launch getArgumentTypeAtIndex:2][0]=='@' &&
                   ((BOOL(*)(id,SEL,id))objc_msgSend)(workspace,open,@"com.pixelomer.shijimademoprefs")) return;
            }
        }
        message=@"افتح تطبيق Shijima Demo من الشاشة الرئيسية. النظام ما سمح بفتحه من هون.";
    }
    UIAlertController *alert=[UIAlertController alertControllerWithTitle:@"Shijima" message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"حسناً" style:UIAlertActionStyleDefault handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (UILabel *)label:(NSString *)text size:(CGFloat)size weight:(UIFontWeight)weight {
    UILabel *label = [TTJokerLabel new];
    label.text = text; label.font = [UIFont systemFontOfSize:size weight:weight];
    label.textColor = UIColor.whiteColor; label.numberOfLines = 0;
    label.textAlignment = NSTextAlignmentLeft;
    label.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    return label;
}
- (UIView *)row:(NSString *)title symbol:(NSString *)symbol key:(NSString *)key defaultValue:(BOOL)value tag:(NSInteger)tag {
    TTJokerRow *row = [TTJokerRow new];
    row.backgroundColor = [UIColor colorWithWhite:1 alpha:0.045];
    row.layer.cornerRadius = 18; row.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    UIImageView *icon = [[UIImageView alloc] initWithImage:[UIImage systemImageNamed:symbol]];
    icon.tintColor = [UIColor colorWithRed:0.82 green:0.93 blue:1 alpha:1];
    icon.contentMode = UIViewContentModeScaleAspectFit;
    UILabel *label = [self label:title size:15 weight:UIFontWeightMedium];
    UISwitch *toggle = [TTJokerSwitch new]; toggle.on = PB(key, value); toggle.tag = tag;
    toggle.accessibilityLabel = title;
    toggle.onTintColor = [UIColor colorWithRed:0.78 green:0.70 blue:0.51 alpha:1];
    [toggle addTarget:self action:@selector(changed:) forControlEvents:UIControlEventValueChanged];
    row.icon=icon;row.caption=label;row.toggle=toggle;
    for(UIView *view in @[icon,label,toggle]) [row addSubview:view];
    [row.heightAnchor constraintEqualToConstant:78].active=YES;
    TTJokerForceLTR(row);
    return row;
}
- (UIButton *)iconRow:(NSString *)title tag:(NSInteger)tag {
    UIButton *row=[UIButton buttonWithType:UIButtonTypeSystem];row.tag=tag;
    row.backgroundColor=[UIColor colorWithWhite:1 alpha:.08];row.layer.cornerRadius=18;
    row.contentHorizontalAlignment=UIControlContentHorizontalAlignmentLeft;row.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    [row setTitle:[@"  " stringByAppendingString:title] forState:UIControlStateNormal];
    [row setImage:TTToolGlyph(tag,tag==1221?@"arrow.down.circle":@"eye") forState:UIControlStateNormal];
    row.imageView.contentMode=UIViewContentModeScaleAspectFit;row.tintColor=UIColor.whiteColor;
    row.titleLabel.font=[UIFont systemFontOfSize:15 weight:UIFontWeightMedium];
    [row.heightAnchor constraintEqualToConstant:58].active=YES;
    row.accessibilityLabel=title;row.accessibilityHint=@"اختيار صورة أو إعادة الافتراضي";
    [row addTarget:self action:@selector(changeToolIcon:) forControlEvents:UIControlEventTouchUpInside];return row;
}
- (void)changeToolIcon:(UIButton *)sender {
    [[TTToolPicker shared] choose:sender.tag presenter:self source:sender];
}
- (UIButton *)categoryTile:(NSInteger)index {
    NSArray *names=@[@"التنزيل",@"الخصوصية",@"الخلاصة",@"الملف الشخصي",@"المظهر",@"Shijima",@"شخصية Joker"];
    NSArray *symbols=@[@"arrow.down.circle",@"hand.raised",@"play.rectangle",@"person.crop.square",@"slider.horizontal.3",@"pawprint",@"cat"];
    TTJokerTile *tile=[TTJokerTile buttonWithType:UIButtonTypeCustom];tile.tag=index;
    tile.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
    tile.backgroundColor=[UIColor colorWithWhite:1 alpha:.10];tile.layer.cornerRadius=18;
    tile.layer.cornerCurve=kCACornerCurveContinuous;tile.layer.borderWidth=1.0/UIScreen.mainScreen.scale;
    tile.layer.borderColor=[UIColor colorWithWhite:1 alpha:.48].CGColor;tile.clipsToBounds=YES;
    UIView *glass=[UIView new];glass.backgroundColor=[UIColor colorWithWhite:1 alpha:.045];
    glass.userInteractionEnabled=NO;glass.translatesAutoresizingMaskIntoConstraints=YES;[tile addSubview:glass];tile.glass=glass;
    UIImageView *icon=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:symbols[index]]];
    icon.tintColor=[UIColor colorWithRed:.86 green:.91 blue:1 alpha:1];icon.contentMode=UIViewContentModeScaleAspectFit;
    UILabel *label=[self label:names[index] size:11 weight:UIFontWeightSemibold];
    label.textAlignment=NSTextAlignmentCenter;label.numberOfLines=2;
    for(UIView *view in @[icon,label]){view.userInteractionEnabled=NO;view.translatesAutoresizingMaskIntoConstraints=YES;[tile addSubview:view];}
    tile.glyph=icon;tile.caption=label;
    [tile.heightAnchor constraintGreaterThanOrEqualToConstant:86].active=YES;
    // A restrained two-tone SF Symbol treatment preserves crisp geometry.
    NSArray *palette=@[[UIColor colorWithRed:.65 green:.88 blue:1 alpha:1],
                      [UIColor colorWithRed:1 green:.86 blue:.60 alpha:1]];
    UIImageSymbolConfiguration *color=[UIImageSymbolConfiguration configurationWithPaletteColors:palette];
    icon.image=[[UIImage systemImageNamed:symbols[index]] imageByApplyingSymbolConfiguration:color];
    icon.layer.shadowColor=UIColor.whiteColor.CGColor;icon.layer.shadowOpacity=.18;
    icon.layer.shadowRadius=6;icon.layer.shadowOffset=CGSizeZero;
    tile.accessibilityLabel=names[index];tile.accessibilityHint=@"فتح إعدادات القسم";
    tile.isAccessibilityElement=YES;
    [tile addTarget:self action:@selector(openCategory:) forControlEvents:UIControlEventTouchUpInside];
    [tile addTarget:self action:@selector(pressTile:) forControlEvents:UIControlEventTouchDown];
    [tile addTarget:self action:@selector(releaseTile:) forControlEvents:UIControlEventTouchUpInside|UIControlEventTouchUpOutside|UIControlEventTouchCancel];
    return tile;
}
- (void)pressTile:(UIButton *)tile {
    [UIView animateWithDuration:.12 animations:^{tile.alpha=.76;if(!UIAccessibilityIsReduceMotionEnabled())tile.transform=CGAffineTransformMakeScale(.93,.93);}];
}
- (void)releaseTile:(UIButton *)tile {
    [UIView animateWithDuration:.4 delay:0 usingSpringWithDamping:.64 initialSpringVelocity:.4 options:UIViewAnimationOptionBeginFromCurrentState animations:^{tile.alpha=1;tile.transform=CGAffineTransformIdentity;} completion:nil];
}
- (void)openCategory:(UIButton *)tile { [self showCategory:tile.tag animated:YES]; }
- (void)goBack { [self showCategory:-1 animated:YES]; }
- (void)showCategory:(NSInteger)category animated:(BOOL)animated {
    if(self.pageAnimating || category < -1 || category > 6)return;
    BOOL back=category==-1;
    UIView *snapshot=animated?[self.pageScroll snapshotViewAfterScreenUpdates:NO]:nil;
    if(snapshot){snapshot.frame=[self.pageScroll convertRect:self.pageScroll.bounds toView:self.panel.contentView];[self.panel.contentView addSubview:snapshot];}
    for(UIView *view in self.stack.arrangedSubviews.copy) {
        if(view==self.pageHeader)continue;
        [self.stack removeArrangedSubview:view];[view removeFromSuperview];
    }
    self.category=category;self.backButton.hidden=back;
    NSArray *names=@[@"التنزيل",@"الخصوصية",@"الخلاصة",@"الملف الشخصي",@"المظهر",@"Shijima",@"شخصية Joker"];
    self.pageTitle.text=back?@"Joker":names[category];
    if(back) {
        TTJokerGrid *grid=[TTJokerGrid new];
        grid.semanticContentAttribute=UISemanticContentAttributeForceLeftToRight;
        [grid.heightAnchor constraintEqualToConstant:([self shijimaInstalled]?274:180)].active=YES;
        for(NSInteger i=0;i<7;i++){if(i==5 && ![self shijimaInstalled])continue;[grid addSubview:[self categoryTile:i]];}
        [self.stack addArrangedSubview:grid];
        [self.stack addArrangedSubview:[self diagnosticsRow]];
    } else {
    NSArray *featureTitles = @[@"زر التنزيل العائم",@"زر إخفاء العناصر",@"حلقة تقدم التنزيل",@"تنزيل عادي",@"تنزيل HD",@"أعلى جودة متاحة",@"تنزيل الصوت (ملفات)",@"قائمة النسخ"];
    NSArray *featureSymbols = @[@"arrow.down.circle",@"eye",@"circle.dotted",@"arrow.down",@"h.circle",@"sparkles",@"waveform",@"doc.on.doc"];
    NSArray *featureKeys = @[@"tt122.download",@"tt122.eye",@"tt122.progress",@"tt122.standard",@"tt122.hd",@"tt122.highest",@"tt122.audio",@"tt122.copy"];
    for (NSInteger i=0;i<(NSInteger)featureKeys.count;i++)
        if ((category==0 && i!=1) || (category==1 && i==1))
        [self.stack addArrangedSubview:[self row:featureTitles[i] symbol:featureSymbols[i] key:featureKeys[i] defaultValue:YES tag:100+i]];

    if(category==0) {
    [self.stack addArrangedSubview:[self row:@"إظهار بشريط الإيموجي" symbol:@"face.smiling" key:@"tt46.emojiBar" defaultValue:NO tag:246]];
    [self.stack addArrangedSubview:[self row:@"استخدام رابط التشغيل (قد يكون بلا علامة)" symbol:@"play.rectangle" key:@"tt.extra.playbackAsset" defaultValue:NO tag:201]];
    [self.stack addArrangedSubview:[self row:@"حفظ القصة بالضغط المطوّل" symbol:@"arrow.down.circle" key:@"tt.extra.storySave" defaultValue:NO tag:202]];
    }
    if(category==1) {

    }
    if(category==2) {
    [self.stack addArrangedSubview:[self row:@"تعليقات أطول (الحد يقرّره الخادم)" symbol:@"text.bubble" key:@"tt.extra.longComments" defaultValue:NO tag:207]];
    [self.stack addArrangedSubview:[self row:@"إخفاء إعلانات الخلاصة" symbol:@"eye.slash" key:@"tt.extra.noads" defaultValue:NO tag:200]];
    [self.stack addArrangedSubview:[self row:@"الانتقال تلقائياً بعد نهاية الفيديو" symbol:@"arrow.down.to.line" key:@"tt.extra.autoScroll" defaultValue:NO tag:203]];
    [self.stack addArrangedSubview:[self row:@"إظهار شريط تقدم الفيديو دائماً" symbol:@"slider.horizontal.3" key:@"tt.extra.progress" defaultValue:NO tag:204]];
    [self.stack addArrangedSubview:[self row:@"إخفاء ملصقات تحذير الخلاصة" symbol:@"eye.slash" key:@"tt.extra.feedWarnings" defaultValue:NO tag:205]];
    }
    if(category==3) {
    [self.stack addArrangedSubview:[self row:@"حفظ صورة الحساب بالضغط المطوّل" symbol:@"person.crop.square" key:@"tt.extra.profileImage" defaultValue:NO tag:206]];
    [self.stack addArrangedSubview:[self row:@"نسخ النبذة بالضغط المطوّل على الملف" symbol:@"doc.on.doc" key:@"tt.extra.copyBio" defaultValue:NO tag:208]];
    [self.stack addArrangedSubview:[self row:@"شارة يتابعك على الملف" symbol:@"person.badge.plus" key:@"tt.extra.followsYou" defaultValue:NO tag:209]];
    [self.stack addArrangedSubview:[self row:@"عدد الفيديوهات على الملف" symbol:@"rectangle.stack" key:@"tt.extra.videoCount" defaultValue:NO tag:210]];
    }
    if(category==6) {
        for(NSInteger i=0;i<2;i++){
            UIButton *b=[UIButton buttonWithType:UIButtonTypeSystem];
            [b setTitle:i==0?@"استيراد ZIP من الملفات":@"استيراد مجلد من الملفات" forState:UIControlStateNormal];
            b.tag=i;[b.heightAnchor constraintEqualToConstant:46].active=YES;
            [b addTarget:self action:@selector(importPetPack:) forControlEvents:UIControlEventTouchUpInside];
            [self.stack addArrangedSubview:b];
        }
        UIButton *choose=[UIButton buttonWithType:UIButtonTypeSystem];
        NSString *active=[D() stringForKey:@"tt.pet.pack"];
        [choose setTitle:active.length?[NSString stringWithFormat:@"الشخصية · %@",active]:@"الشخصية · Joker الأصلية" forState:UIControlStateNormal];
        choose.titleLabel.lineBreakMode=NSLineBreakByTruncatingMiddle;
        [choose.heightAnchor constraintEqualToConstant:46].active=YES;
        [choose addTarget:self action:@selector(choosePetPack:) forControlEvents:UIControlEventTouchUpInside];
        [self.stack addArrangedSubview:choose];
        [self.stack addArrangedSubview:[self row:@"شخصية Joker" symbol:@"cat" key:@"tt.pet.enabled" defaultValue:NO tag:360]];
        UILabel *label=[self label:@"الحجم" size:14 weight:UIFontWeightSemibold];[self.stack addArrangedSubview:label];
        UISlider *size=[UISlider new];size.minimumValue=28;size.maximumValue=76;
        size.value=[D() objectForKey:@"tt.pet.size"]?[D() doubleForKey:@"tt.pet.size"]:44;
        size.accessibilityLabel=@"حجم الشخصية";
        [size addTarget:self action:@selector(petSize:) forControlEvents:UIControlEventValueChanged];
        [self.stack addArrangedSubview:size];
        UISegmentedControl *kind=[[UISegmentedControl alloc] initWithItems:@[@"قط",@"Joker"]];
        kind.selectedSegmentIndex=[D() integerForKey:@"tt.pet.kind"]==1?1:0;
        [kind addTarget:self action:@selector(petKind:) forControlEvents:UIControlEventValueChanged];
        [self.stack addArrangedSubview:kind];
        [self.stack addArrangedSubview:[self label:@"اسحب الشخصية وارميها. بتمشي وبتقعد فوق الشريط. مستقلة تماماً عن Shijima." size:13 weight:UIFontWeightRegular]];
    }
    if(category==5) {
        BOOL installed=[self shijimaInstalled];
        UILabel *status=[self label:installed?@"Shijima Demo مثبت":@"Shijima غير مثبت (not installed)" size:15 weight:UIFontWeightSemibold];
        [self.stack addArrangedSubview:status];
        UIView *row=[self row:@"تشغيل Shijima (Demo)" symbol:@"pawprint" key:@"tt.shijima.status" defaultValue:installed tag:350];
        for(UIView *v in row.subviews)if([v isKindOfClass:UISwitch.class]){
            UISwitch *sw=(UISwitch *)v;[sw removeTarget:nil action:NULL forControlEvents:UIControlEventAllEvents];
            sw.on=installed;sw.enabled=NO;sw.accessibilityHint=@"النسخة التجريبية لا تدعم الإطفاء";
        }
        [self.stack addArrangedSubview:row];
        UIButton *open=[UIButton buttonWithType:UIButtonTypeSystem];
        [open setTitle:@"فتح إعدادات Shijima" forState:UIControlStateNormal];
        open.tintColor=UIColor.whiteColor;open.backgroundColor=[UIColor colorWithWhite:1 alpha:.10];open.layer.cornerRadius=16;
        [open.heightAnchor constraintGreaterThanOrEqualToConstant:52].active=YES;
        [open addTarget:self action:@selector(openShijima) forControlEvents:UIControlEventTouchUpInside];
        [self.stack addArrangedSubview:open];
        UILabel *note=[self label:@"نسخة Demo ما بتسمح بالإطفاء. تغيير إعدادات Shijima من تطبيقها فقط. بتحتاج تثبيت الحزمتين بجيلبريك متوافق، مو حقن Joker لحاله." size:13 weight:UIFontWeightRegular];
        [self.stack addArrangedSubview:note];
    }
    if(category==4) {
        UIButton *mascot=[UIButton buttonWithType:UIButtonTypeSystem];
        [mascot setTitle:@"الشخصية · Mascot" forState:UIControlStateNormal];mascot.tag=6;
        [mascot.heightAnchor constraintEqualToConstant:46].active=YES;
        [mascot addTarget:self action:@selector(openCategory:) forControlEvents:UIControlEventTouchUpInside];
        [self.stack addArrangedSubview:mascot];
        [self.stack addArrangedSubview:[self iconRow:@"صورة زر التنزيل" tag:1221]];
        [self.stack addArrangedSubview:[self iconRow:@"صورة زر العين" tag:1222]];
        JKAddStylePicker(self.stack);
    [self.stack addArrangedSubview:[self row:@"الشريط الزجاجي" symbol:@"rectangle.bottomthird.inset.filled" key:kPrefEnabled defaultValue:YES tag:0]];
    [self.stack addArrangedSubview:[self row:@"كبسولة عائمة" symbol:@"capsule" key:kPrefFloating defaultValue:YES tag:1]];
    [self.stack addArrangedSubview:[self row:@"الفيديو تحت الشريط" symbol:@"rectangle.expand.vertical" key:kPrefUnder defaultValue:YES tag:2]];
    [self.stack addArrangedSubview:[self row:@"إخفاء أسماء الأزرار" symbol:@"textformat" key:kPrefLabels defaultValue:YES tag:3]];
    UILabel *styleLabel = [self label:@"نمط الشريط" size:13 weight:UIFontWeightSemibold];
    styleLabel.textColor = [UIColor colorWithWhite:1 alpha:0.88];
    [self.stack addArrangedSubview:styleLabel];
    UISegmentedControl *styles = [[UISegmentedControl alloc] initWithItems:@[@"زجاج", @"داكن", @"فاتح"]];
    styles.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    styles.selectedSegmentIndex = MAX(0, MIN(2, TTGStyle()));
    styles.accessibilityLabel = @"نمط الشريط";
    [styles.heightAnchor constraintEqualToConstant:42].active = YES;
    [styles addTarget:self action:@selector(styleChanged:) forControlEvents:UIControlEventValueChanged];
    styles.backgroundColor = [UIColor colorWithWhite:1 alpha:0.08];
    styles.selectedSegmentTintColor = [UIColor colorWithRed:0.80 green:0.74 blue:0.60 alpha:0.32];
    [styles setTitleTextAttributes:@{NSForegroundColorAttributeName:UIColor.whiteColor,
        NSFontAttributeName:[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold]} forState:UIControlStateNormal];
    styles.layer.cornerRadius = 12; styles.clipsToBounds = YES;
    styles.apportionsSegmentWidthsByContent = NO;
    [self.stack addArrangedSubview:styles];

    }

        if(category==1) {
            UILabel *note=[self label:@"زر العين بيخفي عناصر الشاشة. خيارات إخفاء القراءة والأونلاين مو متاحة بهالنسخة." size:13 weight:UIFontWeightRegular];
            note.textColor=[UIColor colorWithWhite:1 alpha:.74];[self.stack addArrangedSubview:note];
        }
        if(category==4) {

    UILabel *note = [self label:@"الإطفاء الكامل بيتطبّق بعد ما تسكّر TikTok وترجع تفتحه." size:12 weight:UIFontWeightRegular];
    note.textColor = [UIColor colorWithWhite:1 alpha:0.80];
    UIView *footer = [UIView new]; footer.backgroundColor = [UIColor colorWithWhite:0 alpha:0.15];
    footer.layer.cornerRadius = 14; note.translatesAutoresizingMaskIntoConstraints = NO;
    [footer addSubview:note];
    [NSLayoutConstraint activateConstraints:@[
        [note.leftAnchor constraintEqualToAnchor:footer.leftAnchor constant:12],
        [note.rightAnchor constraintEqualToAnchor:footer.rightAnchor constant:-12],
        [note.topAnchor constraintEqualToAnchor:footer.topAnchor constant:14],
        [note.bottomAnchor constraintEqualToAnchor:footer.bottomAnchor constant:-14]
    ]];

    [self.stack addArrangedSubview:footer];

        }
    }
    [self.pageScroll setContentOffset:CGPointZero animated:NO];
    [self.view setNeedsLayout];[self.view layoutIfNeeded];
    [self forceLTR:self.view];
    if(!snapshot){UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification,self.pageTitle);return;}
    self.pageAnimating=YES;
    CGFloat offset=UIAccessibilityIsReduceMotionEnabled()?0:MIN(90,self.panel.bounds.size.width*.25);
    self.pageScroll.transform=CGAffineTransformMakeTranslation(back?-offset:offset,0);self.pageScroll.alpha=0;
    [UIView animateWithDuration:UIAccessibilityIsReduceMotionEnabled()?.12:.28 delay:0
        options:UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionBeginFromCurrentState animations:^{
            self.pageScroll.transform=CGAffineTransformIdentity;self.pageScroll.alpha=1;
            snapshot.transform=CGAffineTransformMakeTranslation(back?offset:-offset,0);snapshot.alpha=0;
        } completion:^(BOOL done){[snapshot removeFromSuperview];self.pageAnimating=NO;
            UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification,self.pageTitle);}];
}
- (void)loadView {
    self.view = [TTJokerRoot new]; self.view.backgroundColor = UIColor.clearColor;
    self.view.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    self.view.accessibilityViewIsModal = YES;
    self.overrideUserInterfaceStyle = UIUserInterfaceStyleDark;
    UIButton *shade = [UIButton new]; shade.backgroundColor = [UIColor colorWithWhite:0 alpha:0.10];
    [shade addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    shade.accessibilityLabel = @"إغلاق الإعدادات";
    self.shade = shade; shade.frame = self.view.bounds;
    shade.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:shade];
    self.panelShadow=[UIView new];self.panelShadow.userInteractionEnabled=NO;
    self.panelShadow.layer.shadowColor=UIColor.blackColor.CGColor;
    self.panelShadow.layer.shadowOpacity=.45;self.panelShadow.layer.shadowRadius=22;
    self.panelShadow.layer.shadowOffset=CGSizeMake(0,10);
    [self.view addSubview:self.panelShadow];
    self.panel = [[UIVisualEffectView alloc] initWithEffect:TTJokerGlass()];
    self.panel.translatesAutoresizingMaskIntoConstraints = NO;
    self.panel.layer.cornerRadius = 32;
    self.panel.layer.cornerCurve = kCACornerCurveContinuous;
    self.panel.clipsToBounds = YES;
    self.panel.layer.borderWidth = 0;
    self.panel.layer.borderColor = [UIColor colorWithWhite:1 alpha:0.65].CGColor;
    self.panel.contentView.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    self.panel.contentView.backgroundColor = [UIColor colorWithWhite:0 alpha:0.40];
    self.panel.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [self.view addSubview:self.panel];
    self.sheen=[CAGradientLayer layer];
    self.sheen.colors=@[(id)[UIColor colorWithWhite:1 alpha:.20].CGColor,
                        (id)[UIColor colorWithWhite:1 alpha:0].CGColor,
                        (id)[UIColor colorWithRed:.5 green:.7 blue:1 alpha:.045].CGColor];
    self.sheen.locations=@[@0,@.22,@1];
    self.sheen.startPoint=CGPointMake(.5,0);self.sheen.endPoint=CGPointMake(.5,1);
    [self.panel.contentView.layer insertSublayer:self.sheen atIndex:0];
    self.specular=[CAGradientLayer layer];
    self.specular.colors=@[(id)[UIColor colorWithWhite:1 alpha:.9].CGColor,
                           (id)[UIColor colorWithWhite:1 alpha:.28].CGColor,
                           (id)[UIColor colorWithWhite:1 alpha:.70].CGColor];
    self.specular.startPoint=CGPointZero;self.specular.endPoint=CGPointMake(1,1);
    self.edgeMask=[CAShapeLayer layer];self.edgeMask.fillColor=UIColor.clearColor.CGColor;
    self.edgeMask.strokeColor=UIColor.whiteColor.CGColor;self.edgeMask.lineWidth=1.0/UIScreen.mainScreen.scale;
    self.specular.mask=self.edgeMask;[self.panel.layer addSublayer:self.specular];
    UIScrollView *scroll = [UIScrollView new]; scroll.translatesAutoresizingMaskIntoConstraints = NO;
    self.pageScroll=scroll;
    scroll.alwaysBounceVertical = NO; scroll.showsVerticalScrollIndicator = NO;
    scroll.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [self.panel.contentView addSubview:scroll];
    self.stack = [UIStackView new]; self.stack.translatesAutoresizingMaskIntoConstraints = NO;
    self.stack.axis = UILayoutConstraintAxisVertical; self.stack.spacing = 8;
    self.stack.semanticContentAttribute = UISemanticContentAttributeForceLeftToRight;
    [scroll addSubview:self.stack];
    UIView *header = [TTJokerHeader new];
    UIImage *app = nil;
    NSDictionary *icons = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleIcons"];
    NSString *filename = [icons[@"CFBundlePrimaryIcon"][@"CFBundleIconFiles"] lastObject];
    if ([filename isKindOfClass:NSString.class]) app = [UIImage imageNamed:filename];
    UIImageView *image = [[UIImageView alloc] initWithImage:app ?: [UIImage systemImageNamed:@"play.square.stack.fill"]];
    image.tintColor = UIColor.whiteColor; image.layer.cornerRadius = 15; image.clipsToBounds = YES;
    image.contentMode = UIViewContentModeScaleAspectFit;
    UILabel *title = [self label:@"Joker" size:21 weight:UIFontWeightBold];
    self.pageTitle=title;
    title.textAlignment = NSTextAlignmentLeft;title.numberOfLines=1;title.adjustsFontSizeToFitWidth=YES;
    UILabel *subtitle = [self label:@"إعدادات الزجاج" size:13 weight:UIFontWeightMedium];
    subtitle.textAlignment = NSTextAlignmentLeft; subtitle.textColor = [UIColor colorWithWhite:1 alpha:0.86];
    UIButton *close = [UIButton buttonWithType:UIButtonTypeSystem];
    [close setImage:[UIImage systemImageNamed:@"xmark.circle.fill"] forState:UIControlStateNormal];
    close.tintColor = [UIColor colorWithWhite:1 alpha:0.92]; close.accessibilityLabel = @"إغلاق";
    [close addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    UIButton *back=[UIButton buttonWithType:UIButtonTypeSystem];self.backButton=back;
    [back setImage:[UIImage systemImageNamed:@"chevron.left"] forState:UIControlStateNormal];
    back.tintColor=UIColor.whiteColor;back.accessibilityLabel=@"رجوع";back.hidden=YES;
    [back addTarget:self action:@selector(goBack) forControlEvents:UIControlEventTouchUpInside];
    image.hidden=YES;
    for (UIView *view in @[image,title,subtitle,close,back]) { view.translatesAutoresizingMaskIntoConstraints=YES; [header addSubview:view]; }
    TTJokerHeader *jh=(TTJokerHeader *)header;jh.back=back;jh.titleView=title;jh.subtitleView=subtitle;jh.closeView=close;jh.home=YES;
    [header.heightAnchor constraintEqualToConstant:62].active=YES;
    self.subtitle=subtitle;self.closeButton=close;
    [self.stack addArrangedSubview:header];
    self.pageHeader=header;
    [self.stack removeArrangedSubview:header];[header removeFromSuperview];
    header.translatesAutoresizingMaskIntoConstraints=NO;[self.panel.contentView addSubview:header];
    [NSLayoutConstraint activateConstraints:@[
        [header.leftAnchor constraintEqualToAnchor:self.panel.contentView.leftAnchor constant:14],
        [header.rightAnchor constraintEqualToAnchor:self.panel.contentView.rightAnchor constant:-14],
        [header.topAnchor constraintEqualToAnchor:self.panel.contentView.topAnchor constant:10]]];
    [self showCategory:-1 animated:NO];
    UILayoutGuide *safe = self.view.safeAreaLayoutGuide;
    NSLayoutConstraint *height = [self.panel.heightAnchor constraintEqualToConstant:550];
    height.priority = UILayoutPriorityDefaultHigh;
    self.panelHeight = height;
    NSLayoutConstraint *width = [self.panel.widthAnchor constraintEqualToAnchor:safe.widthAnchor multiplier:.70];
    width.priority = UILayoutPriorityDefaultHigh;
    self.panelCenter = [self.panel.centerYAnchor constraintEqualToAnchor:self.view.topAnchor constant:300];
    self.panelCenter.priority = UILayoutPriorityDefaultHigh;
    [NSLayoutConstraint activateConstraints:@[
        self.panelCenter,
        [self.panel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [self.panel.widthAnchor constraintLessThanOrEqualToConstant:390],
        [self.panel.leftAnchor constraintGreaterThanOrEqualToAnchor:safe.leftAnchor constant:14],
        [self.panel.rightAnchor constraintLessThanOrEqualToAnchor:safe.rightAnchor constant:-14],
        width,
        [self.panel.bottomAnchor constraintLessThanOrEqualToAnchor:safe.bottomAnchor constant:-92],
        [self.panel.topAnchor constraintGreaterThanOrEqualToAnchor:safe.topAnchor constant:12],
        height,
        [scroll.leftAnchor constraintEqualToAnchor:self.panel.contentView.leftAnchor],
        [scroll.rightAnchor constraintEqualToAnchor:self.panel.contentView.rightAnchor],
        [scroll.topAnchor constraintEqualToAnchor:header.bottomAnchor],
        [scroll.bottomAnchor constraintEqualToAnchor:self.panel.contentView.bottomAnchor],
        [self.stack.leftAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.leftAnchor constant:12],
        [self.stack.rightAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.rightAnchor constant:-12],
        [self.stack.topAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.topAnchor constant:10],
        [self.stack.bottomAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.bottomAnchor constant:-22],
        [self.stack.widthAnchor constraintEqualToAnchor:scroll.frameLayoutGuide.widthAnchor constant:-24]
    ]];

}
- (void)forceLTR:(UIView *)view {
    TTJokerForceLTR(view);
}
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    if(!self.panelHeight)return;
    [self forceLTR:self.view];
    ((TTJokerHeader *)self.pageHeader).home=self.category==-1;[self.pageHeader setNeedsLayout];[self.pageHeader layoutIfNeeded];
    self.pageTitle.textAlignment=NSTextAlignmentLeft;self.subtitle.textAlignment=NSTextAlignmentLeft;
    self.sheen.frame=self.panel.bounds;self.specular.frame=self.panel.bounds;self.edgeMask.frame=self.panel.bounds;
    CGFloat inset=self.edgeMask.lineWidth/2;
    self.edgeMask.path=[UIBezierPath bezierPathWithRoundedRect:CGRectInset(self.panel.bounds,inset,inset) cornerRadius:32-inset].CGPath;
    self.panelShadow.frame=self.panel.frame;
    self.panelShadow.layer.shadowPath=[UIBezierPath bezierPathWithRoundedRect:self.panelShadow.bounds cornerRadius:32].CGPath;
    CGFloat width = MAX(200, self.panel.bounds.size.width - 24);
    CGFloat natural = [self.stack systemLayoutSizeFittingSize:CGSizeMake(width, UILayoutFittingCompressedSize.height)
        withHorizontalFittingPriority:UILayoutPriorityRequired verticalFittingPriority:UILayoutPriorityFittingSizeLevel].height + 94;
    CGFloat top = self.view.safeAreaInsets.top + 12;
    CGFloat bottom = self.view.bounds.size.height - self.view.safeAreaInsets.bottom - 92;
    UIView *glass = [self.bar viewWithTag:kGlassTag];
    if (glass.window && glass.bounds.size.height > 0) {
        CGRect barFrame = [glass convertRect:glass.bounds toView:self.view];
        if (barFrame.origin.y > top + 150) bottom = MIN(bottom, barFrame.origin.y - 16);
    }
    CGFloat available = MAX(120, bottom - top);
    CGFloat desired = MIN(MAX(180, natural), available);
    if (fabs(self.panelHeight.constant - desired) > 0.5) self.panelHeight.constant = desired;
    CGFloat center = top + available / 2;
    if (fabs(self.panelCenter.constant - center) > 0.5) self.panelCenter.constant = center;
}
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    self.panel.transform = CGAffineTransformMakeTranslation(0, 45);
    self.panel.alpha = 0; self.shade.alpha = 0;
    [UIView animateWithDuration:UIAccessibilityIsReduceMotionEnabled() ? 0.15 : 0.55 delay:0
        usingSpringWithDamping:0.84 initialSpringVelocity:0.3 options:UIViewAnimationOptionBeginFromCurrentState
        animations:^{ self.panel.transform = CGAffineTransformIdentity; self.panel.alpha = 1; self.shade.alpha = 1; } completion:nil];
}
- (void)importPetPack:(UIButton *)sender {
    __weak TTGSettingsController *weakSelf=self;
    [[TTPackPicker shared] choose:self folder:sender.tag==1 completion:^(NSString *pack){
        [D() setObject:pack forKey:@"tt.pet.pack"];[D() setBool:YES forKey:@"tt.pet.enabled"];
        [[TTJokerPet shared] refresh];[weakSelf showCategory:6 animated:NO];
    }];
}
- (void)choosePetPack:(UIButton *)sender {
    UIAlertController *menu=[UIAlertController alertControllerWithTitle:@"الشخصية" message:@"صور Shimeji فقط. ملفات XML لا يتم تشغيلها." preferredStyle:UIAlertControllerStyleActionSheet];
    __weak TTGSettingsController *weakSelf=self;
    NSMutableArray *packs=[NSMutableArray arrayWithObject:@""];[packs addObjectsFromArray:TTPackIDs()];
    for(NSString *pack in packs)[menu addAction:[UIAlertAction actionWithTitle:pack.length?pack:@"Joker الأصلية" style:UIAlertActionStyleDefault handler:^(UIAlertAction *a){
        [D() setObject:pack forKey:@"tt.pet.pack"];[[TTJokerPet shared] refresh];[weakSelf showCategory:6 animated:NO];
    }]];
    [menu addAction:[UIAlertAction actionWithTitle:@"إلغاء" style:UIAlertActionStyleCancel handler:nil]];
    menu.popoverPresentationController.sourceView=sender;menu.popoverPresentationController.sourceRect=sender.bounds;
    [self presentViewController:menu animated:YES completion:nil];
}
- (void)petSize:(UISlider *)sender {[D() setDouble:sender.value forKey:@"tt.pet.size"];[[TTJokerPet shared] refresh];}
- (void)petKind:(UISegmentedControl *)sender {[D() setInteger:sender.selectedSegmentIndex forKey:@"tt.pet.kind"];[[TTJokerPet shared] refresh];}
- (void)changed:(UISwitch *)sender {
    if(sender.tag==246){[D() setBool:sender.on forKey:@"tt46.emojiBar"];[[TT122Hub shared] refresh];return;}
    if(sender.tag==360){[D() setBool:sender.on forKey:@"tt.pet.enabled"];[[TTJokerPet shared] refresh];return;}

    if (sender.tag == 210) { [D() setBool:sender.on forKey:@"tt.extra.videoCount"]; return; }
    if (sender.tag == 209) { [D() setBool:sender.on forKey:@"tt.extra.followsYou"]; return; }
    if (sender.tag == 208) { [D() setBool:sender.on forKey:@"tt.extra.copyBio"]; return; }
    if (sender.tag == 207) { [D() setBool:sender.on forKey:@"tt.extra.longComments"]; return; }
    if (sender.tag == 206) { [D() setBool:sender.on forKey:@"tt.extra.profileImage"]; return; }
    if (sender.tag == 205) { [D() setBool:sender.on forKey:@"tt.extra.feedWarnings"]; return; }
    if (sender.tag == 204) { [D() setBool:sender.on forKey:@"tt.extra.progress"]; return; }
    if (sender.tag == 203) { [D() setBool:sender.on forKey:@"tt.extra.autoScroll"]; return; }
    if (sender.tag == 202) { [D() setBool:sender.on forKey:@"tt.extra.storySave"]; return; }
    if (sender.tag == 201) { [D() setBool:sender.on forKey:@"tt.extra.playbackAsset"]; return; }
    if (sender.tag == 200) { [D() setBool:sender.on forKey:@"tt.extra.noads"]; return; }
    if (sender.tag >= 100 && sender.tag < 108) {
        NSArray *features = @[@"tt122.download",@"tt122.eye",@"tt122.progress",@"tt122.standard",@"tt122.hd",@"tt122.highest",@"tt122.audio",@"tt122.copy"];
        [D() setBool:sender.on forKey:features[sender.tag-100]];
        [[TT122Hub shared] refresh]; return;
    }
    NSArray *keys = @[kPrefEnabled,kPrefFloating,kPrefUnder,kPrefLabels];
    if (sender.tag < 0 || sender.tag >= (NSInteger)keys.count) return;
    [D() setBool:sender.on forKey:keys[sender.tag]];
    TTGRefreshAll();
}
- (void)styleChanged:(UISegmentedControl *)sender {
    [D() setInteger:sender.selectedSegmentIndex forKey:kPrefStyle];
    [[self.bar viewWithTag:kGlassTag] removeFromSuperview];
    [[self.bar viewWithTag:kSelTag] removeFromSuperview];
    TTGRefreshAll();
}
- (void)close {
    [TTJokerPet shared].covered=NO;[[TTJokerPet shared] refresh];
    [UIView animateWithDuration:0.18 animations:^{ self.panel.alpha = 0; self.shade.alpha = 0;
        self.panel.transform = CGAffineTransformMakeTranslation(0, 24); }
        completion:^(BOOL finished) { [self dismissViewControllerAnimated:NO completion:nil]; }];
}
@end

static void Check(BOOL ok,NSString *message) {if(!ok){NSLog(@"FAIL %@",message);exit(1);}}
static void ShotView(UIView *view,NSString *name) {
    [view setNeedsLayout];[view layoutIfNeeded];
    UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithBounds:view.bounds];
    UIImage *im=[r imageWithActions:^(UIGraphicsImageRendererContext *c){[view drawViewHierarchyInRect:view.bounds afterScreenUpdates:YES];}];
    NSString *folder=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"shots"];
    [NSFileManager.defaultManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:nil];
    [UIImagePNGRepresentation(im) writeToFile:[folder stringByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
}
static void Shot(TTGSettingsController *vc,NSString *name) {ShotView(vc.view,name);}
@interface InstalledSettings:TTGSettingsController
@end
@implementation InstalledSettings
- (BOOL)shijimaInstalled {return YES;}
@end
// Deterministic UIKit drag input fixture, never compiled into the tweak.
@interface JK60TestPan:UIPanGestureRecognizer
@property(nonatomic) UIGestureRecognizerState fixtureState;
@property(nonatomic) CGPoint fixtureDelta;
@end
@implementation JK60TestPan
- (UIGestureRecognizerState)state{return self.fixtureState;}
- (CGPoint)translationInView:(UIView *)view{return self.fixtureDelta;}
@end
@interface PreviewApp:UIResponder<UIApplicationDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation PreviewApp
- (BOOL)application:(UIApplication *)app didFinishLaunchingWithOptions:(NSDictionary *)options {
    [[UIView appearance] setSemanticContentAttribute:UISemanticContentAttributeForceRightToLeft];
    [[UILabel appearance] setTextAlignment:NSTextAlignmentRight];
    self.window=[[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.backgroundColor=[UIColor colorWithRed:.12 green:.16 blue:.22 alpha:1];
    CAGradientLayer *backdrop=[CAGradientLayer layer];backdrop.frame=self.window.bounds;
    backdrop.colors=@[(id)UIColor.systemTealColor.CGColor,(id)UIColor.systemOrangeColor.CGColor,(id)UIColor.systemBlueColor.CGColor];
    [self.window.layer insertSublayer:backdrop atIndex:0];
    TTGSettingsController *vc=[TTGSettingsController new];self.window.rootViewController=vc;[self.window makeKeyAndVisible];
    // Prime real AVPlayers before entering the long main-queue fixture block.
    // AVFoundation's main-queue readiness callbacks cannot drain reentrantly
    // from runUntilDate inside that same block.
    NSURL *fixtureURL=[NSBundle.mainBundle URLForResource:@"player-fixture" withExtension:@"mp4"];
    AVPlayer *fixtureActive=[AVPlayer playerWithURL:fixtureURL],*fixtureInactive=[AVPlayer playerWithURL:fixtureURL];
    [fixtureActive.currentItem.asset loadValuesAsynchronouslyForKeys:@[@"playable",@"tracks"] completionHandler:^{}];
    [fixtureInactive.currentItem.asset loadValuesAsynchronouslyForKeys:@[@"playable",@"tracks"] completionHandler:^{}];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,5*NSEC_PER_SEC),dispatch_get_main_queue(), ^{
        Check([vc.pageTitle.text isEqual:@"Joker"],@"brand");
        Check(vc.backButton.hidden,@"home back hidden");
        Check(vc.stack.arrangedSubviews.count==2,@"one physical grid");
        NSInteger tiles=0;
        for(UIView *row in vc.stack.arrangedSubviews)for(UIView *v in row.subviews)if([v isKindOfClass:UIButton.class])tiles++;
        Check(tiles==6,@"six tiles when Shijima absent");
        for(UIView *tile in vc.stack.arrangedSubviews.firstObject.subviews)Check(tile.tag!=5,@"no absent Shijima tile");
        Check(vc.pageHeader.superview==vc.panel.contentView,@"pinned header");
        [vc.view setNeedsLayout];[vc.view layoutIfNeeded];
        Check(vc.panel.bounds.size.width<=vc.view.bounds.size.width*.71,@"compact width");
        Check(CGRectGetMinX(vc.pageTitle.frame)<CGRectGetMinX(vc.closeButton.frame),@"physical title left close right");
        UIView *grid=vc.stack.arrangedSubviews.firstObject;[grid layoutIfNeeded];
        Check(grid.subviews[0].frame.origin.x<grid.subviews[1].frame.origin.x,@"download first left");
        Check(grid.subviews[2].frame.origin.y==grid.subviews[0].frame.origin.y,@"three columns");
        UIButton *shareRow=(UIButton *)vc.stack.arrangedSubviews.lastObject;
        Check([shareRow.accessibilityLabel isEqual:@"مشاركة سجل التشخيص"],@"49 share row bottom");
        NSURL *logURL=[vc diagnosticLogURL];
        Check([logURL.lastPathComponent isEqual:@"joker_debug.txt"],@"49 correct file");
        Check([NSFileManager.defaultManager fileExistsAtPath:logURL.path],@"49 missing log created");
        [@"fixture-class-only" writeToURL:logURL atomically:YES encoding:NSUTF8StringEncoding error:nil];
        Check([[NSString stringWithContentsOfURL:[vc diagnosticLogURL] encoding:NSUTF8StringEncoding error:nil] hasPrefix:@"fixture-class-only"],@"49 existing log preserved, capture appended");
        for(TTJokerTile *tile in grid.subviews) {
            Check(tile.caption.text.length>0 && tile.glyph.image!=nil,@"49 all tile content present");
            Check(CGRectContainsRect(tile.bounds,tile.caption.frame) && CGRectContainsRect(tile.bounds,tile.glyph.frame),@"49 tile glyph and label contained");
            Check(tile.glyph.frame.origin.x<tile.bounds.size.width*.5,@"49 icon top left");
        }
        UIView *headerFixture=[[UIView alloc] initWithFrame:CGRectMake(0,60,self.window.bounds.size.width,60)];
        UILabel *tabA=[[UILabel alloc] initWithFrame:CGRectMake(40,8,70,30)];tabA.text=@"لك";
        UILabel *tabB=[[UILabel alloc] initWithFrame:CGRectMake(120,8,80,30)];tabB.text=@"أتابعه";
        UIButton *searchFixture=[[UIButton alloc] initWithFrame:CGRectMake(210,8,30,30)];
        [headerFixture addSubview:tabA];[headerFixture addSubview:tabB];[headerFixture addSubview:searchFixture];[self.window addSubview:headerFixture];
        TT49HideTop(self.window);Check(headerFixture.hidden,@"49 hides entire top tabs plus icons");
        TT49HideTop(self.window);TT49RestoreTop();Check(!headerFixture.hidden,@"49 repeated hide restores original visibility");
        [headerFixture removeFromSuperview];
        NSMutableArray<UIView *> *exactTop=[NSMutableArray new];
        for(NSString *name in @[@"TikTokFeedTabControl",@"TikTokFeedFadeScrollView",@"TTKFeedTabCornerItemView",
            @"AWELiveFeedEntranceView",@"TTKSearchEntranceButton",@"TikTokFeedTabItemNearbyControl",
            @"TikTokFeedTabItemFollowControl",@"TikTokFeedTabItemMallControl"]) {
            Class cls=NSClassFromString(name);
            if(!cls){cls=objc_allocateClassPair([name isEqualToString:@"TikTokFeedFadeScrollView"]?UIScrollView.class:UIView.class,name.UTF8String,0);objc_registerClassPair(cls);}
            UIView *item=[[cls alloc] initWithFrame:CGRectMake(10,60,44,44)];
            item.alpha=.73;item.userInteractionEnabled=YES;[self.window addSubview:item];[exactTop addObject:item];
        }
        TT50EyeActive=YES;TT50Tree(self.window);
        for(UIView *item in exactTop)Check(item.alpha==0 && !item.userInteractionEnabled,@"50 exact class hidden without tab labels");
        for(UIView *item in exactTop){item.alpha=1;item.userInteractionEnabled=YES;TT50BeforeDraw(item);}
        for(UIView *item in exactTop)Check(item.alpha==0 && !item.userInteractionEnabled,@"50 lifecycle reapply");
        Class newClass=NSClassFromString(@"TikTokFeedTabItemControl");
        if(!newClass){newClass=objc_allocateClassPair(UIView.class,"TikTokFeedTabItemControl",0);objc_registerClassPair(newClass);}
        UIView *reused=[[newClass alloc] initWithFrame:CGRectMake(50,60,30,44)];
        reused.alpha=.4;reused.userInteractionEnabled=NO;[self.window addSubview:reused];
        TT50BeforeDraw(reused);Check(reused.alpha==0,@"50 newly reused tab stays hidden");
        UIView *video=[UIView new];Class rightClass=NSClassFromString(@"TTKFeedRightInteractionStackView");
        if(!rightClass){rightClass=objc_allocateClassPair(UIView.class,"TTKFeedRightInteractionStackView",0);objc_registerClassPair(rightClass);}
        UIView *right=[[rightClass alloc] init];right.alpha=.8;[video addSubview:right];
        TT50Tree(video);
        Check(right.hidden && right.alpha==0 && video.alpha==1 && !video.hidden,@"50 guards only chrome never video/root");
        Check(TT50Alpha(right,1)==0 && TT50Hidden(right,NO) && !TT50Interaction(right,YES),@"50 synchronous setter clamps before draw");
        TT50Restore();
        Check(fabs(right.alpha-.8)<.001 && !right.hidden,@"50 chrome restores without animation");
        for(UIView *item in exactTop){Check(fabs(item.alpha-.73)<.001 && item.userInteractionEnabled,@"50 original state restored");[item removeFromSuperview];}
        Check(fabs(reused.alpha-.4)<.001 && !reused.userInteractionEnabled,@"50 disabled state preserved");[reused removeFromSuperview];
        UIWindow *lateWindow=[[UIWindow alloc] initWithWindowScene:self.window.windowScene];
        lateWindow.hidden=YES;
        Check([TT50DiagnosticWindows() containsObject:lateWindow],@"50 capture includes newly created hidden windows");
        NSLog(@"PASS 50 exact top classes, lifecycle reapply, reuse, original-state restore and fresh all-window capture");
        #include "style_smoke.inc"
        #include "sheet_smoke.inc"
        Shot(vc,@"home");
        for(NSInteger page=0;page<7;page++){
            [vc showCategory:page animated:NO];
            Check(!vc.backButton.hidden,@"back on category");
            Check(vc.category==page && vc.stack.arrangedSubviews.count>0,@"category populated");
            Check(vc.panelHeight.constant>100,@"panel height");
            [vc.view setNeedsLayout];[vc.view layoutIfNeeded];
            CGRect backFrame=[vc.backButton convertRect:vc.backButton.bounds toView:vc.view];
            CGRect titleFrame=[vc.pageTitle convertRect:vc.pageTitle.bounds toView:vc.view];
            CGRect closeFrame=[vc.closeButton convertRect:vc.closeButton.bounds toView:vc.view];
            Check(backFrame.origin.x<titleFrame.origin.x && CGRectGetMaxX(titleFrame)<=closeFrame.origin.x+1,@"all subpage header physical LTR");
            Check(vc.pageTitle.textAlignment==NSTextAlignmentLeft,@"all subpage titles left aligned");
            for(UIView *row in vc.stack.arrangedSubviews)if([row isKindOfClass:TTJokerRow.class]){
                TTJokerRow *r=(TTJokerRow *)row;[r layoutIfNeeded];
                Check(CGRectGetMaxX(r.icon.frame)<CGRectGetMinX(r.caption.frame) && CGRectGetMaxX(r.caption.frame)<=CGRectGetMinX(r.toggle.frame),@"all switches right icons left");
            }
            Shot(vc,[NSString stringWithFormat:@"category-%ld",(long)page]);
            [vc showCategory:-1 animated:NO];
            Check(vc.category==-1 && vc.stack.arrangedSubviews.count==2,@"back restores grid");
        }
        // Regression 47: persist both toggle directions and reopen the category.
        [vc showCategory:0 animated:NO];[vc.view layoutIfNeeded];
        UISwitch *emojiSwitch=(UISwitch *)[vc.stack viewWithTag:246];
        Check(emojiSwitch && emojiSwitch.enabled,@"47 emoji switch enabled");
        Check([emojiSwitch.accessibilityLabel isEqual:@"إظهار بشريط الإيموجي"],@"47 Arabic emoji label");
        [emojiSwitch setOn:YES];[emojiSwitch sendActionsForControlEvents:UIControlEventValueChanged];
        Check([D() boolForKey:@"tt46.emojiBar"],@"47 ON persists");
        [vc showCategory:-1 animated:NO];[vc showCategory:0 animated:NO];[vc.view layoutIfNeeded];
        emojiSwitch=(UISwitch *)[vc.stack viewWithTag:246];
        Check(emojiSwitch.on,@"47 ON survives category reopen");
        [emojiSwitch setSemanticContentAttribute:UISemanticContentAttributeForceRightToLeft];
        [emojiSwitch layoutIfNeeded];
        Check(emojiSwitch.semanticContentAttribute==UISemanticContentAttributeForceLeftToRight,@"47 switch resists RTL host");
        Shot(vc,@"47-download-on");
        [emojiSwitch setOn:NO];[emojiSwitch sendActionsForControlEvents:UIControlEventValueChanged];
        Check(![D() boolForKey:@"tt46.emojiBar"],@"47 OFF persists");
        [vc showCategory:-1 animated:NO];[vc showCategory:0 animated:NO];
        Check(!((UISwitch *)[vc.stack viewWithTag:246]).on,@"47 OFF survives reopen");
        InstalledSettings *installed=[InstalledSettings new];[installed loadViewIfNeeded];
        [installed showCategory:-1 animated:NO];
        Check(installed.stack.arrangedSubviews.firstObject.subviews.count==7,@"installed Shijima tile retained");
        // Original mascot rendering, physics, settings, and pause-state smoke.
        [D() setBool:YES forKey:@"tt.pet.enabled"];
        TTJokerPet *pet=[TTJokerPet shared];[pet attach:vc];[pet refresh];
        Check(pet.frames.count==4,@"four original cached sprite frames");
        Check(![UIImagePNGRepresentation(pet.frames[0]) isEqual:UIImagePNGRepresentation(pet.frames[1])],@"animated sprite frames differ");
        pet.sprite.center=CGPointMake(100,120);[pet begin:@"fall" duration:10];pet.randomSeed=123;pet.velocity=CGPointMake(0,0);
        for(int i=0;i<180;i++){[pet advance:.033];if([pet.behavior isEqual:@"stand"])break;}
        Check(fabs(pet.sprite.center.y-[pet floorY])<.1,@"gravity floor clamp");
        Check(pet.sprite.center.x>=pet.sprite.bounds.size.width/2,@"edge clamp");
        pet.covered=YES;[pet refresh];Check((!pet.link || pet.link.paused) && pet.sprite.hidden,@"pause when settings cover mascot");
        pet.covered=NO;pet.uiHidden=YES;[pet refresh];Check(!pet.link || pet.link.paused,@"eye pauses display link");
        pet.uiHidden=NO;[D() setBool:NO forKey:@"tt.pet.enabled"];[pet refresh];Check((!pet.link || pet.link.paused) && pet.sprite.hidden,@"disabled pauses sprite");
        [D() setBool:YES forKey:@"tt.pet.enabled"];[D() setInteger:1 forKey:@"tt.pet.kind"];[D() setDouble:64 forKey:@"tt.pet.size"];[pet refresh];
        Check(pet.kind==1 && pet.sprite.bounds.size.width==64,@"character and size controls");
        pet.sprite.center=CGPointMake(90,[pet floorY]);
        Check(pet.sprite.superview==self.window,@"mascot attached to window, not settings controller");
        Check(!pet.sprite.hidden && pet.sprite.image && pet.sprite.alpha>.99,@"enabled mascot visible");
        Check(CGRectContainsRect(self.window.bounds,pet.sprite.frame),@"nonzero mascot on screen");
        Check(pet.sprite.bounds.size.width==64 && pet.sprite.bounds.size.height==64,@"nonzero sprite size");
        ShotView(self.window,@"mascot");
        vc.panel.hidden=YES;
        ShotView(self.window,@"mascot-uncovered");
        vc.panel.hidden=NO;
        [D() setBool:NO forKey:@"tt.pet.enabled"];[pet refresh];
        // Provider UTI regression: generic-data ZIPs remain selectable.
        NSArray *pickerTypes=TTPackPickerTypes(NO);
        Check([pickerTypes containsObject:UTTypeZIP] && [pickerTypes containsObject:UTTypeArchive] &&
              [pickerTypes containsObject:UTTypeData] && [pickerTypes containsObject:UTTypeFolder] &&
              [pickerTypes containsObject:[UTType typeWithFilenameExtension:@"zip"]],@"ZIP provider types accepted");
        Check([TTPackPickerTypes(YES) isEqual:@[UTTypeFolder]],@"separate folder picker types");
        UIDocumentPickerViewController *copyPicker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:pickerTypes asCopy:YES];
        Check(copyPicker!=nil,@"broad copy picker initializes");
        // Pack import integration: real ZIP reader, folder import, pose fallback and malicious input.
        NSString *fixtures=[NSBundle.mainBundle.bundlePath stringByAppendingPathComponent:@"Fixtures"];
        NSError *packError=nil;
        NSString *imported=TTPackImport([NSURL fileURLWithPath:[fixtures stringByAppendingPathComponent:@"good.zip"]],&packError);
        Check(imported!=nil,@"valid nested ZIP imported");
        Check(TTPackLoad(imported).count==46,@"all 46 PNG frames persisted");
        [D() setObject:imported forKey:@"tt.pet.pack"];[pet refresh];
        Check(pet.packFrames.count==46,@"active imported pack");
        Check([pet.packActions[@"walk"][0][0] integerValue]==3,@"custom actions frame indices persisted");
        Check([pet.packActions[@"walk"][0][3] integerValue]==112,@"custom pose anchor persisted");
        for(NSString *state in @[@"stand",@"walk",@"fall",@"drag",@"sit",@"climb"]) {
            [pet updateSprite:state];Check(pet.sprite.image!=nil && [pet.state isEqual:state],@"all six sprite states");
        }
        [D() setBool:YES forKey:@"tt.pet.enabled"];[pet refresh];pet.sprite.center=CGPointMake(90,[pet floorY]);
        [pet updateSprite:@"walk"];ShotView(self.window,@"pack-walk");
        [pet updateSprite:@"drag"];ShotView(self.window,@"pack-drag");
        for(NSString *bad in @[@"traversal.zip",@"symlink.zip",@"duplicate.zip",@"badpng.zip",@"missing.zip",@"oversize.zip",@"xxe.zip",@"entity.zip",@"deepxml.zip",@"badxml.zip",@"largexml.zip"]){
            packError=nil;
            Check(TTPackImport([NSURL fileURLWithPath:[fixtures stringByAppendingPathComponent:bad]],&packError)==nil && packError!=nil,@"unsafe ZIP rejected");
        }
        NSString *folder=TTPackImport([NSURL fileURLWithPath:[fixtures stringByAppendingPathComponent:@"folder"]],&packError);
        Check(folder && TTPackLoad(folder).count==1,@"minimal folder pack imported");
        [D() setObject:folder forKey:@"tt.pet.pack"];[pet refresh];[pet updateSprite:@"climb"];
        Check(pet.sprite.image==pet.packFrames[@1],@"missing pose falls back to stand");
        #include "mascot_smoke.inc"
        #include "emoji_smoke.inc"
        [D() setObject:@"" forKey:@"tt.pet.pack"];[pet refresh];Check(!pet.packFrames,@"original default restored");
        [D() setBool:NO forKey:@"tt.pet.enabled"];[pet refresh];
        // Persisted artwork, cache reload, target isolation and reset.
        UIGraphicsImageRenderer *renderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(64,40)];
        UIImage *image=[renderer imageWithActions:^(UIGraphicsImageRendererContext *ctx){[UIColor.redColor setFill];UIRectFill(CGRectMake(0,0,64,40));}];
        NSURL *url=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"test-icon.png"]];
        [UIImagePNGRepresentation(image) writeToURL:url atomically:YES];
        [[TTToolPicker shared] importURL:url target:1221];
        dispatch_async(dispatch_get_main_queue(), ^{
            Check(TTToolImage(1221)!=nil,@"imported image");
            [TTToolCache() removeObjectForKey:@1221];
            UIImage *saved=TTToolImage(1221);
            Check(saved && saved.size.width==28,@"bounded persisted icon");
            Check(saved.renderingMode==UIImageRenderingModeAlwaysOriginal,@"preserves photo color");
            Check(TTToolImage(1222)==nil,@"eye unchanged");
            [NSFileManager.defaultManager removeItemAtPath:TTToolPath(1221) error:nil];
            [TTToolCache() removeObjectForKey:@1221];Check(TTToolImage(1221)==nil,@"reset");
            [vc showCategory:-1 animated:NO];
            UIButton *button=(UIButton *)vc.stack.arrangedSubviews.lastObject;
            [button sendActionsForControlEvents:UIControlEventTouchUpInside];
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,2*NSEC_PER_SEC),dispatch_get_main_queue(), ^{
                Check([vc.presentedViewController isKindOfClass:UIActivityViewController.class],@"49 row opens native share sheet");
                ShotView(self.window,@"49-share-sheet");
                NSLog(@"PASS 49 local file and native share presentation");
                NSLog(@"PASS UIKit grid/category/back/layout/art persistence smoke");
                fflush(stdout);exit(0);
            });
        });
    });
    return YES;
}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(PreviewApp.class));}}
