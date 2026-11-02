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
static void Shot(TTGSettingsController *vc,NSString *name) {
    [vc.view setNeedsLayout];[vc.view layoutIfNeeded];
    UIGraphicsImageRenderer *r=[[UIGraphicsImageRenderer alloc] initWithBounds:vc.view.bounds];
    UIImage *im=[r imageWithActions:^(UIGraphicsImageRendererContext *c){[vc.view drawViewHierarchyInRect:vc.view.bounds afterScreenUpdates:YES];}];
    NSString *folder=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"shots"];
    [NSFileManager.defaultManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:nil];
    [UIImagePNGRepresentation(im) writeToFile:[folder stringByAppendingPathComponent:[name stringByAppendingString:@".png"]] atomically:YES];
}
@interface PreviewApp:UIResponder<UIApplicationDelegate>
@property(nonatomic,strong) UIWindow *window;
@end
@implementation PreviewApp
- (BOOL)application:(UIApplication *)app didFinishLaunchingWithOptions:(NSDictionary *)options {
    [[UIView appearance] setSemanticContentAttribute:UISemanticContentAttributeForceRightToLeft];
    self.window=[[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.backgroundColor=[UIColor colorWithRed:.12 green:.16 blue:.22 alpha:1];
    CAGradientLayer *backdrop=[CAGradientLayer layer];backdrop.frame=self.window.bounds;
    backdrop.colors=@[(id)UIColor.systemTealColor.CGColor,(id)UIColor.systemOrangeColor.CGColor,(id)UIColor.systemBlueColor.CGColor];
    [self.window.layer insertSublayer:backdrop atIndex:0];
    TTGSettingsController *vc=[TTGSettingsController new];self.window.rootViewController=vc;[self.window makeKeyAndVisible];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(), ^{
        Check([vc.pageTitle.text isEqual:@"Joker"],@"brand");
        Check(vc.backButton.hidden,@"home back hidden");
        Check(vc.stack.arrangedSubviews.count==1,@"one physical grid");
        NSInteger tiles=0;
        for(UIView *row in vc.stack.arrangedSubviews)for(UIView *v in row.subviews)if([v isKindOfClass:UIButton.class])tiles++;
        Check(tiles==7,@"exactly seven tiles");
        Check(vc.pageHeader.superview==vc.panel.contentView,@"pinned header");
        [vc.view setNeedsLayout];[vc.view layoutIfNeeded];
        Check(vc.panel.bounds.size.width<=vc.view.bounds.size.width*.71,@"compact width");
        Check(CGRectGetMinX(vc.pageTitle.frame)<CGRectGetMinX(vc.closeButton.frame),@"physical title left close right");
        UIView *grid=vc.stack.arrangedSubviews.firstObject;[grid layoutIfNeeded];
        Check(grid.subviews[0].frame.origin.x<grid.subviews[1].frame.origin.x,@"download first left");
        Check(grid.subviews[2].frame.origin.y==grid.subviews[0].frame.origin.y,@"three columns");
        Shot(vc,@"home");
        for(NSInteger page=0;page<7;page++){
            [vc showCategory:page animated:NO];
            Check(!vc.backButton.hidden,@"back on category");
            Check(vc.category==page && vc.stack.arrangedSubviews.count>0,@"category populated");
            Check(vc.panelHeight.constant>100,@"panel height");
            Shot(vc,[NSString stringWithFormat:@"category-%ld",(long)page]);
            [vc showCategory:-1 animated:NO];
            Check(vc.category==-1 && vc.stack.arrangedSubviews.count==1,@"back restores grid");
        }
        // Original mascot rendering, physics, settings, and pause-state smoke.
        [D() setBool:YES forKey:@"tt.pet.enabled"];
        TTJokerPet *pet=[TTJokerPet shared];[pet attach:vc];[pet refresh];
        Check(pet.frames.count==4,@"four original cached sprite frames");
        Check(![UIImagePNGRepresentation(pet.frames[0]) isEqual:UIImagePNGRepresentation(pet.frames[1])],@"animated sprite frames differ");
        pet.sprite.center=CGPointMake(100,120);pet.velocity=CGPointMake(500,0);
        for(int i=0;i<180;i++){[pet advance:.033];}
        Check(fabs(pet.sprite.center.y-[pet floorY])<.1,@"gravity floor clamp");
        Check(pet.sprite.center.x>=pet.sprite.bounds.size.width/2,@"edge clamp");
        pet.covered=YES;[pet refresh];Check((!pet.link || pet.link.paused) && pet.sprite.hidden,@"pause when settings cover mascot");
        pet.covered=NO;pet.uiHidden=YES;[pet refresh];Check(!pet.link || pet.link.paused,@"eye pauses display link");
        pet.uiHidden=NO;[D() setBool:NO forKey:@"tt.pet.enabled"];[pet refresh];Check((!pet.link || pet.link.paused) && pet.sprite.hidden,@"disabled pauses sprite");
        [D() setBool:YES forKey:@"tt.pet.enabled"];[D() setInteger:1 forKey:@"tt.pet.kind"];[D() setDouble:64 forKey:@"tt.pet.size"];[pet refresh];
        Check(pet.kind==1 && pet.sprite.bounds.size.width==64,@"character and size controls");
        pet.sprite.center=CGPointMake(90,[pet floorY]);Shot(vc,@"mascot");
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
