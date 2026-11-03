// Dots-tap state indication. Native controls never move in the model tree.
// Only keyed Core Animation transform/opacity tracks run on the compositor.
// No display link, per-frame traversal/layout, reparenting or copied controls.
static NSString *const JK62Key=@"tt62.collapseMotion";
static NSString *const JK62Track=@"joker62.visibility";
static BOOL JK62Running=NO;
static NSUInteger JK62Generation=0;
static NSTimer *JK62Completion;
static NSArray<NSString *> *JK62Names(void){return @[@"ظهور متتابع بنابض",@"تلاشي",@"انزلاق",@"تكبير بنابض",@"بدون حركة"];}
static NSInteger JK62Style(void){
    id n=[NSUserDefaults.standardUserDefaults objectForKey:JK62Key];
    if(![n isKindOfClass:NSNumber.class])return 0;
    double v=[n doubleValue];return isfinite(v)&&v==floor(v)&&v>=0&&v<5?(NSInteger)v:0;
}
static NSInteger JK62Effective(NSInteger style,BOOL reduce){return reduce&&style!=4?1:style;}
static NSArray *JK62Baseline(UIView *v){return [JK62Held() objectForKey:v]?:[JK60Saved() objectForKey:v]?:[TT50Saved() objectForKey:v]?:@[@(v.alpha),@(v.hidden),@(v.userInteractionEnabled)];}
static void JK62Cancel(void){
    ++JK62Generation;JK62Running=NO;[JK62Completion invalidate];JK62Completion=nil;
    JK60Restoring=YES;JK62Writing=YES;
    for(UIView *v in JK62Held().keyEnumerator.allObjects){
        NSArray *r=[JK62Held() objectForKey:v];
        [v.layer removeAnimationForKey:JK62Track];
        v.alpha=TT50Alpha(v,[r[0] doubleValue]);v.hidden=TT50Hidden(v,[r[1] boolValue]);
        v.userInteractionEnabled=TT50Interaction(v,[r[2] boolValue]);
    }
    [JK62Held() removeAllObjects];[JK62Parents() removeAllObjects];JK62Writing=NO;JK60Restoring=NO;
}
static void JK62Finish(void){
    JK62Cancel();
    if(JK60Active()){
        // These weak records cover every involved window, including an outgoing
        // reused cell. New views already received the regular synchronous guards.
        for(UIView *v in JK60Saved().keyEnumerator.allObjects)JK60BeforeDraw(v);
    }else JK60Restore();
}
static void JK62SetStyle(NSInteger style){
    JK62Finish();
    if(style<=0||style>4)[NSUserDefaults.standardUserDefaults removeObjectForKey:JK62Key];
    else [NSUserDefaults.standardUserDefaults setInteger:style forKey:JK62Key];
}
// Rail first, then all remaining chrome. Reverse this timeline on exit.
static CFTimeInterval JK62Delay(BOOL closed,BOOL rail,NSUInteger index,NSUInteger count){
    if(rail)return closed?.22+(count-1-index)*.045:index*.045;
    return closed?0:(count?count-1:0)*.045+.41;
}
static CATransform3D JK62Off(CATransform3D base,NSInteger style,BOOL rail){
    if(style==0||style==3)return CATransform3DScale(base,.9,.9,1);
    if(style==2)return CATransform3DTranslate(base,rail?18:0,rail?0:12,0);
    return base;
}
static void JK62Animate(UIWindow *w){
    if(!w)return;
    static dispatch_once_t observer;dispatch_once(&observer,^{
        [NSNotificationCenter.defaultCenter addObserverForName:UIAccessibilityReduceMotionStatusDidChangeNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){JK62Finish();}];
    });
    NSInteger style=JK62Effective(JK62Style(),UIAccessibilityIsReduceMotionEnabled());
    // Capture presentation values BEFORE cancelling an interrupted sequence.
    NSMapTable *from=[NSMapTable weakToStrongObjectsMapTable];
    for(UIView *v in JK62Held().keyEnumerator.allObjects){
        CALayer *shown=v.layer.presentationLayer;
        if([v.layer animationForKey:JK62Track]) [from setObject:@[@(shown?shown.opacity:v.layer.opacity),[NSValue valueWithCATransform3D:shown?shown.transform:v.layer.transform]] forKey:v];
    }
    JK62Cancel();
    // Release the old collapse snapshot without ever presenting an intermediate
    // frame. TT50 continues to own eye-hidden state independently.
    JK60Restore();
    if(style==4){if(JK60Active())JK60Tree(w);return;}
    NSMutableArray<UIView *> *queue=[NSMutableArray arrayWithObject:w],*chrome=[NSMutableArray new];
    for(NSUInteger i=0;i<queue.count&&i<8000;i++){
        UIView *v=queue[i];if(JK60Chrome(v))[chrome addObject:v];
        [queue addObjectsFromArray:[v.subviews copy]];
    }
    UIView *rail=JKRailStack(w);
    NSMutableArray<UIView *> *roots=[NSMutableArray new],*buttons=[NSMutableArray new];
    NSHashTable *set=[NSHashTable weakObjectsHashTable];for(UIView *v in chrome)[set addObject:v];
    // Hold all known chrome so nested labels/avatars don't disappear halfway
    // through their parent's exit. An arriving reused cell is deliberately NOT held.
    for(UIView *v in chrome){JK60Save(v);[JK62Held() setObject:JK62Baseline(v) forKey:v];if(v.superview)[JK62Parents() setObject:v.superview forKey:v];}
    for(UIView *v in chrome){
        BOOL nested=NO;for(UIView *a=v.superview;a&&a!=w;a=a.superview)if([set containsObject:a]){nested=YES;break;}
        if(!nested&&v!=rail)[roots addObject:v];
    }
    if(rail && [set containsObject:rail]){
        for(UIView *v in [rail.subviews copy]){
            // Direct action bundles include counts/follow/music decoration.
            // Do not animate internal glass layers independently.
            if(v.bounds.size.width>0&&v.bounds.size.height>0){[buttons addObject:v];if(!JK62Hold(v))[JK62Held() setObject:JK62Baseline(v) forKey:v];if(v.superview)[JK62Parents() setObject:v.superview forKey:v];}
        }
        [buttons sortUsingComparator:^NSComparisonResult(UIView *a,UIView *b){
            CGFloat ay=a.center.y,by=b.center.y;return ay<by?NSOrderedAscending:ay>by?NSOrderedDescending:NSOrderedSame;
        }];
    }
    JK62Running=YES;NSUInteger generation=JK62Generation;
    // Disable real input throughout exit and delayed entry. Exact native state
    // is restored at settlement. Do not alter targets, recognizers or geometry.
    for(UIView *v in JK62Held().keyEnumerator.allObjects)v.userInteractionEnabled=NO;
    NSMutableArray *all=[NSMutableArray arrayWithArray:buttons];[all addObjectsFromArray:roots];
    CFTimeInterval total=.2,now=CACurrentMediaTime();BOOL closed=JKRailClosed;
    for(NSUInteger i=0;i<all.count;i++){
        UIView *v=all[i];NSArray *r=[JK62Held() objectForKey:v];
        if([r[1] boolValue]||[r[0] doubleValue]<=.01||v.hidden||v.alpha<=.01)continue;
        BOOL isRail=i<buttons.count;
        CFTimeInterval delay=style==0?JK62Delay(closed,isRail,i,buttons.count):0;
#ifdef JK62_FIXTURE
        fprintf(stderr,"62 track style=%ld rail=%d closed=%d index=%lu count=%lu delay=%.3f stack=%p view=%p\n",(long)style,isRail,closed,(unsigned long)i,(unsigned long)buttons.count,delay,rail,v);fflush(stderr);
#endif
        CFTimeInterval duration=(style==0&&isRail)||style==3?.38:.2;
        total=MAX(total,delay+duration);
        CATransform3D base=v.layer.transform,off=JK62Off(base,style,isRail);
        NSArray *prior=[from objectForKey:v];
        id startTx=prior?prior[1]:[NSValue valueWithCATransform3D:closed?base:off];
        id endTx=[NSValue valueWithCATransform3D:closed?off:base];
        NSNumber *startAlpha=prior?prior[0]:(closed?@([r[0] doubleValue]):@0);
        NSNumber *endAlpha=closed?@0:@([r[0] doubleValue]);
        CABasicAnimation *fade=[CABasicAnimation animationWithKeyPath:@"opacity"];
        fade.fromValue=startAlpha;fade.toValue=endAlpha;fade.duration=MIN(duration,.2);
        fade.fillMode=kCAFillModeBoth;fade.removedOnCompletion=NO;
        NSMutableArray *tracks=[NSMutableArray arrayWithObject:fade];
        if(style!=1){
            CABasicAnimation *tx;
            if((style==0&&isRail)||style==3){
                CASpringAnimation *spring=[CASpringAnimation animationWithKeyPath:@"transform"];
                spring.mass=1;spring.stiffness=240;spring.damping=22;spring.initialVelocity=0;tx=spring;
            }else tx=[CABasicAnimation animationWithKeyPath:@"transform"];
            // Non-rail chrome in the default sequence fades + slides.
            if(style==0&&!isRail){
                off=CATransform3DTranslate(base,0,12,0);
                startTx=prior?prior[1]:[NSValue valueWithCATransform3D:closed?base:off];
                endTx=[NSValue valueWithCATransform3D:closed?off:base];
            }
            tx.fromValue=startTx;tx.toValue=endTx;tx.duration=duration;[tracks addObject:tx];
        }
        CAAnimationGroup *group=[CAAnimationGroup animation];group.animations=tracks;
        group.duration=duration;// Delay belongs to the receiving layer's local timeline. Converting
        // (now + delay) loses the delay under a paused UIKit ancestor.
        group.beginTime=[v.layer convertTime:now fromLayer:nil]+delay;
        group.timingFunction=[CAMediaTimingFunction functionWithControlPoints:.23 :1 :.32 :1];
        group.fillMode=kCAFillModeBoth;group.removedOnCompletion=NO;
        [v.layer addAnimation:group forKey:JK62Track];
#ifdef JK62_FIXTURE
        fprintf(stderr,"62 timing now=%.5f requested=%.5f retrieved=%.5f global=%.5f speed=%.1f begin=%.5f offset=%.5f\n",now,group.beginTime,[v.layer animationForKey:JK62Track].beginTime,[v.layer convertTime:[v.layer animationForKey:JK62Track].beginTime toLayer:nil],v.layer.speed,v.layer.beginTime,v.layer.timeOffset);fflush(stderr);
#endif
    }
    // Hidden flags are committed only after the last reverse-stagger exit.
    // Generation invalidates stale completion blocks on rapid taps/style changes.
    JK62Completion=[NSTimer timerWithTimeInterval:total repeats:NO block:^(NSTimer *timer){
        if(JK62Running&&generation==JK62Generation)JK62Finish();
    }];
    [NSRunLoop.mainRunLoop addTimer:JK62Completion forMode:NSRunLoopCommonModes];
}
