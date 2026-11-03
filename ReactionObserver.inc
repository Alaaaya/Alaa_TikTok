// Observe UIKit hierarchy/lifecycle without intercepting or replacing TikTok gestures.
static BOOL TT48Pending=NO;
static void TT48Schedule(void) {
    if(![NSThread isMainThread] || !PB(@"tt46.emojiBar",NO) || TT48Pending)return;
    TT48Pending=YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.12*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        // Keep pending during the scan to suppress hooks caused by our own companion.
        [[TT122Hub shared] refresh];TT48Pending=NO;
    });
}
static void TT48Press(void) {
    if(!PB(@"tt46.emojiBar",NO))return;
    CFTimeInterval now=CACurrentMediaTime();
    if(TT48LastPress>0 && now-TT48LastPress<.35)return;
    TT48LastPress=now;TT48Debug(@"long-press");
    for(NSNumber *delay in @[@.08,@.35,@.8,@1.5]) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(delay.doubleValue*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
            TT48Schedule();TT48Debug(@"after-long-press");
        });
    }
}
@interface TT48GestureObserver:NSObject
+ (instancetype)shared;
- (void)pressed:(UILongPressGestureRecognizer *)gesture;
@end
@implementation TT48GestureObserver
+ (instancetype)shared {static TT48GestureObserver *s;static dispatch_once_t once;dispatch_once(&once,^{s=[self new];});return s;}
- (void)pressed:(UILongPressGestureRecognizer *)gesture {
    if(gesture.state==UIGestureRecognizerStateBegan)TT48Press();
}
@end
static char TT48ObservedGesture;
static void TT48ObserveGesture(UIGestureRecognizer *gesture) {
    if(![gesture isKindOfClass:UILongPressGestureRecognizer.class] || objc_getAssociatedObject(gesture,&TT48ObservedGesture))return;
    objc_setAssociatedObject(gesture,&TT48ObservedGesture,@YES,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [gesture addTarget:[TT48GestureObserver shared] action:@selector(pressed:)];
}
static void TT48Added(UIView *view) {
    if(![NSThread isMainThread] || !PB(@"tt46.emojiBar",NO))return;
    // Companion controls must never become candidates or cause recursive refresh.
    for(UIView *p=view;p;p=p.superview)if([p isKindOfClass:TT46PillExtension.class])return;
    [TT48Births() setObject:@(CACurrentMediaTime()) forKey:view];
    for(UIGestureRecognizer *g in view.gestureRecognizers)TT48ObserveGesture(g);
    TT48Schedule();
}
