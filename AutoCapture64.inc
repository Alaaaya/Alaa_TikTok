// Opt-in, structure-only, finite UIKit event capture. No native UI mutation.
#define JK64_SESSION 45
#define JK64_TTL 600
#define JK64_SCAN_BUDGET 12
#define JK64_VISITS 1800
static BOOL JK64Armed,JK64Pending;
static NSUInteger JK64Generation,JK64Scans,JK64Events;
static NSTimeInterval JK64Deadline,JK64LastTime;
static NSString *JK64Last,*JK64Filter=@"not-armed",*JK64Trigger=@"none";
static NSInteger JK64Best;
static NSHashTable *JK64Baseline;
static NSTimeInterval JK64Now(void){return NSProcessInfo.processInfo.systemUptime;}
static void JK64Expire(void){
    if(JK64Last && (JK64Now()-JK64LastTime<0 || JK64Now()-JK64LastTime>JK64_TTL)){JK64Last=nil;JK64LastTime=0;JK64Best=0;}
}
static BOOL JK64Active(void){
    if(!NSThread.isMainThread || !JK64Armed)return NO;
    if(JK64Now()>JK64Deadline || JK64Scans>=JK64_SCAN_BUDGET){JK64Armed=NO;JK64Filter=@"session-ended";JK64Baseline=nil;return NO;}
    return YES;
}
static BOOL JK64IgnoreController(UIViewController *c){
    NSString *n=NSStringFromClass(c.class);
    return [n hasPrefix:@"TTGSettings"] || [c isKindOfClass:UIActivityViewController.class] ||
        [c isKindOfClass:UIAlertController.class] || [c isKindOfClass:UIColorPickerViewController.class] ||
        [c isKindOfClass:UIDocumentPickerViewController.class] || [c isKindOfClass:UIImagePickerController.class];
}
static NSArray<UIView *> *JK64Roots(NSHashTable *excluded,NSHashTable *modal){
    NSMutableArray *roots=[NSMutableArray new];NSUInteger controllers=0;
    for(UIWindow *w in UIApplication.sharedApplication.windows){
        if(w.hidden || w.alpha<.01 || w.windowLevel!=UIWindowLevelNormal)continue;
        if(roots.count>=16)break;
        [roots addObject:w];NSMutableArray *q=[NSMutableArray new];if(w.rootViewController)[q addObject:w.rootViewController];
        NSHashTable *seen=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
        for(NSUInteger i=0;i<q.count&&controllers<96;i++){
            UIViewController *c=q[i];if([seen containsObject:c])continue;[seen addObject:c];controllers++;
            if(JK64IgnoreController(c)){if(c.isViewLoaded)[excluded addObject:c.view];continue;}
            if(c.presentingViewController && c.isViewLoaded)[modal addObject:c.view];
            if(c.presentedViewController&&q.count<96)[q addObject:c.presentedViewController];
            for(UIViewController *child in c.childViewControllers){if(q.count>=96)break;[q addObject:child];}
        }
    }
    return roots;
}
// Single bounded BFS. Weak baseline holds identities only, never the native views.
static NSArray<UIView *> *JK64Views(NSHashTable *excluded,NSHashTable *modal,BOOL *capped){
    NSMutableArray *q=[JK64Roots(excluded,modal) mutableCopy],*out=[NSMutableArray new];
    for(NSUInteger i=0;i<q.count&&i<JK64_VISITS;i++){
        UIView *v=q[i];if([excluded containsObject:v]||JK63Own(v)||v.hidden||v.alpha<.01)continue;[out addObject:v];
        for(UIView *child in v.subviews){if(q.count>=JK64_VISITS){*capped=YES;break;}[q addObject:child];}
    }
    return out;
}
static void JK64Stop(void){JK64Armed=NO;JK64Pending=NO;JK64Baseline=nil;JK64Generation++;}
static void JK64Arm(void){
    if(!NSThread.isMainThread)return;
    JK64Stop();JK64Last=nil;JK64LastTime=0;JK64Best=0;JK64Scans=0;JK64Events=0;
    JK64Baseline=[NSHashTable weakObjectsHashTable];BOOL capped=NO;
    NSHashTable *ex=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality],*mo=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
    for(UIView *v in JK64Views(ex,mo,&capped)){
        BOOL visible=YES;UIView *a=v;for(NSUInteger d=0;a&&d<32;d++,a=a.superview)if(a.hidden||a.alpha<.05){visible=NO;break;}
        if(visible)[JK64Baseline addObject:v];
    }
    JK64Deadline=JK64Now()+JK64_SESSION;JK64Armed=YES;
    JK64Filter=capped?@"armed-baseline-capped":@"armed";
    NSUInteger gen=JK64Generation;
    // One expiry callback, not a poll. Clear memory even if no lifecycle event follows.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,JK64_SESSION*NSEC_PER_SEC),dispatch_get_main_queue(),^{
        if(gen==JK64Generation){JK64Stop();JK64Filter=@"session-expired";}
    });
}
static void JK64Scan(NSString *trigger){
    if(!JK64Active())return;JK64Scans++;JK64Trigger=trigger;
    NSHashTable *ex=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality],*modal=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
    BOOL capped=NO;NSArray<UIView *> *views=JK64Views(ex,modal,&capped);
    // Summaries propagated in reverse BFS, O(nodes * bounded ancestor depth), no per-candidate subtree walks.
    NSMapTable *lists=[NSMapTable strongToStrongObjectsMapTable],*inputs=[NSMapTable strongToStrongObjectsMapTable];
    for(UIView *v in views.reverseObjectEnumerator){
        NSUInteger l=[[lists objectForKey:v] unsignedIntegerValue]+([v isKindOfClass:UITableView.class]||[v isKindOfClass:UICollectionView.class]||([v isKindOfClass:UIScrollView.class]&&![v isKindOfClass:UITextView.class])?1:0);
        NSUInteger t=[[inputs objectForKey:v] unsignedIntegerValue]+([v isKindOfClass:UITextField.class]||[v isKindOfClass:UITextView.class]?1:0);
        [lists setObject:@(l) forKey:v];[inputs setObject:@(t) forKey:v];
        if(v.superview){[lists setObject:@([[lists objectForKey:v.superview] unsignedIntegerValue]+l) forKey:v.superview];[inputs setObject:@([[inputs objectForKey:v.superview] unsignedIntegerValue]+t) forKey:v.superview];}
    }
    UIView *best=nil;NSInteger score=0;NSString *reason=@"no-new-sheet-structure";
    for(UIView *v in views){
        if([v isKindOfClass:UIWindow.class]||[JK64Baseline containsObject:v]||!v.window||v.hidden||v.alpha<.05)continue;
        BOOL visible=YES;UIView *ancestor=v;
        for(NSUInteger d=0;ancestor&&d<32;d++,ancestor=ancestor.superview){if(ancestor.hidden||ancestor.alpha<.05||[ex containsObject:ancestor]){visible=NO;break;}}
        if(!visible)continue;
        CGRect r=[v convertRect:v.bounds toView:v.window];CGSize w=v.window.bounds.size;
        if(w.width<=0||w.height<=0)continue;
        CGFloat ratio=r.size.height/w.height;
        BOOL partial=r.size.width>=w.width*.65&&ratio>=.32&&ratio<=.96&&CGRectGetMaxY(r)>=w.height*.75;
        BOOL presented=[modal containsObject:v]&&r.size.width>=w.width*.65&&ratio>=.32;
        if(!partial&&!presented)continue;
        NSUInteger l=[[lists objectForKey:v] unsignedIntegerValue],t=[[inputs objectForKey:v] unsignedIntegerValue];
        BOOL name=[NSStringFromClass(v.class).lowercaseString containsString:@"comment"];
        // Even a comment-named feed action or 56px emoji strip cannot qualify.
        // Unknown class requires both list and composer; presented root can be full screen.
        if(!l || (!t&&!name))continue;
        NSInteger rank=(t?100:0)+(name?40:0)+(presented?20:0)+(partial?10:0);
        if(rank>score){score=rank;best=v;reason=name?@"new-comment-name-plus-list-sheet":@"new-generic-list-composer-sheet";}
    }
    if(best && score>JK64Best){
        NSMutableString *out=[NSMutableString stringWithFormat:@"=== Joker 64 retained sheet candidate ===\ntrigger=%@ filter=%@ score=%ld discoveryVisits=%lu scan=%lu/%d discoveryCapped=%d nodeLimit=%d depthLimit=%d byteLimit=%d queueLimit=%d controllerLimit=96 windowLimit=16 windowControllerBoundsMayApply=1\nDiscovery is structural, NOT verified comment-sheet identity. No content is read.\n",JK63Symbol(trigger),reason,(long)score,(unsigned long)views.count,(unsigned long)JK64Scans,JK64_SCAN_BUDGET,capped,JK_DUMP_NODES,JK_DUMP_DEPTH,JK_DUMP_BYTES,JK_DUMP_QUEUE];
        NSUInteger count=0;NSHashTable *seen=[NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
        JK63Tree(best,out,seen,ex,&count);
        JK63Line(out,[NSString stringWithFormat:@"end nodes=%lu nodeCapReached=%d byteCapNear=%d depthAndQueueMayBeBounded=1",(unsigned long)count,count>=JK_DUMP_NODES,out.length>=JK_DUMP_BYTES-500]);
        JK64Last=[out copy];JK64LastTime=JK64Now();JK64Best=score;JK64Filter=@"sheet-retained";
        // Retain only one best snapshot. Bounded eligibility timer, no history or files until share.
        NSTimeInterval retainedTime=JK64LastTime;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(JK64_TTL+1)*NSEC_PER_SEC),dispatch_get_main_queue(),^{if(JK64LastTime==retainedTime)JK64Expire();});
    } else JK64Filter=best?@"lower-or-equal-candidate-not-replaced":reason;
}
static void JK64Event(NSString *trigger){
    if(!JK64Active()||JK64Pending||JK64Events>=24)return;
    JK64Events++;JK64Pending=YES;NSUInteger gen=JK64Generation;
    // A single coalesced settle callback, and one finite later retry per event.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.25*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        if(gen!=JK64Generation)return;JK64Pending=NO;JK64Scan(trigger);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(.65*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
            if(gen==JK64Generation)JK64Scan(@"settle_retry");
        });
    });
}
static void JK64Export(NSString *path){
    if(!NSThread.isMainThread)return;JK64Expire();
    NSMutableString *out=[NSMutableString stringWithFormat:@"Joker 64 DIAGNOSTIC ONLY. No cards styling.\nNo text, input, accessibility strings, credentials, images or uploads.\narmed=%d scans=%lu/%d events=%lu/24 lastFilter=%@ lastTrigger=%@\n",JK64Active(),(unsigned long)JK64Scans,JK64_SCAN_BUDGET,(unsigned long)JK64Events,JK64Filter,JK63Symbol(JK64Trigger)];
    if(JK64Last){[out appendFormat:@"retainedAgeSeconds=%.1f ttlSeconds=%d\n%@\n",JK64Now()-JK64LastTime,JK64_TTL,JK64Last];}
    else [out appendString:@"No retained sheet. Arm capture, open comments within 45 seconds, close comments, then reopen Joker and share within 10 minutes. Unknown sheets must contain a list and composer; discovery can miss custom renderers.\n"];
    [out writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
}
