// 60 narrow regression: collapse never owns the native dock visibility.
{
    banner.text=@"Joker 60 simulator TEST DATA\nNative controls fixture, NOT real TikTok";
    void(^checkDock)(void)=^{
        CGRect r=[dock convertRect:dock.bounds toView:self.window];
        Check(!dock.hidden&&dock.alpha>.99&&CGRectContainsRect(self.window.bounds,r)&&dock.window==self.window,@"60 native dock visible inside screen in both rail states");
        for(UIView *a=dock.superview;a;a=a.superview)Check(!a.hidden&&a.alpha>.01,@"60 dock ancestors are not collapse-hidden");
        for(UIView *b in tabs)Check(!b.hidden&&b.alpha>.99&&b.userInteractionEnabled,@"60 all native dock controls remain visible and live");
    };
    checkDock();
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkDock();
    Check(JKRailClosed,@"60 tap collapses without hiding dock");
    ShotView(self.window,@"60-collapsed-dock-visible");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];JKRailApply(self.window,NO);checkDock();
    Check(!JKRailClosed,@"60 tap expands without hiding dock");
    ShotView(self.window,@"60-expanded-dock-visible");
    // The screenshot with a native back arrow/search footer may be a detail
    // context with no native tabs. Never fabricate a dock or unhide native tabs
    // which TikTok has explicitly hidden for another screen.
    dock.hidden=YES;JKStyleApplyAll();Check(dock.hidden,@"60 native screen-level hidden dock stays native-hidden, not forcibly resurrected");dock.hidden=NO;
    JK60TestPan *pan=[JK60TestPan new];
    BOOL before=JKRailClosed;
    dots.dragged=NO;pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointMake(2,2);[dots drag:pan];pan.fixtureState=UIGestureRecognizerStateEnded;[dots drag:pan];
    Check(!dots.dragged,@"60 sub8pt movement remains a tap");
    [dots sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JKRailClosed!=before,@"60 movement below drag threshold still taps");
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
        checkDock();
    }
    // Persisted normalized center adapts to a new window size, not a pixel cache.
    NSDictionary *saved=[[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] copy];CGRect portrait=self.window.frame;
    self.window.frame=CGRectMake(portrait.origin.x,portrait.origin.y,portrait.size.height,portrait.size.width);
    [dots placeFloating];CGPoint expect=[dots clampFloating:CGPointMake([saved[@"x"] doubleValue]*self.window.bounds.size.width,[saved[@"y"] doubleValue]*self.window.bounds.size.height)];
    Check(hypot(dots.center.x-expect.x,dots.center.y-expect.y)<.01&&CGRectContainsRect(UIEdgeInsetsInsetRect(self.window.bounds,self.window.safeAreaInsets),dots.frame),@"60 rotation adapts normalized position and safe-area clamp");
    [dots removeFromSuperview];JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];Check(hypot(dots.center.x-expect.x,dots.center.y-expect.y)<.01,@"60 position persists after recreation in rotated size");
    self.window.frame=portrait;[dots placeFloating];
    Check([[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] isEqual:saved],@"60 rotation does not overwrite persisted normalized preference");
    [D() setInteger:0 forKey:JKStyleKey];JKStyleApplyAll();Check(![self.window viewWithTag:9156]&&!objc_getAssociatedObject(dock,kJK59DockState),@"60 default cleans up draggable trigger and dock decoration");
    [D() setInteger:3 forKey:JKStyleKey];JKStyleApplyAll();dots=(id)[self.window viewWithTag:9156];
    Check([[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] isEqual:saved],@"60 persisted position survives style-off and style-on");
    BOOL collapsed=JKRailClosed;TT50EyeActive=YES;TT50Tree(fd);JKStyleApplyAll();Check(dots.hidden&&!dots.userInteractionEnabled,@"60 eye hides and disables drag trigger");
    pan.fixtureState=UIGestureRecognizerStateBegan;pan.fixtureDelta=CGPointMake(100,100);[dots drag:pan];Check([[NSUserDefaults.standardUserDefaults dictionaryForKey:@"tt122.position.9156"] isEqual:saved],@"60 hidden eye-state drag cannot mutate saved position");
    checkDock();TT50Restore();JKStyleApplyAll();Check(!dots.hidden&&dots.userInteractionEnabled&&JKRailClosed==collapsed,@"60 eye restores trigger without changing collapse or position");
    dots.dragged=NO;[dots sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JKRailClosed!=collapsed,@"60 tap toggles after eye and drag recreation");
    dots.dragged=NO;[dots sendActionsForControlEvents:UIControlEventTouchUpInside];Check(JKRailClosed==collapsed,@"60 second tap restores both states");
    if(!JKRailClosed){dots.dragged=NO;[dots flip];}JKRailApply(self.window,NO);checkDock();
    ShotView(self.window,@"60-dragged-left-collapsed-dock-visible");
    dots.dragged=NO;[dots flip];JKRailApply(self.window,NO);
    Check(!JKRailClosed,@"60 leaves existing 59 restore fixture expanded");
    NSLog(@"PASS 60 dock visible expanded+collapsed; threshold-safe drag, all edges, normalized persistence, recreation, rotation, tap, eye and default cleanup");
}
