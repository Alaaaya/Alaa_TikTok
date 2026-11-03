// Explicit ownership registry. Never discovers or edits native UIKit glass.
@interface JK61GlassRecord:NSObject
@property(nonatomic,weak) UIView *view,*tint;
@property(nonatomic,strong) UIColor *background,*tintColor,*border;
@property(nonatomic,strong) UIVisualEffect *effect;
@property(nonatomic,strong) UIViewPropertyAnimator *animator;
@property(nonatomic) CGFloat width;
@property(nonatomic,copy) NSString *role;
@end
@implementation JK61GlassRecord
- (void)dealloc{[_animator stopAnimation:YES];}
@end
static NSMapTable<UIView *,JK61GlassRecord *> *JK61Glasses(void){static NSMapTable *m;static dispatch_once_t once;dispatch_once(&once,^{m=[NSMapTable weakToStrongObjectsMapTable];});return m;}
static NSDictionary *JK61Values;
static NSArray *JK61Keys(void){return @[@"tint",@"blur",@"edge"];}
static NSDictionary *JK61Defaults(void){return @{@"tint":@0.5,@"blur":@1,@"edge":@0};}
static NSDictionary *JK61Read(void){if(!JK61Values){id v=[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt61.glass"];JK61Values=v?:JK61Defaults();}return JK61Values;}
static CGFloat JK61Value(NSString *key){id v=JK61Read()[key];double d=[v isKindOfClass:NSNumber.class]?[v doubleValue]:[JK61Defaults()[key] doubleValue];return isfinite(d)?MAX(0,MIN(1,d)):[JK61Defaults()[key] doubleValue];}
static BOOL JK61Customized(void){return fabs(JK61Value(@"tint")-.5)>.001||JK61Value(@"blur")<.999||JK61Value(@"edge")>0;}
static void JK61Apply(JK61GlassRecord *r){
    UIView *v=r.view;if(!v)return;
    BOOL custom=JK61Customized(),reduce=UIAccessibilityIsReduceTransparencyEnabled();
    if(r.animator && (!custom||reduce)){[r.animator stopAnimation:YES];r.animator=nil;}
    v.backgroundColor=r.background;r.tint.backgroundColor=r.tintColor;
    v.layer.borderWidth=r.width;v.layer.borderColor=r.border.CGColor;
    if([v isKindOfClass:UIVisualEffectView.class]){
        UIVisualEffectView *e=(id)v;
        if(!custom||reduce){e.effect=reduce?nil:r.effect;}
        else if(!r.animator){
            e.effect=nil;__weak UIVisualEffectView *weak=e;UIVisualEffect *target=r.effect;
            r.animator=[[UIViewPropertyAnimator alloc] initWithDuration:1 curve:UIViewAnimationCurveLinear animations:^{weak.effect=target;}];
            [r.animator startAnimation];[r.animator pauseAnimation];
        }
        if(r.animator)r.animator.fractionComplete=MAX(.02,MIN(.999,JK61Value(@"blur")));
    }
    if(custom||reduce){
        CGFloat tint=reduce?1:MAX(0,(JK61Value(@"tint")-.5)*.7);
        UIView *target=r.tint?:([v isKindOfClass:UIVisualEffectView.class]?((UIVisualEffectView *)v).contentView:v);
        // Bounded dimming affects the owned material only, never child text alpha.
        UIColor *base=r.tint?r.tintColor:r.background;CGFloat a=base?CGColorGetAlpha(base.CGColor):0;
        CGFloat adjusted=MAX(tint,a*(.25+1.5*JK61Value(@"tint")));
        if([r.role isEqual:@"panel"])adjusted=MAX(.16,adjusted);
        target.backgroundColor=reduce?UIColor.blackColor:[(base?:UIColor.blackColor) colorWithAlphaComponent:adjusted];
        v.layer.borderWidth=r.width+JK61Value(@"edge")*1.5;
        v.layer.borderColor=[UIColor colorWithWhite:1 alpha:MAX(r.border?CGColorGetAlpha(r.border.CGColor):0,JK61Value(@"edge")*.8)].CGColor;
    }
}
static void JK61Register(UIView *v,UIView *tint,NSString *role){
    if(!v)return;if(!tint&&[v isKindOfClass:UIVisualEffectView.class])tint=((UIVisualEffectView *)v).contentView;JK61GlassRecord *r=[JK61Glasses() objectForKey:v];
    if(!r){r=[JK61GlassRecord new];r.view=v;r.tint=tint;r.role=role;r.background=v.backgroundColor;r.tintColor=tint.backgroundColor;r.border=v.layer.borderColor?[UIColor colorWithCGColor:v.layer.borderColor]:nil;r.width=v.layer.borderWidth;if([v isKindOfClass:UIVisualEffectView.class])r.effect=((UIVisualEffectView *)v).effect;[JK61Glasses() setObject:r forKey:v];}
    JK61Apply(r);
}
static void JK61Refresh(void){for(UIView *v in JK61Glasses().keyEnumerator.allObjects)JK61Apply([JK61Glasses() objectForKey:v]);}
@interface JK61Persistence:NSObject
@property(nonatomic,strong) NSTimer *timer;
+(instancetype)shared;
-(void)accessibility:(NSNotification *)note;
@end
@implementation JK61Persistence
+(instancetype)shared{static JK61Persistence *s;static dispatch_once_t once;dispatch_once(&once,^{s=[self new];[NSNotificationCenter.defaultCenter addObserver:s selector:@selector(accessibility:) name:UIAccessibilityReduceTransparencyStatusDidChangeNotification object:nil];});return s;}
-(void)accessibility:(NSNotification *)note{JK61Refresh();}
@end
static void JK61Set(NSString *key,CGFloat value){
    NSMutableDictionary *d=[JK61Read() mutableCopy];d[key]=@(isfinite(value)?MAX(0,MIN(1,value)):0);JK61Values=[d copy];JK61Refresh();
    JK61Persistence *p=[JK61Persistence shared];[p.timer invalidate];
    p.timer=[NSTimer scheduledTimerWithTimeInterval:.2 repeats:NO block:^(__unused NSTimer *t){[NSUserDefaults.standardUserDefaults setObject:JK61Values forKey:@"tt61.glass"];}];
}
static void JK61Reset(void){[[JK61Persistence shared].timer invalidate];JK61Values=JK61Defaults();[NSUserDefaults.standardUserDefaults removeObjectForKey:@"tt61.glass"];JK61Refresh();}
