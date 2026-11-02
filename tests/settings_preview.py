from pathlib import Path
s=Path('Tweak.x').read_text()
controller=s[s.index('@interface TTGSettingsController'):s.index('static void TTGShowSettings(UITabBar *bar) {',s.index('@interface TTGSettingsController'))]
prefix=r"""
#import <UIKit/UIKit.h>
#import <Photos/Photos.h>
#import <objc/runtime.h>
#import "../CustomTools.inc"
static NSString *kPrefEnabled=@"enabled",*kPrefFloating=@"floating",*kPrefUnder=@"under",*kPrefLabels=@"labels",*kPrefStyle=@"style";
static NSInteger kGlassTag=777,kSelTag=778;
static NSUserDefaults *D(void){return NSUserDefaults.standardUserDefaults;}
static BOOL PB(NSString *key,BOOL fallback){id v=[D() objectForKey:key];return v?[v boolValue]:fallback;}
static NSInteger TTGStyle(void){return [D() integerForKey:kPrefStyle];}
static void TTGRefreshAll(void){}
static void TTToolRefresh(void){}
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
    self.window=[[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.backgroundColor=[UIColor colorWithRed:.12 green:.16 blue:.22 alpha:1];
    TTGSettingsController *vc=[TTGSettingsController new];self.window.rootViewController=vc;[self.window makeKeyAndVisible];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(), ^{
        Check([vc.pageTitle.text isEqual:@"Joker"],@"brand");
        Check(vc.backButton.hidden,@"home back hidden");
        Check(vc.stack.arrangedSubviews.count==3,@"three grid rows");
        NSInteger tiles=0;
        for(UIStackView *row in vc.stack.arrangedSubviews)for(UIView *v in row.arrangedSubviews)if([v isKindOfClass:UIButton.class])tiles++;
        Check(tiles==5,@"exactly five tiles");
        Check(vc.pageHeader.superview==vc.panel.contentView,@"pinned header");
        Shot(vc,@"home");
        for(NSInteger page=0;page<5;page++){
            [vc showCategory:page animated:NO];
            Check(!vc.backButton.hidden,@"back on category");
            Check(vc.category==page && vc.stack.arrangedSubviews.count>0,@"category populated");
            Check(vc.panelHeight.constant>100,@"panel height");
            Shot(vc,[NSString stringWithFormat:@"category-%ld",(long)page]);
            [vc showCategory:-1 animated:NO];
            Check(vc.category==-1 && vc.stack.arrangedSubviews.count==3,@"back restores grid");
        }
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
