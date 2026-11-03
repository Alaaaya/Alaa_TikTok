// 63 diagnostic only. Public UIKit metadata, no label/input/accessibility text reads.
// No automatic/timer capture, networking, styling, private-class binding or file reads.
#define JK_DUMP_NODES 900
#define JK_DUMP_DEPTH 18
#define JK_DUMP_BYTES 160000
#define JK_DUMP_SCAN 4000
#define JK_DUMP_QUEUE 2000
static NSString *JK63BeforeSettings;
static NSTimeInterval JK63BeforeTime;
static NSString *JK63Symbol(NSString *s) {
    if(!s.length || s.length>160)return @"<omitted>";
    static NSCharacterSet *bad;static dispatch_once_t once;
    dispatch_once(&once,^{bad=[[NSCharacterSet characterSetWithCharactersInString:@"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.$:"] invertedSet];});
    return [s rangeOfCharacterFromSet:bad].location==NSNotFound?s:@"<omitted>";
}
static void JK63Line(NSMutableString *out,NSString *line) {
    // All generated data is ASCII. Never truncate halfway through a symbol or record.
    if(out.length+line.length+1<=JK_DUMP_BYTES)[out appendFormat:@"%@\n",line];
}
static BOOL JK63Own(UIView *v) {
    NSString *n=NSStringFromClass(v.class);
    return [n hasPrefix:@"TTJoker"] || [n hasPrefix:@"TT122"] || [n hasPrefix:@"TT46"] || [n isEqual:@"JKStyleItem"];
}
static NSString *JKDumpTargets(UIControl *c) {
    NSMutableArray *parts=[NSMutableArray new];NSUInteger targets=0;
    for(id t in c.allTargets){
        if(targets++>=8)break;
        NSMutableArray *sels=[NSMutableArray new];
        for(NSNumber *ev in @[@(UIControlEventTouchUpInside),@(UIControlEventTouchDown),@(UIControlEventValueChanged),@(UIControlEventPrimaryActionTriggered)]){
            for(NSString *s in [c actionsForTarget:(t==NSNull.null?nil:t) forControlEvent:(UIControlEvents)ev.unsignedIntegerValue]){
                if(sels.count>=16)break;
                [sels addObject:[NSString stringWithFormat:@"%@@%@",JK63Symbol(s),ev]];
            }
        }
        [parts addObject:[NSString stringWithFormat:@"%@{%@}",t==NSNull.null?@"nil":JK63Symbol(NSStringFromClass([t class])),[sels componentsJoinedByString:@","]]];
    }
    return [NSString stringWithFormat:@" targets=[%@]",[parts componentsJoinedByString:@";"]];
}
static void JK63Tree(UIView *root,NSMutableString *out,NSHashTable *seen,NSHashTable *excluded,NSUInteger *count) {
    NSMutableArray<UIView *> *queue=[NSMutableArray arrayWithObject:root];
    NSMutableArray<NSNumber *> *parents=[NSMutableArray arrayWithObject:@0],*depths=[NSMutableArray arrayWithObject:@0];
    for(NSUInteger i=0;i<queue.count && *count<JK_DUMP_NODES && out.length<JK_DUMP_BYTES-300;i++){
        UIView *v=queue[i];NSUInteger depth=depths[i].unsignedIntegerValue;
        if([seen containsObject:v] || [excluded containsObject:v] || JK63Own(v) || depth>JK_DUMP_DEPTH)continue;
        [seen addObject:v];NSUInteger node=++(*count);
        NSMutableString *line=[NSMutableString stringWithFormat:@"node=%lu parent=%lu depth=%lu %@ frame=%@ bounds=%@ hidden=%d alpha=%.3f interaction=%d radius=%.2f border=%.2f constraints=%lu gestures=%lu",(unsigned long)node,(unsigned long)parents[i].unsignedIntegerValue,(unsigned long)depth,JK63Symbol(NSStringFromClass(v.class)),NSStringFromCGRect(v.frame),NSStringFromCGRect(v.bounds),v.hidden,v.alpha,v.userInteractionEnabled,v.layer.cornerRadius,v.layer.borderWidth,(unsigned long)v.constraints.count,(unsigned long)v.gestureRecognizers.count];
        if([v isKindOfClass:UIControl.class]){UIControl *c=(id)v;[line appendFormat:@" selected=%d enabled=%d highlighted=%d%@",c.selected,c.enabled,c.highlighted,JKDumpTargets(c)];}
        if([v isKindOfClass:UIScrollView.class]){UIScrollView *s=(id)v;[line appendFormat:@" contentSize=%@ offset=%@ dragging=%d",NSStringFromCGSize(s.contentSize),NSStringFromCGPoint(s.contentOffset),s.dragging];}
        if([v isKindOfClass:UITextField.class])[line appendFormat:@" secure=%d",((UITextField *)v).secureTextEntry];
        JK63Line(out,line);
        NSUInteger gestures=0;for(UIGestureRecognizer *g in v.gestureRecognizers){if(gestures++>=8)break;JK63Line(out,[NSString stringWithFormat:@" gesture node=%lu %@ state=%ld enabled=%d cancels=%d",(unsigned long)node,JK63Symbol(NSStringFromClass(g.class)),(long)g.state,g.enabled,g.cancelsTouchesInView]);}
        if(depth<JK_DUMP_DEPTH)for(UIView *s in v.subviews){if(queue.count>=JK_DUMP_QUEUE)break;[queue addObject:s];[parents addObject:@(node)];[depths addObject:@(depth+1)];}
    }
}
static NSString *JKHierarchyCapture(void) {
    if(!NSThread.isMainThread)return @"capture skipped: not on main thread\n";
    NSMutableString *out=[NSMutableString stringWithString:@"=== Joker 63 structure-only diagnostic ===\nNo text, usernames, input values, accessibility strings, images, URLs or credentials are read.\nComment-name candidates are discovery hints, NOT verified comment-sheet classes.\n"];
    NSArray<UIWindow *> *windows=UIApplication.sharedApplication.windows;
    NSHashTable *excluded=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
    NSUInteger controllers=0;
    // Current controller structure, no model/description/KVC inspection. Don't load views.
    for(UIWindow *w in windows.reverseObjectEnumerator){
        NSMutableArray *queue=[NSMutableArray new];if(w.rootViewController)[queue addObject:w.rootViewController];
        for(NSUInteger i=0;i<queue.count && controllers<96;i++){
            UIViewController *c=queue[i];controllers++;
            NSString *n=NSStringFromClass(c.class);
            if([n hasPrefix:@"TTGSettings"] || [n hasPrefix:@"UIActivity"]){if(c.isViewLoaded)[excluded addObject:c.view];continue;}
            JK63Line(out,[NSString stringWithFormat:@"controller=%@ loaded=%d presented=%d children=%lu",JK63Symbol(n),c.isViewLoaded,c.presentedViewController!=nil,(unsigned long)c.childViewControllers.count]);
            if(c.presentedViewController && queue.count<96)[queue addObject:c.presentedViewController];
            for(UIViewController *child in c.childViewControllers){if(queue.count>=96)break;[queue addObject:child];}
        }
    }
    // Bounded breadth-first discovery reaches deeply nested sheets without a feed-only allowlist.
    NSMutableArray<UIView *> *candidates=[NSMutableArray new];NSUInteger scanned=0;
    for(UIWindow *w in windows.reverseObjectEnumerator){
        NSMutableArray<UIView *> *queue=[NSMutableArray arrayWithObject:w];
        for(NSUInteger i=0;i<queue.count && scanned<JK_DUMP_SCAN;i++){
            UIView *v=queue[i];scanned++;
            if([excluded containsObject:v] || JK63Own(v))continue;
            NSString *n=NSStringFromClass(v.class).lowercaseString;
            if([n containsString:@"comment"] && candidates.count<48)[candidates addObject:v];
            for(UIView *s in v.subviews){if(queue.count>=JK_DUMP_QUEUE)break;[queue addObject:s];}
        }
    }
    JK63Line(out,[NSString stringWithFormat:@"candidateRoots=%lu scanned=%lu (name heuristic only)",(unsigned long)candidates.count,(unsigned long)scanned]);
    NSHashTable *seen=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];NSUInteger count=0;
    for(UIView *v in candidates){JK63Line(out,@"-- candidate subtree --");JK63Tree(v,out,seen,excluded,&count);}
    for(UIWindow *w in windows.reverseObjectEnumerator){JK63Line(out,@"-- window fallback subtree --");JK63Tree(w,out,seen,excluded,&count);}
    JK63Line(out,[NSString stringWithFormat:@"=== end nodes=%lu nodeLimit=%d depthLimit=%d scanLimit=%d byteLimit=%d; output may be bounded ===",(unsigned long)count,JK_DUMP_NODES,JK_DUMP_DEPTH,JK_DUMP_SCAN,JK_DUMP_BYTES]);
    return out;
}
static void JK63RememberBeforeSettings(void) {
    if(!NSThread.isMainThread)return;
    JK63BeforeSettings=JKHierarchyCapture();JK63BeforeTime=NSProcessInfo.processInfo.systemUptime;
}
static void JKAppendCapture(NSString *path) {
    if(!NSThread.isMainThread)return;
    NSMutableString *out=[NSMutableString stringWithString:@"Joker 63 DIAGNOSTIC ONLY. Cards style is not implemented or selectable.\n"];
    NSTimeInterval age=NSProcessInfo.processInfo.systemUptime-JK63BeforeTime;
    if(JK63BeforeSettings && age>=0 && age<=120){[out appendString:@"=== before Joker settings (less than 120 seconds ago) ===\n"];[out appendString:JK63BeforeSettings];}
    else [out appendString:@"No recent pre-settings snapshot. Keep comments open, reopen Joker settings, share within 120 seconds.\n"];
    JK63BeforeSettings=nil;JK63BeforeTime=0;
    [out appendString:@"=== at explicit diagnostics share ===\n"];[out appendString:JKHierarchyCapture()];
    // Fresh structure-only export. Never append arbitrary old log contents or include legacy text.
    [out writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
}
