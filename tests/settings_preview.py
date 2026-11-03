from pathlib import Path
s=Path('Tweak.x').read_text()
controller=s[s.index('// Physical LTR containers'):s.index('static void TTGShowSettings(UITabBar *bar) {',s.index('@interface TTGSettingsController'))]
prefix=r"""
#import <UIKit/UIKit.h>
#import <Photos/Photos.h>
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
@interface TT122Hub:NSObject
+(instancetype)shared;
-(void)refresh;
@end
@implementation TT122Hub
+(instancetype)shared {static TT122Hub *s;if(!s)s=[self new];return s;}
-(void)refresh {}
@end
"""
suffix=r"""
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
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(), ^{
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
        Check([[NSString stringWithContentsOfURL:[vc diagnosticLogURL] encoding:NSUTF8StringEncoding error:nil] isEqual:@"fixture-class-only"],@"49 existing log preserved");
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
            Check([vc.presentedViewController isKindOfClass:UIActivityViewController.class],@"49 row opens native share sheet");
            NSLog(@"PASS 49 local file and native share presentation");
            NSLog(@"PASS UIKit grid/category/back/layout/art persistence smoke");
            fflush(stdout);exit(0);
        });
    });
    return YES;
}
@end
int main(int argc,char **argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(PreviewApp.class));}}
"""
Path('tests/SettingsPreview.m').write_text(prefix+controller+suffix)
