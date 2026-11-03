// 60 final owner scope: ALL feed UI hidden, only dots and mascot survive.
{
    banner.text=@"Joker 60 simulator TEST DATA\nAll-feed collapse + draggable dots";
    NSMutableArray *chrome=[NSMutableArray arrayWithObjects:fr,dock,author,caption,avatar,music,nil];
    NSMutableArray *added=[NSMutableArray new];
    for(NSString *n in @[@"TikTokFeedTabControl",@"TikTokFeedFadeScrollView",@"AWELiveFeedEntranceView",@"TTKSearchEntranceButton",@"TTKFeedBaseBottomBarView",@"AWEUserNameLabel"]){
        UIView *v=[[Fixture(n,UIView.class) alloc] initWithFrame:CGRectMake(10,100,100,25)];[fd addSubview:v];[chrome addObject:v];[added addObject:v];
    }
    UIView *mascot=[[UIView alloc] initWithFrame:CGRectMake(170,400,30,30)];mascot.backgroundColor=UIColor.orangeColor;mascot.accessibilityLabel=@"Enabled mascot fixture";[self.window addSubview:mascot];
    UIView *priorHidden=added[1];priorHidden.hidden=YES;priorHidden.alpha=.42;priorHidden.userInteractionEnabled=NO;
    UIView *priorDim=added[2];priorDim.alpha=.37;
    NSMutableArray *baseline=[NSMutableArray new];for(UIView *v in chrome)[baseline addObject:@[@(v.alpha),@(v.hidden),@(v.userInteractionEnabled)]];
    void(^checkHidden)(void)=^{JKAlaaToggle *live=(id)[self.window viewWithTag:9156];for(UIView *v in chrome)Check(v.hidden&&v.alpha==0&&!v.userInteractionEnabled,@"60 ALL rail dock top tabs LIVE search author caption bottom-search chrome hidden");Check(live&&!live.hidden&&live.alpha==1&&live.userInteractionEnabled&&!mascot.hidden&&mascot.alpha==1&&mascot.userInteractionEnabled,@"60 only floating dots and enabled mascot remain live; video container untouched");Check(!fd.hidden&&fd.alpha==1&&!pv.hidden,@"60 never hides feed/video/shared ancestors");};
    void(^checkRestored)(void)=^{for(NSUInteger i=0;i<chrome.count;i++){UIView *v=chrome[i];NSArray *r=baseline[i];Check(v.alpha==[r[0] doubleValue]&&v.hidden==[r[1] boolValue]&&v.userInteractionEnabled==[r[2] boolValue],@"60 tap restores EXACT prior visibility/alpha/input, including previously hidden/dim views");}};
    Check(!JKRailClosed,@"60 fixture begins expanded");ShotView(self.window,@"60-expanded");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkHidden();ShotView(self.window,@"60-collapsed");
    // New cell/tab chrome before display: synchronous setter guards and subtree
    // lifecycle are the same functions called by production UIView hooks.
    UIView *newDock=[[Fixture(@"TTKTabBar",UIView.class) alloc] initWithFrame:CGRectMake(0,h-80,w,80)];[fd addSubview:newDock];JK60Tree(newDock);
    Check(newDock.hidden&&newDock.alpha==0&&!newDock.userInteractionEnabled&&JK60Alpha(newDock,1)==0&&JK60Hidden(newDock,NO)&&!JK60Interaction(newDock,YES),@"60 newly re-added dock cannot flash via native setters");
    UIView *cell=[UIView new];UIView *newCaption=[[Fixture(@"AWEPlayInteractionDescriptionLabel",UIView.class) alloc] init];[cell addSubview:newCaption];[fd addSubview:cell];JK60Tree(cell);JK60BeforeDraw(newCaption);
    Check(newCaption.hidden&&newCaption.alpha==0&&!cell.hidden,@"60 reused/new feed cell hides only chrome before layout, not video container");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkRestored();Check(!newDock.hidden&&newDock.alpha==1&&!newCaption.hidden,@"60 expansion restores newly attached UI baseline too");[newDock removeFromSuperview];[cell removeFromSuperview];
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkHidden();
    JK60TestPan *pan=[JK60TestPan new];
    BOOL before=JKRailClosed;
    dots.dragged=NO;pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointMake(2,2);[dots drag:pan];pan.fixtureState=UIGestureRecognizerStateEnded;[dots drag:pan];
    Check(!dots.dragged,@"60 sub8pt movement remains a tap");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JKRailClosed!=before,@"60 movement below drag threshold still taps");
    JKRailClosed=YES;JKRailApply(self.window,NO);
    for(NSValue *delta in @[[NSValue valueWithCGPoint:CGPointMake(-300,-330)],[NSValue valueWithCGPoint:CGPointMake(-9999,-9999)],[NSValue valueWithCGPoint:CGPointMake(9999,9999)],[NSValue valueWithCGPoint:CGPointMake(-9999,0)]]){
        BOOL closed=JKRailClosed;dots.dragged=NO;
        pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointZero;[dots drag:pan];
        pan.fixtureState=UIGestureRecognizerStateChanged;pan.fixtureDelta=delta.CGPointValue;[dots drag:pan];
        pan.fixtureState=UIGestureRecognizerStateEnded;[dots drag:pan];
        [dots sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JKRailClosed==closed&&dots.dragged,@"60 dragged touch cannot accidentally toggle");
        UIEdgeInsets i=self.window.safeAreaInsets;CGRect safe=UIEdgeInsetsInsetRect(self.window.bounds,i);CGRect r=[dots convertRect:dots.bounds toView:self.window];
        Check(CGRectContainsRect(safe,r),@"60 drag clamps full control at left top right bottom safe edges");
        CGPoint center=[dots.superview convertPoint:dots.center toView:self.window];
        NSDictionary *saved=[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"];
        Check(fabs([saved[@"x"] doubleValue]*self.window.bounds.size.width-center.x)<.01&&fabs([saved[@"y"] doubleValue]*self.window.bounds.size.height-center.y)<.01,@"60 normalized drag position persisted");
        [dots removeFromSuperview];JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];
        CGPoint restored=[dots.superview convertPoint:dots.center toView:self.window];Check(hypot(center.x-restored.x,center.y-restored.y)<.01,@"60 recreated button restores persisted position");
        Check(dots.superview==self.window&&dots.gestureRecognizers.count==1&&dots.dragRecognizer.cancelsTouchesInView,@"60 drag recognizer local to button, no full-screen input interception");
        CGPoint outside=CGPointMake(CGRectGetMidX(self.window.bounds),CGRectGetMinY(self.window.bounds)+100);
        if(!CGRectContainsPoint(dots.frame,outside))Check([self.window hitTest:outside withEvent:nil]!=dots,@"60 outside button touches retain native hit testing");
        checkHidden();
    }
    // Persisted normalized center adapts to a new window size, not a pixel cache.
    NSDictionary *saved=[[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] copy];CGRect portrait=self.window.frame;
    self.window.frame=CGRectMake(portrait.origin.x,portrait.origin.y,portrait.size.height,portrait.size.width);
    [dots placeFloating];CGPoint expect=[dots clampFloating:CGPointMake([saved[@"x"] doubleValue]*self.window.bounds.size.width,[saved[@"y"] doubleValue]*self.window.bounds.size.height)];
    Check(hypot(dots.center.x-expect.x,dots.center.y-expect.y)<.01&&CGRectContainsRect(UIEdgeInsetsInsetRect(self.window.bounds,self.window.safeAreaInsets),dots.frame),@"60 rotation adapts normalized position and safe-area clamp");
    [dots removeFromSuperview];JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];Check(hypot(dots.center.x-expect.x,dots.center.y-expect.y)<.01,@"60 position persists after recreation in rotated size");
    self.window.frame=portrait;[dots placeFloating];
    Check([[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] isEqual:saved],@"60 rotation does not overwrite persisted normalized preference");

    // Eye ON while collapsed, expand while eye remains ON, then eye OFF.
    TT50EyeActive=YES;TT50Tree(fd);JKStyleApplyAll();Check(dots.hidden&&!dots.userInteractionEnabled,@"60 eye independently disables floating trigger");
    JKRailClosed=NO;JKRailApply(self.window,NO);Check(fr.hidden&&caption.hidden,@"60 expand does not override independent eye");
    TT50Restore();JKStyleApplyAll();checkRestored();
    // Opposite order: eye starts first, then collapse, then eye clears.
    TT50EyeActive=YES;TT50Tree(fd);JKRailClosed=YES;JKRailApply(self.window,NO);TT50Restore();JKStyleApplyAll();checkHidden();
    dots.dragged=NO;[dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkRestored();
    NSDictionary *position=[[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] copy];
    dots.dragged=NO;[dots flip];JKRailApply(self.window,NO);checkHidden();
    [D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();Check(![self.window viewWithTag:9156]&&JK60Saved().count==0&&!dock.hidden&&fr.alpha==1,@"60 style-off cleans collapse snapshots and restores native UI");
    [D() setInteger:3 forKey:JKStyleKey];JKRailClosed=NO;JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];Check([[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] isEqual:position],@"60 saved drag survives default and style recreation");
    JK60TestModalRoot *modalFixture=[JK60TestModalRoot new];Check(!JK60ModalCovered(modalFixture),@"60 absent modal permits floating input");modalFixture.fixturePresented=[UIViewController new];Check(JK60ModalCovered(modalFixture),@"60 native settings sheet covers floating input independently");
    for(UIView *v in added)[v removeFromSuperview];[mascot removeFromSuperview];
    NSLog(@"PASS 60 ALL feed chrome collapse exact restore; independent eye both orders; fresh-cell no flicker hooks; drag threshold edge clamp persistence recreation rotation default modal");
}
