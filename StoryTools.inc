// Evidence-backed story controllers only; never bypasses story-view receipts.
static id TTStoryControllerForView(UIView *view) {
    UIResponder *responder=view;
    for(NSUInteger i=0;responder && i<40;i++,responder=responder.nextResponder) {
        if([responder isKindOfClass:UIViewController.class]) {
            NSString *name=NSStringFromClass(responder.class);
            if([name isEqualToString:@"TTKStoryContainerViewController"] || [name isEqualToString:@"TTKStoryDetailContainerViewController"]) return responder;
        }
    }
    return nil;
}
static id TTStoryModel(id controller) {
    id model=TTGActionObject(controller,@"model");
    if([NSStringFromClass([controller class]) isEqualToString:@"TTKStoryDetailContainerViewController"]) model=TTGActionObject(model,@"currentPlayingStory");
    return TTGActionObject(model,@"video") ? model : nil;
}
@interface TTStoryTools : NSObject <UIGestureRecognizerDelegate>
+ (instancetype)shared;
- (void)attach:(UIView *)view;
@end
@implementation TTStoryTools
+ (instancetype)shared { static TTStoryTools *s;static dispatch_once_t once;dispatch_once(&once,^{s=[self new];});return s; }
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gesture shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)other { return YES; }
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gesture {
    return [D() boolForKey:@"tt.extra.storySave"] && TTStoryControllerForView(gesture.view)!=nil;
}
- (void)attach:(UIView *)view {
    if(![view isKindOfClass:UIView.class])return;
    for(UIGestureRecognizer *g in view.gestureRecognizers) if([g.name isEqualToString:@"AlaaStorySave"])return;
    UILongPressGestureRecognizer *press=[[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(save:)];
    press.minimumPressDuration=.65;press.cancelsTouchesInView=NO;press.delegate=self;press.name=@"AlaaStorySave";[view addGestureRecognizer:press];
}
- (void)save:(UILongPressGestureRecognizer *)press {
    if(press.state!=UIGestureRecognizerStateBegan || ![D() boolForKey:@"tt.extra.storySave"])return;
    UIViewController *controller=TTStoryControllerForView(press.view);id model=TTStoryModel(controller);
    if(!model){[[TT122Hub shared] toast:@"حفظ هذه القصة غير متاح"];return;}
    TT122Hub *hub=[TT122Hub shared];
    [hub track:controller];[hub start:model quality:0];
}
@end
