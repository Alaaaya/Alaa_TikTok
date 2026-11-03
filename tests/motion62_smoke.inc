// Same production module, UIKit Arabic/LTR fixture. No private target guesses.
{
    banner.text=@"Joker 62 · simulator TEST DATA\nDots motion, native actions preserved";
    JKRailClosed=NO;JK62Finish();JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];
    NSMutableArray *motionChrome=[NSMutableArray arrayWithObjects:fr,dock,caption,author,avatar,music,nil];
    UIView *dim=[[Fixture(@"TikTokFeedTabControl",UIView.class) alloc] initWithFrame:CGRectMake(100,70,170,30)];dim.alpha=.37;
    dim.transform=CGAffineTransformMakeRotation(.07);dim.userInteractionEnabled=NO;[fd addSubview:dim];[motionChrome addObject:dim];
    UIView *alreadyHidden=[[Fixture(@"AWELiveFeedEntranceView",UIView.class) alloc] initWithFrame:CGRectMake(10,70,40,30)];alreadyHidden.hidden=YES;alreadyHidden.alpha=.42;[fd addSubview:alreadyHidden];[motionChrome addObject:alreadyHidden];
    NSMutableArray *saved=[NSMutableArray new];
    for(UIView *v in motionChrome)[saved addObject:@[@(v.alpha),@(v.hidden),@(v.userInteractionEnabled),[NSValue valueWithCGAffineTransform:v.transform]]];
    void(^assertOriginal)(void)=^{
        for(NSUInteger i=0;i<motionChrome.count;i++){UIView *v=motionChrome[i];NSArray *r=saved[i];
            Check(fabs(v.alpha-[r[0] doubleValue])<.001&&v.hidden==[r[1] boolValue]&&v.userInteractionEnabled==[r[2] boolValue]&&CGAffineTransformEqualToTransform(v.transform,[r[3] CGAffineTransformValue]),@"62 EXACT original visibility alpha input and native transform");
        }
    };
    void(^assertClosed)(void)=^{for(UIView *v in motionChrome)Check(v.hidden&&v.alpha==0&&!v.userInteractionEnabled,@"62 settled exit hides ALL chrome without invisible input");};
    void(^waitMotion)(void)=^{
        NSDate *end=[NSDate dateWithTimeIntervalSinceNow:1.5];
        while(JK62Running&&end.timeIntervalSinceNow>0)[NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:.02]];
        Check(!JK62Running&&JK62Held().count==0,@"62 scheduled completion settles and clears weak records");
    };
    [D() removeObjectForKey:JK62Key];Check(JK62Style()==0,@"62 default stagger spring");
    for(id unknown in @[@(-1),@99,@1.5,@"fade",@{},@[ @1 ]]){[D() setObject:unknown forKey:JK62Key];Check(JK62Style()==0,@"62 unknown stored type/value falls back to default");}
    for(NSInteger style=0;style<5;style++){
        fprintf(stderr,"62 exercise style %ld\n",(long)style);fflush(stderr);
        JK62SetStyle(style);Check(JK62Style()==style,@"62 five choices persist/read back");
        Check(JK62Effective(style,YES)==(style==4?4:1),@"62 Reduce Motion uses fade or none for every style");
        dots.dragged=NO;[dots flip];
        if(style!=4){
            Check(JK62Running&&!fr.hidden&&JK60Alpha(fr,1)==1&&!JK60Hidden(fr,NO)&&!JK60Interaction(fr,YES),@"62 outgoing native chrome remains visible until exit ends, interaction guarded");
            if(style==0){
                CAAnimationGroup *first=[((UIView *)fr.subviews.firstObject).layer animationForKey:JK62Track];
                CAAnimationGroup *last=[((UIView *)fr.subviews.lastObject).layer animationForKey:JK62Track];
                CAAnimationGroup *dockExit=[dock.layer animationForKey:JK62Track];
                Check(first&&last&&dockExit&&first.beginTime>=last.beginTime&&last.beginTime>dockExit.beginTime,@"62 reverse exit dock/tabs/caption first, rail bottom to top");
            }
            UIView *fresh=[[Fixture(@"TTKFeedBaseBottomBarView",UIView.class) alloc] initWithFrame:CGRectMake(10,100,80,30)];[fd addSubview:fresh];JK60BeforeDraw(fresh);
            Check(fresh.hidden&&!fresh.userInteractionEnabled&&fresh.alpha==0,@"62 arriving/reused cell is NOT outgoing-held and cannot flash");[fresh removeFromSuperview];
            waitMotion();
        }
        assertClosed();ShotView(self.window,[NSString stringWithFormat:@"62-style-%ld-collapsed",(long)style]);
        [dots flip];
        if(style!=4){
            Check(!fr.hidden&&!fr.userInteractionEnabled,@"62 expanding real controls stay noninteractive until sequence settles");
            if(style==0){
                CAAnimationGroup *first=[((UIView *)fr.subviews.firstObject).layer animationForKey:JK62Track];
                CAAnimationGroup *last=[((UIView *)fr.subviews.lastObject).layer animationForKey:JK62Track];
                CAAnimationGroup *dockEntry=[dock.layer animationForKey:JK62Track];
                Check(first&&last&&dockEntry&&first.beginTime<=last.beginTime&&dockEntry.beginTime>=last.beginTime+last.duration,@"62 entry rail top to bottom spring then dock tabs caption fade-slide");
            }
            for(UIView *v in JK62Held())if([v.layer animationForKey:JK62Track]){
                CAAnimationGroup *g=(id)[v.layer animationForKey:JK62Track];
                for(CAPropertyAnimation *track in g.animations)Check([track.keyPath isEqual:@"opacity"]||[track.keyPath isEqual:@"transform"],@"62 compositor-only animated properties");
            }
            waitMotion();
        }
        assertOriginal();ShotView(self.window,[NSString stringWithFormat:@"62-style-%ld-expanded",(long)style]);
        // Same-turn interruption and stale completion cancellation, including NONE.
        for(NSUInteger i=0;i<15;i++)[dots flip];
        JK62Finish();assertClosed();[dots flip];JK62Finish();assertOriginal();
        Check(!JK62Running&&JK62Held().count==0,@"62 rapid odd/even taps no stuck alpha or cumulative transforms");
    }
    JK62SetStyle(0);
    [dots flip];waitMotion();[dots flip];
    NSDate *half=[NSDate dateWithTimeIntervalSinceNow:.09];while(half.timeIntervalSinceNow>0)[NSRunLoop.mainRunLoop runMode:NSDefaultRunLoopMode beforeDate:half];
    UIView *sample=fr.subviews.firstObject;CALayer *presentation=sample.layer.presentationLayer;
    float opacity=presentation?presentation.opacity:sample.layer.opacity;
    CATransform3D tx=presentation?presentation.transform:sample.layer.transform;
    [dots flip];CAAnimationGroup *reverse=(id)[sample.layer animationForKey:JK62Track];
    if(reverse){
        CABasicAnimation *fade=reverse.animations.firstObject;
        Check(fabs([fade.fromValue doubleValue]-opacity)<.1,@"62 midflight reverse captures presentation opacity");
        CABasicAnimation *transform=reverse.animations.count>1?reverse.animations[1]:nil;
        Check(!transform||fabs([transform.fromValue CATransform3DValue].m11-tx.m11)<.1,@"62 reversal uses presentation transform, never cumulative model transforms");
    }
    [NSNotificationCenter.defaultCenter postNotificationName:UIAccessibilityReduceMotionStatusDidChangeNotification object:nil];Check(!JK62Running,@"62 accessibility motion preference notification settles in-flight motion");
    JK62SetStyle(1);Check(!JK62Running,@"62 changing style settles an in-flight sequence");assertClosed();
    JKRailClosed=NO;JKRailApply(self.window,YES);TT50EyeActive=YES;TT50Tree(fd);JK62Finish();
    Check(fr.hidden&&caption.hidden,@"62 eye ON during entry wins over animation/restore");TT50Restore();assertOriginal();
    TT50EyeActive=YES;TT50Tree(fd);JKRailClosed=YES;JKRailApply(self.window,YES);TT50Restore();JK62Finish();assertClosed();
    JKRailClosed=NO;JKRailApply(self.window,YES);JK62Finish();assertOriginal();
    // Reparent an already-held native caption to a reused feed cell mid-exit.
    JKRailClosed=YES;JKRailApply(self.window,YES);
    UIView *reuseRoot=[UIView new];[fd addSubview:reuseRoot];
    [reuseRoot addSubview:dim];JK60BeforeDraw(dim);
    Check(!JK62Hold(dim)&&dim.hidden&&dim.alpha==0&&!dim.userInteractionEnabled&&![dim.layer animationForKey:JK62Track],@"62 reused/reparented outgoing chrome loses hold synchronously without flash");
    [fd addSubview:dim];[reuseRoot removeFromSuperview];JK62Finish();assertClosed();
    JKRailClosed=NO;JKRailApply(self.window,YES);JK62Finish();assertOriginal();
    // Rotation during sequence preserves the native model transform and drag pos.
    NSDictionary *position=[[D() dictionaryForKey:@"tt122.position.9156"] copy];CGRect portrait=self.window.frame;
    JKRailClosed=YES;JKRailApply(self.window,YES);self.window.frame=CGRectMake(0,0,portrait.size.height,portrait.size.width);[dots placeFloating];JK62Finish();assertClosed();
    self.window.frame=portrait;[dots placeFloating];JKRailClosed=NO;JKRailApply(self.window,YES);JK62Finish();assertOriginal();
    Check([[D() dictionaryForKey:@"tt122.position.9156"] isEqual:position],@"62 motion/rotation never overwrite dots drag persistence");
    // Reuse existing drag fixture while animated, no accidental toggle.
    JK60TestPan *motionPan=[JK60TestPan new];dots.dragged=NO;
    motionPan.fixtureState=UIGestureRecognizerStateBegan;motionPan.fixtureDelta=CGPointZero;[dots drag:motionPan];
    motionPan.fixtureState=UIGestureRecognizerStateChanged;motionPan.fixtureDelta=CGPointMake(-40,-20);[dots drag:motionPan];
    motionPan.fixtureState=UIGestureRecognizerStateEnded;[dots drag:motionPan];[dots flip];Check(!JKRailClosed,@"62 dots drag never triggers collapse animation");dots.dragged=NO;
    // Default style switch while animating must release snapshots and action state.
    JKRailClosed=YES;JKRailApply(self.window,YES);[D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();
    Check(!JK62Running&&JK62Held().count==0&&JK60Saved().count==0&&!fr.hidden,@"62 switching to native midflight restores before removing dots");
    for(NSUInteger i=0;i<tabs.count;i++)Check([((UIView *)tabs[i]).gestureRecognizers containsObject:tabGRs[i]],@"62 native gesture identity survives every animation");
    [D() setInteger:3 forKey:JKStyleKey];JKRailClosed=NO;JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];
    [dim removeFromSuperview];[alreadyHidden removeFromSuperview];JK62SetStyle(4);
    NSLog(@"PASS 62 UIKit all five styles, reverse stagger, delayed hidden, scheduled completion, exact transforms, presentation reversal, rapid taps, eye both orders, fresh cell, rotation, drag, native cleanup and gesture identity");
}
