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
        @"TTKFeedBaseBottomBarView",@"ACCCLACaptionLabel",@"JKStyleItem"] containsObject:name] ||
        ([name hasPrefix:@"TikTokFeedTabItem"] && [name hasSuffix:@"Control"]);
}
static void TT50Save(UIView *view) {
    if(![TT50Saved() objectForKey:view])
        [TT50Saved() setObject:@[@(view.alpha),@(view.hidden),@(view.userInteractionEnabled)] forKey:view];
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
            view.alpha=[state[0] doubleValue];view.hidden=[state[1] boolValue];
            view.userInteractionEnabled=[state[2] boolValue];
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
