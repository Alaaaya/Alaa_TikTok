// Opt-in bottom-bar paint only. No frame, visibility, navigation or gesture writes.
@interface JK61BarPaint:NSObject
@property(nonatomic,weak) UIView *bar;
@property(nonatomic,strong) UIView *plate;
@property(nonatomic,strong) NSMapTable<UIView *,NSArray *> *ink;
@end
@implementation JK61BarPaint
@end
static const void *kJK61BarSelected=&kJK61BarSelected;
static NSMapTable<UIView *,JK61BarPaint *> *JK61Bars(void){static NSMapTable *m;static dispatch_once_t once;dispatch_once(&once,^{m=[NSMapTable weakToStrongObjectsMapTable];});return m;}
static UIColor *JK61BarColor(void){id a=[NSUserDefaults.standardUserDefaults arrayForKey:@"tt61.barColor"];if(![a isKindOfClass:NSArray.class]||[a count]!=3)return nil;for(id n in a)if(![n isKindOfClass:NSNumber.class]||!isfinite([n doubleValue])||[n doubleValue]<0||[n doubleValue]>1)return nil;return [UIColor colorWithRed:[a[0] doubleValue] green:[a[1] doubleValue] blue:[a[2] doubleValue] alpha:1];}
static BOOL JK61ActualBar(UIView *v){return ([v isKindOfClass:UITabBar.class]&&![NSStringFromClass(v.class) isEqualToString:@"TTKFakeTabBar"])||[NSStringFromClass(v.class) isEqualToString:@"TTKTabBar"];}
static void JK61BarRestore(UIView *bar){JK61BarPaint *r=[JK61Bars() objectForKey:bar];if(!r)return;[r.plate removeFromSuperview];for(UIView *v in r.ink.keyEnumerator.allObjects){NSArray *a=[r.ink objectForKey:v];v.tintColor=a[0]==NSNull.null?nil:a[0];if([v isKindOfClass:UILabel.class])((UILabel *)v).textColor=a[1];if([v isKindOfClass:UIImageView.class])((UIImageView *)v).image=a[2]==NSNull.null?nil:a[2];if([v isKindOfClass:UITabBar.class])((UITabBar *)v).unselectedItemTintColor=a[3]==NSNull.null?nil:a[3];}[JK61Bars() removeObjectForKey:bar];}
static void JK61BarInk(JK61BarPaint *r,UIView *v,UIColor *color,BOOL selected,NSUInteger depth){
    if(depth>10||v==r.plate||[v isKindOfClass:UIVisualEffectView.class]||[NSStringFromClass(v.class) isEqualToString:@"AWETabBarPlusButton"])return;
    id actual=objc_getAssociatedObject(v,kJK61BarSelected);if(actual)selected=[actual boolValue];
    if([v isKindOfClass:UILabel.class]||[v isKindOfClass:UIImageView.class]||[v isKindOfClass:UITabBar.class]){
        [r.ink setObject:@[v.tintColor?:NSNull.null,[v isKindOfClass:UILabel.class]?((UILabel *)v).textColor?:UIColor.whiteColor:NSNull.null,[v isKindOfClass:UIImageView.class]?((UIImageView *)v).image?:NSNull.null:NSNull.null,[v isKindOfClass:UITabBar.class]?((UITabBar *)v).unselectedItemTintColor?:NSNull.null:NSNull.null] forKey:v];
        UIColor *ink=[color colorWithAlphaComponent:selected?1:.68];v.tintColor=ink;
        if([v isKindOfClass:UILabel.class])((UILabel *)v).textColor=ink;
        if([v isKindOfClass:UIImageView.class]){UIImageView *im=(id)v;im.image=[im.image imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];}
        if([v isKindOfClass:UITabBar.class]){v.tintColor=color;((UITabBar *)v).unselectedItemTintColor=[color colorWithAlphaComponent:.68];}
    }
    BOOL sel=selected||([v isKindOfClass:UIControl.class]&&((UIControl *)v).selected);
    for(UIView *c in [v.subviews copy])JK61BarInk(r,c,color,sel,depth+1);
}
static void JK61BarApply(UIView *bar){
    if(!JK61ActualBar(bar))return;JK61BarRestore(bar);UIColor *color=JK61BarColor();if(!color)return;
    JK61BarPaint *r=[JK61BarPaint new];r.bar=bar;r.ink=[NSMapTable weakToStrongObjectsMapTable];r.plate=[[UIView alloc] initWithFrame:bar.bounds];r.plate.tag=9369;r.plate.userInteractionEnabled=NO;r.plate.accessibilityElementsHidden=YES;r.plate.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;r.plate.backgroundColor=color;r.plate.layer.cornerRadius=bar.layer.cornerRadius;r.plate.clipsToBounds=YES;
    // Under native controls, above backgrounds. Reuse the native bar's shape.
    NSUInteger index=0;for(UIView *c in bar.subviews){NSString *n=NSStringFromClass(c.class);if([c isKindOfClass:UIControl.class]||[n hasSuffix:@"TabBarButton"]||[n isEqualToString:@"AWETabBarPlusButton"])break;index++;}[bar insertSubview:r.plate atIndex:MIN(index,bar.subviews.count)];
    [JK61Bars() setObject:r forKey:bar];CGFloat red=0,green=0,blue=0;[color getRed:&red green:&green blue:&blue alpha:nil];UIColor *ink=(.2126*red+.7152*green+.0722*blue)>.55?UIColor.blackColor:UIColor.whiteColor;JK61BarInk(r,bar,ink,NO,0);
}
static void JK61BarRefresh(void){for(UIView *bar in JK61Bars().keyEnumerator.allObjects){JK61BarRestore(bar);[bar setNeedsLayout];}for(UIWindow *window in UIApplication.sharedApplication.windows){NSMutableArray *q=[NSMutableArray arrayWithObject:window];for(NSUInteger i=0;i<q.count&&i<6000;i++){UIView *v=q[i];if(JK61ActualBar(v))JK61BarApply(v);else [q addObjectsFromArray:v.subviews];}}}
static void JK61SetBarColor(UIColor *color){CGFloat r,g,b,a;if(color&&[color getRed:&r green:&g blue:&b alpha:&a])[NSUserDefaults.standardUserDefaults setObject:@[@(r),@(g),@(b)] forKey:@"tt61.barColor"];else [NSUserDefaults.standardUserDefaults removeObjectForKey:@"tt61.barColor"];JK61BarRefresh();}
