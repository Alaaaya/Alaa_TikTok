// On-demand, bounded structure capture. Runs only when the user taps the diagnostics share row.
// Records class names, frames, states, targets and action selector names. Never records caption,
// author or other user text: label text is redacted to its length unless it is a static control word.
#define JK_DUMP_NODES 450
#define JK_DUMP_DEPTH 8
#define JK_DUMP_BYTES 90000
static BOOL JKDumpAllowed(NSString *t) {
    static NSSet *ok;static dispatch_once_t once;
    dispatch_once(&once,^{ok=[NSSet setWithArray:@[@"Follow",@"Following",@"Follow back",@"Friends",@"Home",@"Shop",@"Discover",@"Inbox",@"Profile",@"Friends",@"Reels",@"Messages",@"Search",@"For You",@"Live",@"LIVE",@"Report",@"Download",@"Not interested",@"متابعة",@"تتابعه",@"أتابعه",@"الرئيسية",@"الأصدقاء",@"الصندوق",@"الملف الشخصي",@"التسوق",@"اكتشف",@"بحث",@"لك",@"تتابع",@"المتابَعون",@"الأصدقاء"]];});
    return [ok containsObject:t];
}
static NSString *JKDumpText(NSString *t) {
    if(!t.length)return @"";
    return JKDumpAllowed(t)?[NSString stringWithFormat:@"\"%@\"",t]:[NSString stringWithFormat:@"<text len=%lu>",(unsigned long)t.length];
}
static NSString *JKDumpTargets(UIControl *c) {
    NSMutableArray *parts=[NSMutableArray new];
    for(id t in c.allTargets){
        NSMutableArray *sels=[NSMutableArray new];
        for(NSNumber *ev in @[@(UIControlEventTouchUpInside),@(UIControlEventTouchDown),@(UIControlEventValueChanged),@(UIControlEventPrimaryActionTriggered)]){
            for(NSString *s in [c actionsForTarget:(t==NSNull.null?nil:t) forControlEvent:(UIControlEvents)ev.unsignedIntegerValue])[sels addObject:[NSString stringWithFormat:@"%@@%@",s,ev]];
        }
        [parts addObject:[NSString stringWithFormat:@"%@{%@}",t==NSNull.null?@"nil":NSStringFromClass([t class]),[sels componentsJoinedByString:@","]]];
    }
    NSUInteger ga=c.gestureRecognizers.count;
    return [NSString stringWithFormat:@" targets=[%@] gestures=%lu",[parts componentsJoinedByString:@";"],(unsigned long)ga];
}
static void JKDumpView(UIView *v,NSUInteger depth,NSMutableString *out,NSUInteger *count) {
    if(*count>=JK_DUMP_NODES || out.length>JK_DUMP_BYTES || depth>JK_DUMP_DEPTH)return;
    NSString *name=NSStringFromClass(v.class);
    if([name hasPrefix:@"TTJoker"] || [name hasPrefix:@"TT122"] || [name hasPrefix:@"TT46"] || [name isEqual:@"JKStyleItem"])return;
    (*count)++;
    NSMutableString *line=[NSMutableString stringWithFormat:@"%@%@ frame=%@ hid=%d a=%.2f ui=%d",[@"" stringByPaddingToLength:depth*2 withString:@" " startingAtIndex:0],name,NSStringFromCGRect(v.frame),v.hidden,v.alpha,v.userInteractionEnabled];
    if([v isKindOfClass:UIControl.class]){UIControl *c=(UIControl *)v;[line appendFormat:@" sel=%d en=%d hl=%d%@",c.selected,c.enabled,c.highlighted,JKDumpTargets(c)];}
    if([v isKindOfClass:UILabel.class])[line appendFormat:@" %@",JKDumpText(((UILabel *)v).text)];
    if([v isKindOfClass:UIButton.class])[line appendFormat:@" title=%@",JKDumpText([(UIButton *)v titleForState:UIControlStateNormal])];
    if([v isKindOfClass:UIImageView.class])[line appendFormat:@" img=%d",((UIImageView *)v).image!=nil];
    if([v isKindOfClass:UITabBar.class]){UITabBar *b=(UITabBar *)v;[line appendFormat:@" items=%lu selIdx=%lu",(unsigned long)b.items.count,(unsigned long)(b.selectedItem?[b.items indexOfObject:b.selectedItem]:NSNotFound)];}
    if(v.accessibilityIdentifier.length && JKDumpAllowed(v.accessibilityIdentifier))[line appendFormat:@" id=%@",v.accessibilityIdentifier];
    [out appendFormat:@"%@\n",line];
    for(UIView *s in v.subviews)JKDumpView(s,depth+1,out,count);
}
static BOOL JKDumpRoot(UIView *v) {
    static NSSet *roots;static dispatch_once_t once;
    dispatch_once(&once,^{roots=[NSSet setWithArray:@[@"AWEPlayInteractionAuthorView",@"AWEPlayInteractionDescriptionLabel",@"AWEScrollLabelView",@"TTKFeedBaseBottomBarView",@"ACCCLACaptionLabel",@"TikTokFeedTabControl",@"TTKFeedTabCornerItemView",@"TTKFeedRightInteractionStackView",@"AWELiveFeedEntranceView"]];});
    NSString *n=NSStringFromClass(v.class);
    return [roots containsObject:n] || [v isKindOfClass:UITabBar.class] || ([n hasPrefix:@"TikTokFeedTabItem"]);
}
static NSString *JKHierarchyCapture(void) {
    if(![NSThread isMainThread])return @"capture skipped: not on main thread\n";
    NSMutableString *out=[NSMutableString stringWithFormat:@"\n=== Joker hierarchy capture (structure only, no user text) ===\n"];
    NSUInteger count=0,visited=0;
    for(UIWindow *w in UIApplication.sharedApplication.windows) {
        NSMutableArray<UIView *> *queue=[NSMutableArray arrayWithObject:w];
        for(NSUInteger i=0;i<queue.count && visited<4000;i++){
            UIView *v=queue[i];visited++;
            if(JKDumpRoot(v)){[out appendFormat:@"-- root in window %@ frame=%@\n",NSStringFromClass(w.class),NSStringFromCGRect(w.frame)];JKDumpView(v,0,out,&count);continue;}
            [queue addObjectsFromArray:v.subviews];
        }
    }
    [out appendFormat:@"=== end: %lu nodes ===\n",(unsigned long)count];
    if(count==0)[out appendString:@"no feed views found: open the For You feed first, then share again\n"];
    return out;
}
static void JKAppendCapture(NSString *path) {
    NSString *cap=JKHierarchyCapture();
    NSString *old=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil]?:@"";
    if(old.length>100000)old=[old substringFromIndex:old.length-100000];
    [[old stringByAppendingString:cap] writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
}
