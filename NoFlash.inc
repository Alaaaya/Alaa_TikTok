// 60 collapse and independent eye state compose without poisoning snapshots.
static BOOL JKRailClosed=YES;
static BOOL JK60Restoring=NO;
static NSMapTable<UIView *,NSArray *> *JK60Saved(void){static NSMapTable *m;static dispatch_once_t once;dispatch_once(&once,^{m=[NSMapTable weakToStrongObjectsMapTable];});return m;}
static BOOL JK60Active(void){return !JK60Restoring && JKRailClosed && [NSUserDefaults.standardUserDefaults integerForKey:@"tt.interface.style"]==3;}
// Transition records exist only during a dots-tap sequence. Guards remain
// synchronous for newly inserted cells, which are never added to the sequence.
static NSMapTable<UIView *,NSArray *> *JK62Held(void){static NSMapTable *m;static dispatch_once_t once;dispatch_once(&once,^{m=[NSMapTable weakToStrongObjectsMapTable];});return m;}
static NSMapTable<UIView *,UIView *> *JK62Parents(void){static NSMapTable *m;static dispatch_once_t once;dispatch_once(&once,^{m=[NSMapTable weakToWeakObjectsMapTable];});return m;}
static BOOL JK62Writing=NO;
static BOOL JK62Hold(UIView *v){
    if(![JK62Held() objectForKey:v])return NO;
    // A native reused/reparented cell must not inherit an outgoing animation.
    // O(1), no per-frame tree scan and no ownership cycle.
    if([JK62Parents() objectForKey:v]!=v.superview){
        [v.layer removeAnimationForKey:@"joker62.visibility"];
        [JK62Held() removeObjectForKey:v];[JK62Parents() removeObjectForKey:v];return NO;
    }
    return YES;
}
static BOOL JK60Chrome(UIView *view);
static void JK60Save(UIView *view);
static CGFloat JK60Alpha(UIView *v,CGFloat a){if(JK60Active()&&JK60Chrome(v)){JK60Save(v);return JK62Hold(v)?a:0;}return a;}
static BOOL JK60Hidden(UIView *v,BOOL h){if(JK60Active()&&JK60Chrome(v)){JK60Save(v);return JK62Hold(v)?h:YES;}return h;}
static BOOL JK60Interaction(UIView *v,BOOL i){if(!JK62Writing&&JK62Hold(v))return NO;if(JK60Active()&&JK60Chrome(v)){JK60Save(v);return NO;}return i;}
// Synchronous eye-state guards. Never hide a video container, cell, or interaction root.
static BOOL TT50EyeActive=NO;
static NSMapTable<UIView *,NSArray *> *TT50Saved(void) {
    static NSMapTable *map;static dispatch_once_t once;
    dispatch_once(&once,^{map=[NSMapTable weakToStrongObjectsMapTable];});return map;
}
static BOOL TT50Chrome(UIView *view) {
    NSString *name=NSStringFromClass(view.class);
    return [@[@"TikTokFeedTabControl",@"TikTokFeedFadeScrollView",@"TTKFeedTabCornerItemView",
        @"AWELiveFeedEntranceView",@"TTKSearchEntranceButton",@"TTKFeedRightInteractionStackView",
        @"AWEPlayInteractionAuthorView",@"AWEPlayInteractionDescriptionLabel",@"AWEScrollLabelView",
        @"TTKFeedBaseBottomBarView",@"ACCCLACaptionLabel",@"JKStyleItem",@"AWEPlayInteractionUserAvatarView",@"AWEPlayInteractionFollowPromptView",@"AWEPlayInteractionMusicView"] containsObject:name] ||
        ([name hasPrefix:@"TikTokFeedTabItem"] && [name hasSuffix:@"Control"]);
}
static void TT50Save(UIView *view) {
    if(![TT50Saved() objectForKey:view])
        [TT50Saved() setObject:([JK60Saved() objectForKey:view]?:@[@(view.alpha),@(view.hidden),@(view.userInteractionEnabled)]) forKey:view];
}
static CGFloat TT50Alpha(UIView *view,CGFloat requested) {
    if(!TT50EyeActive || !TT50Chrome(view))return requested;
    TT50Save(view);return 0;
}
static BOOL TT50Hidden(UIView *view,BOOL requested) {
    if(!TT50EyeActive || !TT50Chrome(view))return requested;
    TT50Save(view);return YES;
}
static BOOL TT50Interaction(UIView *view,BOOL requested) {
    if(!TT50EyeActive || !TT50Chrome(view))return requested;
    TT50Save(view);return NO;
}
static void TT50BeforeDraw(UIView *view) {
    if(!TT50EyeActive || !TT50Chrome(view))return;
    TT50Save(view);
    [UIView performWithoutAnimation:^{
        if(view.alpha!=0)view.alpha=0;
        if(!view.hidden)view.hidden=YES;
        if(view.userInteractionEnabled)view.userInteractionEnabled=NO;
        [view.layer removeAnimationForKey:@"opacity"];
    }];
}
static void TT50Tree(UIView *root) {
    if(!TT50EyeActive || !root)return;
    NSMutableArray *queue=[NSMutableArray arrayWithObject:root];
    for(NSUInteger i=0;i<queue.count && i<2200;i++) {
        UIView *v=queue[i];TT50BeforeDraw(v);
        if(queue.count<2200)[queue addObjectsFromArray:v.subviews];
    }
}
static void TT50Restore(void) {
    TT50EyeActive=NO;
    [UIView performWithoutAnimation:^{
        for(UIView *view in TT50Saved().keyEnumerator.allObjects) {
            NSArray *state=[TT50Saved() objectForKey:view];
            view.alpha=JK60Alpha(view,[state[0] doubleValue]);view.hidden=JK60Hidden(view,[state[1] boolValue]);
            view.userInteractionEnabled=JK60Interaction(view,[state[2] boolValue]);
        }
    }];
    [TT50Saved() removeAllObjects];
}
static void TT50Lifecycle(UIView *view) {
    if(!TT50EyeActive)return;
    NSString *name=NSStringFromClass(view.class);
    if([name isEqual:@"TTKFeedInteractionRootView"] || [name isEqual:@"AWEFeedViewCell"] ||
       [name isEqual:@"TTKFeedInteractionMainView"])TT50Tree(view);
    else TT50BeforeDraw(view);
}

static BOOL JK60Chrome(UIView *view){
    NSString *n=NSStringFromClass(view.class);
    // Never hide a feed/cell/video container, window, mascot or floating trigger.
    return TT50Chrome(view)||[@[@"TTKTabBar",@"TTKFakeTabBar",@"TTKMultiTabNavigationArrowView",@"AWEFeedPlayerBottomProgressBar",@"TTKFeedTabCornerItemView",@"TTKFeedBaseBottomBarView",@"AWEUserNameLabel",@"AWEAwemeMusicInfoView"] containsObject:n]||view.tag==9360||view.tag==9361||view.tag==1221||view.tag==1222||view.tag==1223;
}
static void JK60Save(UIView *v){if(![JK60Saved() objectForKey:v])[JK60Saved() setObject:([TT50Saved() objectForKey:v]?:@[@(v.alpha),@(v.hidden),@(v.userInteractionEnabled)]) forKey:v];}
static void JK60BeforeDraw(UIView *v){if(!JK60Active()||!JK60Chrome(v)||JK62Hold(v))return;JK60Save(v);[UIView performWithoutAnimation:^{v.alpha=0;v.hidden=YES;v.userInteractionEnabled=NO;[v.layer removeAnimationForKey:@"opacity"];}];}
static void JK60Tree(UIView *root){if(!JK60Active()||!root)return;NSMutableArray *q=[NSMutableArray arrayWithObject:root];for(NSUInteger i=0;i<q.count&&i<8000;i++){UIView *v=q[i];JK60BeforeDraw(v);[q addObjectsFromArray:[v.subviews copy]];}}
static void JK60Restore(void){JK60Restoring=YES;for(UIView *v in JK60Saved().keyEnumerator.allObjects){NSArray *r=[JK60Saved() objectForKey:v];v.alpha=TT50Alpha(v,[r[0] doubleValue]);v.hidden=TT50Hidden(v,[r[1] boolValue]);v.userInteractionEnabled=TT50Interaction(v,[r[2] boolValue]);}[JK60Saved() removeAllObjects];JK60Restoring=NO;}
static BOOL JK60PaintVisible(UIView *v){NSArray *r=[JK60Saved() objectForKey:v];return r?([r[0] doubleValue]>.01&&![r[1] boolValue]):v.alpha>.01&&!v.hidden;}
