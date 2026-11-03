// Real UIKit actions, nested native labels and real AVPlayer public state.
{
    [tab removeFromSuperview];
    banner.text=@"Joker 59 simulator TEST DATA\nNative controls fixture, NOT real TikTok";
    caption.attributedText=[[NSAttributedString alloc] initWithString:@"وجه منحوت بالمطرقة! ترند جديد\nيعود رغم كل التحذيرات #ترند #عربي" attributes:@{NSParagraphStyleAttributeName:rtl,NSForegroundColorAttributeName:UIColor.whiteColor,NSFontAttributeName:[UIFont systemFontOfSize:14]}];
    CGFloat w=self.window.bounds.size.width,h=self.window.bounds.size.height;
    UIView *dock=[[Fixture(@"TTKTabBar",UIView.class) alloc] initWithFrame:CGRectMake(0,h-83,w,83)];[fd addSubview:dock];
    UIView *blur=[[Fixture(@"TTKTabBarBlurView",UIView.class) alloc] initWithFrame:dock.bounds];blur.backgroundColor=UIColor.blackColor;[dock addSubview:blur];
    NSMutableArray *tabs=[NSMutableArray new],*tabGRs=[NSMutableArray new];
    NSArray *titles=@[@"الرئيسية",@"الأصدقاء",@"",@"الوارد",@"الملف الشخصي"],*icons=@[@"house",@"person.2",@"plus",@"tray",@"person"];
    NSObject *counter=[NSObject new]; // native tap target tested through addAction, not forwarded overlays
    __block NSInteger navigations=0;
    for(NSUInteger i=0;i<5;i++){
        Class c=Fixture(i==2?@"AWETabBarPlusButton":@"TTKTabBarButton",UIButton.class);
        UIButton *b=[[c alloc] initWithFrame:CGRectMake(i*w/5,0,w/5,49)];b.selected=NO;
        b.accessibilityLabel=titles[i];
        [b addAction:[UIAction actionWithHandler:^(__unused UIAction *action){navigations++;}] forControlEvents:i==2?UIControlEventTouchUpInside:UIControlEventTouchDown];
        UITapGestureRecognizer *g=[[UITapGestureRecognizer alloc] init];[b addGestureRecognizer:g];[tabGRs addObject:g];
        UIView *content=[[UIView alloc] initWithFrame:b.bounds];content.userInteractionEnabled=NO;[b addSubview:content];
        UIImageView *im=[[UIImageView alloc] initWithImage:[UIImage systemImageNamed:icons[i]]];im.frame=CGRectMake(25,0,30,30);[content addSubview:im];
        UILabel *l=[[UILabel alloc] initWithFrame:CGRectMake(0,32,w/5,13)];l.text=titles[i];l.font=[UIFont systemFontOfSize:10];l.textColor=UIColor.whiteColor;[content addSubview:l];
        [dock addSubview:b];[tabs addObject:b];
    }
    (void)counter;
    UITabBar *fake=[[Fixture(@"TTKFakeTabBar",UITabBar.class) alloc] initWithFrame:dock.frame];fake.userInteractionEnabled=NO;
    NSMutableArray *fakeItems=[NSMutableArray new];for(NSUInteger i=0;i<4;i++)[fakeItems addObject:[[UITabBarItem alloc] initWithTitle:titles[i<2?i:i+1] image:nil tag:i]];fake.items=fakeItems;fake.selectedItem=fakeItems[0];[fd insertSubview:fake belowSubview:dock];
    CGRect dockBefore=dock.frame;NSArray *buttonFrames=[tabs valueForKey:@"frame"];
    Class playerCls=Fixture(@"TTPlayerView",UIView.class);
    UIView *pv=[[playerCls alloc] initWithFrame:fd.bounds];[fd insertSubview:pv atIndex:0];
    UIView *off=[[playerCls alloc] initWithFrame:CGRectOffset(fd.bounds,0,-h)];[fd insertSubview:off atIndex:0];
    NSURL *url=[NSBundle.mainBundle URLForResource:@"player-fixture" withExtension:@"mp4"];
    Check(url!=nil,@"59 bundled local playable media fixture");
    AVPlayer *active=[AVPlayer playerWithURL:url],*inactive=[AVPlayer playerWithURL:url];active.automaticallyWaitsToMinimizeStalling=NO;inactive.automaticallyWaitsToMinimizeStalling=NO;
    AVPlayerLayer *pl=[AVPlayerLayer playerLayerWithPlayer:active];pl.frame=pv.bounds;[pv.layer addSublayer:pl];
    AVPlayerLayer *ol=[AVPlayerLayer playerLayerWithPlayer:inactive];ol.frame=off.bounds;[off.layer addSublayer:ol];
    NSDate *limit=[NSDate dateWithTimeIntervalSinceNow:8];
    while((active.currentItem.status!=AVPlayerItemStatusReadyToPlay || inactive.currentItem.status!=AVPlayerItemStatusReadyToPlay) && limit.timeIntervalSinceNow>0)[NSRunLoop.mainRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:.02]];
    [active playImmediatelyAtRate:1];inactive.muted=NO;
    Check(active.status==AVPlayerStatusReadyToPlay && active.rate==1 && inactive.rate==0,@"59 real AVPlayer fixture ready and playing; offscreen paused");
    JKRailClosed=NO;[D() setInteger:3 forKey:JKStyleKey];JKStyleApplyAll();
    Check(JK59ActivePlayer(self.window)==active,@"59 active player from visible TTPlayerView AVPlayerLayer only");
    CGRect dr=[dock convertRect:dock.bounds toView:self.window];
    Check(CGRectEqualToRect(dr,CGRectMake(16,h-74,w-32,62)) && dock.layer.cornerRadius==31,@"59 native dock exact reference x16 bottom12 height62 radius31");
    for(NSUInteger i=0;i<5;i++){
        UIButton *b=tabs[i];Check(b.superview==dock && [b.gestureRecognizers containsObject:tabGRs[i]] && b.userInteractionEnabled && !b.hidden && b.alpha==1,@"59 dock keeps live native navigation buttons and gestures");
        [b sendActionsForControlEvents:i==2?UIControlEventTouchUpInside:UIControlEventTouchDown];
        Check([b hitTest:CGPointMake(b.bounds.size.width/2,20) withEvent:nil]==b,@"59 native tab hit test is not intercepted by inert decoration");
        JK59DockRecord *r=objc_getAssociatedObject(b,kJK59DockState);Check(r && r.decoration.superview==b && !r.decoration.userInteractionEnabled,@"59 dock decoration inert, real button hit region live");
        if(i!=2){UILabel *l=(UILabel *)r.decoration.subviews.lastObject;Check([l.text isEqual:titles[i]],@"59 native destination labels preserved, no fake Settings destination");}
    }
    JK59DockRecord *selectedPaint=objc_getAssociatedObject(tabs[0],kJK59DockState);
    Check([((UILabel *)selectedPaint.decoration.subviews.lastObject).textColor isEqual:JK59Amber()] && fake.alpha==0,@"59 actual selected UITabBar item gives amber when all native button.selected flags are false; fake paint suppressed");
    fake.selectedItem=fakeItems[3];JKStyleApplyAll();selectedPaint=objc_getAssociatedObject(tabs[4],kJK59DockState);Check([((UILabel *)selectedPaint.decoration.subviews.lastObject).textColor isEqual:JK59Amber()],@"59 fake native selected item change updates correct real tab amber");fake.selectedItem=fakeItems[0];JKStyleApplyAll();
    Check(navigations==5,@"59 all five native dock actions still fire");
    JK59PlayerButton *speed=(id)[self.window viewWithTag:9360],*sound=(id)[self.window viewWithTag:9361];
    Check(speed && sound && speed.enabled && sound.enabled,@"59 controls appear only with safe ready public player bridge");
    [speed sendActionsForControlEvents:UIControlEventTouchUpInside];
    Check(fabs(active.rate-1.5)<.01 && inactive.rate==0 && [speed.accessibilityValue isEqual:@"1.5"],@"59 real rate 1 to1.5 and UI readback; offscreen unchanged");
    [sound sendActionsForControlEvents:UIControlEventTouchUpInside];
    Check(active.muted && !inactive.muted && [sound.accessibilityValue isEqual:@"Muted"],@"59 real mute and UI readback; offscreen unchanged");
    [sound sendActionsForControlEvents:UIControlEventTouchUpInside];Check(!active.muted,@"59 sound restores actual playback audio");
    [active pause];[speed refresh];[speed sendActionsForControlEvents:UIControlEventTouchUpInside];Check(active.rate==0 && !speed.enabled,@"59 rate control cannot resume paused video");[active playImmediatelyAtRate:1];
    // Ambiguous visible feeds cannot be guessed; no inert buttons get drawn.
    off.frame=fd.bounds;JKStyleApplyAll();Check(!JK59ActivePlayer(self.window) && ![self.window viewWithTag:9360] && ![self.window viewWithTag:9361],@"59 ambiguous visible player removes controls");
    off.frame=CGRectOffset(fd.bounds,0,-h);JKStyleApplyAll();
    // No AVPlayerLayer is evidenced in the device log. If absent at runtime,
    // missing bridge is explicit and controls are absent, not fake.
    [pl removeFromSuperlayer];JKStyleApplyAll();Check(!JK59ActivePlayer(self.window) && ![self.window viewWithTag:9360] && ![self.window viewWithTag:9361],@"59 non-AV/custom renderer has no inert top controls");
    [pv.layer addSublayer:pl];JKStyleApplyAll();
    JKAlaaToggle *dots=(id)[self.window viewWithTag:9156];Check(dots && dots.superview!=fr,@"59 independent dots trigger outside native rail");
    for(UIView *v in fr.subviews)Check(v.alpha>0 || objc_getAssociatedObject(v,kJKPaintAlpha),@"59 expanded native rail children restored");
    CGRect tr=[dots convertRect:dots.bounds toView:self.window],rr=[fr convertRect:fr.bounds toView:self.window];
    Check(CGRectGetMaxY(rr)<=CGRectGetMinY(tr)-16 && !CGRectIntersectsRect(tr,[caption convertRect:caption.bounds toView:self.window]),@"59 expanded rail above dots, dots outside left caption");
    [speed refresh];[sound refresh];
    CGRect realCanvas=[caption convertRect:caption.bounds toView:self.window],realAuthor=[author convertRect:author.bounds toView:self.window];
    Check(CGRectGetMaxX(realCanvas)<=CGRectGetMinX(rr)-12 && CGRectGetMaxX(realAuthor)<=CGRectGetMinX(rr)-12 && !CGRectIntersectsRect(realCanvas,[avatar convertRect:avatar.bounds toView:self.window]) && !CGRectIntersectsRect(realCanvas,[music convertRect:music.bounds toView:self.window]) && [nameText.text isEqual:@"blinx ✓"],@"59 Arabic actual drawRect canvas and checked blinx author remain physically left outside native rail/avatar/disc");
    ShotView(self.window,@"59-expanded");ShotView(self.window,@"59-top-and-dock");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];
    Check(JKRailClosed,@"59 native dots collapses state");
    JKRailApply(self.window,NO); // deterministically inspect final animation state
    for(UIView *v in fr.subviews)[v.layer removeAllAnimations];
    for(UIView *v in fr.subviews)Check(v.alpha==0 && !v.userInteractionEnabled && JKStyleAlpha(v,1)==0,@"59 ALL native rail children including avatar/disc/counts collapse and alpha guard");
    Check(dots.alpha==1 && !dots.hidden && dots.userInteractionEnabled,@"59 dots stays live to restore");
    ShotView(self.window,@"59-collapsed");
    TT50EyeActive=YES;TT50Tree(fd);JKStyleApplyAll();Check(dots.hidden && fr.hidden && ![self.window viewWithTag:9360],@"59 eye hides trigger/native rail/top independent of collapse");
    TT50Restore();JKStyleApplyAll();Check(JKRailClosed && !dots.hidden,@"59 eye off preserves collapsed choice");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);
    Check(!JKRailClosed,@"59 dots expands after eye roundtrip");
    for(UIView *v in fr.subviews)Check(v.alpha>0 && v.userInteractionEnabled,@"59 native rail children restored after expand");
    // A tap on a stale visible control cannot mutate an offscreen player.
    JK59PlayerButton *stale=(id)[self.window viewWithTag:9361];[active pause];pv.frame=CGRectOffset(fd.bounds,0,-h);off.frame=fd.bounds;
    BOOL beforeInactive=inactive.muted;[stale sendActionsForControlEvents:UIControlEventTouchUpInside];Check(inactive.muted==beforeInactive && !active.muted,@"59 stale control cannot target newly recycled feed");
    pv.frame=fd.bounds;off.frame=CGRectOffset(fd.bounds,0,-h);JKStyleApplyAll();
    [D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();
    Check(fake.alpha==1 && CGRectEqualToRect(dock.frame,dockBefore) && ![dock viewWithTag:9358] && ![self.window viewWithTag:9156] && ![self.window viewWithTag:9360],@"59 default removes new chrome and restores native dock");
    for(NSUInteger i=0;i<5;i++){UIView *b=tabs[i];Check(CGRectEqualToRect(b.frame,[buttonFrames[i] CGRectValue]) && !objc_getAssociatedObject(b,kJK59DockState) && [b.gestureRecognizers containsObject:tabGRs[i]],@"59 default restores all native tab geometry paint and gesture identities");}
    Check(CGAffineTransformIsIdentity(fr.transform) && avatar.alpha==1 && music.alpha==1 && follow.userInteractionEnabled,@"59 default restores rail geometry avatar follow music");
    ShotView(self.window,@"59-restored-default");
    [active pause];[inactive pause];[fake removeFromSuperview];[dock removeFromSuperview];[pv removeFromSuperview];[off removeFromSuperview];
    NSLog(@"PASS 59 native dock actions and labels, real AVPlayer rate/mute readback, offscreen rejection, full rail collapse, eye independence, default restoration");
}
